#pragma once

#include <vector>
#include <stdint.h>
#include <glm/mat4x4.hpp>
#include "TreeObject.h"

//! Node descriptor for the flattened tree
struct alignas(16) BVHFlatNode
{
    glm::vec4 min;
    glm::vec4 max;
    uint32_t start;
    uint32_t nPrims;
    uint32_t rightOffset;
};

//! \author Brandon Pelfrey
//! A Bounding Volume Hierarchy system for fast Ray-Object intersection tests
class BVH
{

public:
    BVH(uint32_t leafSize = 4);

    std::vector<BVHFlatNode>& getFlatTree();
    std::vector<TreeObject*>& getPrimitives();
    void addPrimitive(TreeObject* primitive);
    void clearPrimitives();

protected:
    //! Build the BVH tree out of build_prims
    void update(Scene& scene);
    //! Build the BVH tree out of build_prims
    void update(Scene& scene, glm::mat4x4& objectToWorld);

private:
    uint32_t nNodes, nLeafs, leafSize;

    // Fast Traversal System
    std::vector<BVHFlatNode> flatTree;
    std::vector<TreeObject*> build_prims;
};
