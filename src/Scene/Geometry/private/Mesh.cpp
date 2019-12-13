#include "Mesh.h"
#include "Scene/Scene.h"
#include "Scene/Geometry/Shape.h"

Geometry::MeshInstance::MeshInstance(Mesh* parent, glm::mat4x4 objectToWorld, int materialID, bool smoothing) :
    Primitive(MESH, -1, nullptr),
    BVH(1),
    parent(parent),
    objectToWorld(objectToWorld),
    materialID(materialID),
    smoothing(smoothing)
{
    primitives.reserve(parent->triangles.size() + parent->spheres.size() + parent->cylinders.size());
    for (int i = 0; i < parent->triangles.size(); i++)
    {
        primitives.push_back(Geometry::Primitive(Geometry::TRIANGLE, i, this));
        addPrimitive(&primitives[primitives.size() - 1]);
    }
    for (int i = 0; i < parent->spheres.size(); i++)
    {
        primitives.push_back(Geometry::Primitive(Geometry::SPHERE, i, this));
        addPrimitive(&primitives[primitives.size() - 1]);
    }
    for (int i = 0; i < parent->cylinders.size(); i++)
    {
        primitives.push_back(Geometry::Primitive(Geometry::CYLINDER, i, this));
        addPrimitive(&primitives[primitives.size() - 1]);
    }
    updateBVHs();
}

AABB Geometry::MeshInstance::getAABB()
{
    return AABB(getFlatTree()[0].min, getFlatTree()[0].max);
}

glm::vec3 Geometry::MeshInstance::getCentroid()
{
    AABB own = getAABB();
    return glm::vec3(
        own.min.x + own.extent.x * 0.5f,
        own.min.y + own.extent.y * 0.5f,
        own.min.z + own.extent.z * 0.5f
    );
}

void Geometry::MeshInstance::setObjectToWorld(glm::mat4x4 objectToWorld)
{
    this->objectToWorld = objectToWorld;
    updateBVHs();
}

void Geometry::MeshInstance::setSmoothing(bool smoothing)
{
    this->smoothing = smoothing;
    parent->instanceChanged();
}

void Geometry::MeshInstance::setMaterialID(int materialID)
{
    this->materialID = materialID;
    parent->instanceChanged();
}

glm::mat4x4 Geometry::MeshInstance::getObjectToWorld()
{
    return objectToWorld;
}

void Geometry::MeshInstance::updateBVHs()
{
    clearPrimitives();
    for (auto& primitive : primitives)
        addPrimitive(&primitive);
    update();
    parent->instanceChanged();
}

// Node for storing state information during traversal.
struct BVHTraversal
{
    int i; // Node
    float mint;
    static BVHTraversal create(int i, float mint);
};

bool Geometry::MeshInstance::intersect(Ray& ray, Scene& scene, bool occlusion)
{
    glm::vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = 0;
    todo[stackptr].mint = -FLT_MAX;
    float tLast = ray.t;

    while (stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        auto& node = getFlatTree()[ni];

        // If this node is further than the closest found intersection, continue
        if (near > ray.t)
            continue;

        // Is leaf -> Intersect
        if (node.nPrims)
        {
            for (int o = 0; o < node.nPrims; ++o)
            {
                auto primitive = dynamic_cast<Geometry::Primitive*>(getPrimitives()[node.rightOffset + o]);
                if (primitive->intersect(ray, *this) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
            auto& leftChild = getFlatTree()[ni + 1];
            auto& rightChild = getFlatTree()[ni + node.rightOffset];

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

Geometry::GPU::MeshInstance::MeshInstance(::Geometry::MeshInstance& o) :
    objectToWorld(o.objectToWorld),
    primitiveOffset(o.gpuPrimitiveOffset),
    bvhOffset(o.bvhOffset),
    smoothing(o.smoothing),
    materialID(o.materialID)
{}
