#include "Mesh.h"
#include "Scene/Scene.h"

Geometry::Mesh::Mesh(Scene& scene, std::string name, int primitiveOffset, int primitiveCount, glm::mat4x4 objectToWorld) :
    Primitive(MESH, -1),
    BVH(1),
    scene(&scene),
    name(name),
    objectToWorld(objectToWorld),
    smoothing(true),
    selected(false)
{
    for (int i = primitiveOffset; i < primitiveOffset + primitiveCount; i++)
        addPrimitive(&this->scene->primitives[i]);
    updateBVHs();
}

AABB Geometry::Mesh::getAABB(Scene& scene, glm::mat4x4& objectToWorld)
{
    return AABB(getFlatTree()[0].min, getFlatTree()[0].max);
}

glm::vec3 Geometry::Mesh::getCentroid(Scene& scene, glm::mat4x4& objectToWorld)
{
    AABB own = getAABB(scene, objectToWorld);
    return glm::vec3(
        own.min.x + own.extent.x * 0.5f,
        own.min.y + own.extent.y * 0.5f,
        own.min.z + own.extent.z * 0.5f
    );
}

void Geometry::Mesh::setObjectToWorld(glm::mat4x4& objectToWorld)
{
    this->objectToWorld = objectToWorld;
    updateBVHs();
}

glm::mat4x4 Geometry::Mesh::getObjectToWorld()
{
    return objectToWorld;
}

void Geometry::Mesh::updateBVHs()
{
    update(*this->scene, objectToWorld);
    scene->setUpdateSceneBVH();
}

// Node for storing state information during traversal.
struct BVHTraversal
{
    int i; // Node
    float mint;
    static BVHTraversal create(int i, float mint);
};

bool Geometry::GPU::Mesh::intersect(Ray& ray, Scene& scene, bool occlusion)
{
    glm::vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = this->bvhOffset;
    todo[stackptr].mint = -FLT_MAX;
    float tLast = ray.t;

    while (stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        auto& node = scene.meshBVHs[ni];

        // If this node is further than the closest found intersection, continue
        if (near > ray.t)
            continue;

        // Is leaf -> Intersect
        if (node.rightOffset == 0)
        {
            for (int o = 0; o < node.nPrims; ++o)
            {
                auto& primitive = scene.primitivesGPU[this->primitiveOffset + node.start + o];
                if (primitive.intersect(ray, *this, scene) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
            auto& leftChild = scene.meshBVHs[ni + 1];
            auto& rightChild = scene.meshBVHs[ni + node.rightOffset];

            auto hitc0 = leftChild.intersect(ray, bbhitsc0);
            auto hitc1 = rightChild.intersect(ray, bbhitsc1);

            // Did we hit both nodes?
            if (hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni + 1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if (bbhitsc1.x < bbhitsc0.x)
                {
                    float ftmp = bbhitsc0.x;
                    bbhitsc0.x = bbhitsc1.x;
                    bbhitsc1.x = ftmp;

                    ftmp = bbhitsc0.y;
                    bbhitsc0.y = bbhitsc1.y;
                    bbhitsc1.y = ftmp;

                    int itmp = closer;
                    closer = other;
                    other = itmp;
                }

                // It's possible that the nearest object is still in the other side, but we'll
                // check the further-awar node later...

                // Push the farther first
                todo[++stackptr] = BVHTraversal::create(other, bbhitsc1.x);

                // And now the closer (with overlap test)
                todo[++stackptr] = BVHTraversal::create(closer, bbhitsc0.x);
            }
            else if (hitc0)
                todo[++stackptr] = BVHTraversal::create(ni + 1, bbhitsc0.x);
            else if (hitc1)
                todo[++stackptr] = BVHTraversal::create(ni + int(node.rightOffset), bbhitsc1.x);

        }
    }

    return ray.t < tLast;
}

int Geometry::GPU::Mesh::getOriginalIndex(Scene& scene)
{
    for (int i = 0; i < scene.meshes.size(); i++)
        if (scene.meshes[i].bvhOffset == this->bvhOffset)
            return i;
    return -1;
}

Geometry::GPU::Mesh::Mesh(::Geometry::Mesh& o) :
    objectToWorld(o.objectToWorld),
    primitiveOffset(o.gpuPrimitiveOffset),
    bvhOffset(o.bvhOffset),
    smoothing(o.smoothing),
    materialID(o.materialID)
{}
