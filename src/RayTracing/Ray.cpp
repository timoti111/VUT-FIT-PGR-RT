#include "Ray.h"
#include "bvh/BVH.h"
#include "RenderInfo.h"
#include "Scene/Scene.h"
#define FLT_MAX 3.402823466e+38

// Node for storing state information during traversal.
struct BVHTraversal
{
    int i; // Node
    float mint;
    static BVHTraversal create(int i, float mint);
};

Geometry::MeshInstance* Ray::traceRay(Scene* scene, bool occlusion)
{
    glm::vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = 0;
    todo[stackptr].mint = -FLT_MAX;
    float tLast = this->t;
    auto& sceneBVH = scene->getFlatTree();
    Geometry::MeshInstance* meshInstance = nullptr;

    while (stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        auto& node = sceneBVH[ni];

        // If this node is further than the closest found intersection, continue
        if (near > this->t)
            continue;

        // Is leaf -> Intersect
        if (node.rightOffset == 0)
        {
            for (int o = 0; o < node.nPrims; ++o)
            {
                Geometry::MeshInstance* mesh = dynamic_cast<Geometry::MeshInstance*>(scene->getPrimitives()[node.start + o]);
                if (mesh->materialID != -1)
                {
                    if (mesh->intersect(*this, *scene, occlusion))
                    {
                        meshInstance = mesh;
                        if (occlusion)
                            return meshInstance;
                    }
                }
            }
        }
        else
        { // Not a leaf
            auto& leftChild = sceneBVH[ni + 1];
            auto& rightChild = sceneBVH[ni + node.rightOffset];

            auto hitc0 = leftChild.intersect(*this, bbhitsc0);
            auto hitc1 = rightChild.intersect(*this, bbhitsc1);

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

    return meshInstance;
}

Ray Ray::createCameraRay(RenderInfo* renderInfo, glm::vec2 pixel, glm::ivec2 resolution)
{
    Ray ray;
    auto& camera = renderInfo->renderParams.camera;
    float sensorHalfHeight = (resolution.y * camera.sensorHalfWidth) / resolution.x;
    float pixelSize = camera.sensorHalfWidth / (resolution.x * 0.5f);
    ray.direction = glm::vec4(glm::normalize(glm::vec3(camera.direction + camera.left * (camera.sensorHalfWidth - (pixel.x + 0.5f) * pixelSize) + camera.up * ((pixel.y + 0.5f) * pixelSize - sensorHalfHeight))), 0.0f);
    ray.origin = glm::vec4(camera.position);
    ray.t = FLT_MAX;
    return ray;
}

BVHTraversal BVHTraversal::create(int i, float mint)
{
    BVHTraversal ret;
    ret.i = i;
    ret.mint = mint;
    return ret;
}
