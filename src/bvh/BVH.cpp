#include <algorithm>
#include "BVH.h"

BVH::BVH(uint32_t leafSize)
    : nNodes(0), nLeafs(0), leafSize(leafSize)
{
}

struct BVHBuildEntry
{
    // If non-zero then this is the index of the parent. (used in offsets)
    uint32_t parent;
    // The range of objects in the object list covered by this node.
    uint32_t start, end;
};

void BVH::update(Scene& scene)
{
    glm::mat4x4 identity = glm::mat4x4(1.0f);
    update(scene, identity);
}

/*! Build the BVH, given an input data set
 *  - Handling our own stack is quite a bit faster than the recursive style.
 *  - Each build stack entry's parent field eventually stores the offset
 *    to the parent of that node. Before that is finally computed, it will
 *    equal exactly three other values. (These are the magic values Untouched,
 *    Untouched-1, and TouchedTwice).
 *  - The partition here was also slightly faster than std::partition.
 */
void BVH::update(Scene& scene, glm::mat4x4& objectToWorld)
{
    BVHBuildEntry todo[128];
    uint32_t stackptr = 0;
    const uint32_t Untouched = 0xffffffff;
    const uint32_t TouchedTwice = 0xfffffffd;

    // Push the root
    todo[stackptr].start = 0;
    todo[stackptr].end = build_prims.size();
    todo[stackptr].parent = 0xfffffffc;
    stackptr++;

    BVHFlatNode node;
    std::vector<BVHFlatNode> buildnodes;
    buildnodes.reserve(build_prims.size() * 2);

    while (stackptr > 0)
    {
        // Pop the next item off of the stack
        BVHBuildEntry& bnode(todo[--stackptr]);
        uint32_t start = bnode.start;
        uint32_t end = bnode.end;
        uint32_t nPrims = end - start;

        nNodes++;
        node.start = start;
        node.nPrims = nPrims;
        node.rightOffset = Untouched;

        // Calculate the bounding box for this node
        AABB bb(build_prims[start]->getAABB(scene, objectToWorld));
        AABB bc(build_prims[start]->getCentroid(scene, objectToWorld));
        for (uint32_t p = start + 1; p < end; ++p)
        {
            bb.expandToInclude(build_prims[p]->getAABB(scene, objectToWorld));
            bc.expandToInclude(build_prims[p]->getCentroid(scene, objectToWorld));
        }
        node.max = glm::vec4(bb.max, 0.0f);
        node.min = glm::vec4(bb.min, 0.0f);

        // If the number of primitives at this point is less than the leaf
        // size, then this will become a leaf. (Signified by rightOffset == 0)
        if (nPrims <= leafSize)
        {
            node.rightOffset = 0;
            nLeafs++;
        }

        buildnodes.push_back(node);

        // Child touches parent...
        // Special case: Don't do this for the root.
        if (bnode.parent != 0xfffffffc)
        {
            buildnodes[bnode.parent].rightOffset--;

            // When this is the second touch, this is the right child.
            // The right child sets up the offset for the flat tree.
            if (buildnodes[bnode.parent].rightOffset == TouchedTwice)
            {
                buildnodes[bnode.parent].rightOffset = nNodes - 1 - bnode.parent;
            }
        }

        // If this is a leaf, no need to subdivide.
        if (node.rightOffset == 0)
            continue;

        // Set the split dimensions
        uint32_t split_dim = bc.maxDimension();

        // Split on the center of the longest axis
        float split_coord = .5f * (bc.min[split_dim] + bc.max[split_dim]);

        // Partition the list of objects on this split
        uint32_t mid = start;
        for (uint32_t i = start; i < end; ++i)
        {
            if (build_prims[i]->getCentroid(scene, objectToWorld)[split_dim] < split_coord)
            {
                std::swap(build_prims[i], build_prims[mid]);
                ++mid;
            }
        }

        // If we get a bad split, just choose the center...
        if (mid == start || mid == end)
        {
            mid = start + (end - start) / 2;
        }

        // Push right child
        todo[stackptr].start = mid;
        todo[stackptr].end = end;
        todo[stackptr].parent = nNodes - 1;
        stackptr++;

        // Push left child
        todo[stackptr].start = start;
        todo[stackptr].end = mid;
        todo[stackptr].parent = nNodes - 1;
        stackptr++;
    }

    // Copy the temp node data to a flat array
    flatTree.clear();
    for (uint32_t n = 0; n < nNodes; ++n)
        flatTree.emplace_back(buildnodes[n]);
}

std::vector<BVHFlatNode>& BVH::getFlatTree()
{
    return flatTree;
}

std::vector<TreeObject*>& BVH::getPrimitives()
{
    return build_prims;
}

void BVH::addPrimitive(TreeObject* primitive)
{
    build_prims.emplace_back(primitive);
}

void BVH::clearPrimitives()
{
    build_prims.clear();
}