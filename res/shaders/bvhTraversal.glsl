#include intersections.glsl
bool IntersectPrimitive(Ray ray, Primitive primitive, Mesh mesh, bool occlusion, inout RayHit intersection)
{
    switch (primitive.type)
    {
        case TRIANGLE:
            vec3 v0 = (mesh.objectToWorld * triangles[primitive.index].vertices[0]).xyz;
            vec3 v1 = (mesh.objectToWorld * triangles[primitive.index].vertices[1]).xyz;
            vec3 v2 = (mesh.objectToWorld * triangles[primitive.index].vertices[2]).xyz;
            float t, u, v;

            if (IntersectTriangle(ray, v0, v1, v2, t, u, v))
            {
                // If we're only looking for occlusion, then any hit is good enough
                if(occlusion)
                    return true;
                // Otherwise, keep the closest intersection only
                if (t < intersection.t)
                {
                    intersection.t = t;
                    intersection.matID = mesh.materialID;
//                    if (mesh.smoothing)
//                    {
//                        vec4 n0 = (mesh.objectToWorld * vec4(triangles[primitive.index].normals[0], 0.0f));
//                        vec4 n1 = (mesh.objectToWorld * vec4(triangles[primitive.index].normals[1], 0.0f));
//                        vec4 n2 = (mesh.objectToWorld * vec4(triangles[primitive.index].normals[2], 0.0f));
//                        intersection.normal = (1 - u - v) * n0 + u * n1 + v * n2;
//                    }
//                    else
                        intersection.normal = vec4(cross(v1 - v0, v2 - v0), 0.0f);
                    vec2 uv0 = triangles[primitive.index].coords[0];
                    vec2 uv1 = triangles[primitive.index].coords[1];
                    vec2 uv2 = triangles[primitive.index].coords[2];
                    intersection.uv = (1.0f - u - v) * uv0 + u * uv1 + v * uv2;
                    
                    intersection.triIndex = primitive.index;
                    return true;
                }
            }
            break;
        case SPHERE:
            Sphere sphere = spheres[primitive.index];
            if (IntersectSphere(ray, sphere.sphere, intersection))
            {
                // If we're only looking for occlusion, then any hit is good enough
                if(occlusion)
                    return true;
                intersection.matID = mesh.materialID;
                return true;
            }
            break;
        case CYLINDER:
            Cylinder cylinder = cylinders[primitive.index];
            if (IntersectCylinder(ray, cylinder.begin.xyz, cylinder.end.xyz, cylinder.radius, intersection))
            {
                // If we're only looking for occlusion, then any hit is good enough
                if(occlusion)
                    return true;
                intersection.matID = mesh.materialID;
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectMesh(Ray ray, Mesh mesh, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    int todo[64];
    int stackptr = 0;
    float tLast = intersection.t;
    // "Push" on the root node to the working set
    todo[stackptr] = mesh.bvhOffset;

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr];
        stackptr--;
        BVHNode node = meshBVHs[ni];

        // Is leaf -> Intersect
        if (node.nPrims > 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Primitive primitive = primitives[mesh.primitiveOffset + node.rightOffset + o];
                if (IntersectPrimitive(ray, primitive, mesh, occlusion, intersection) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
             // Not a leaf
            BVHNode leftChild = meshBVHs[ni + 1];
            BVHNode rightChild = meshBVHs[ni + node.rightOffset];

            bool hitc0 = IntersectAABB(ray, leftChild.minBound, leftChild.maxBound, bbhitsc0, intersection.t);
            bool hitc1 = IntersectAABB(ray, rightChild.minBound, rightChild.maxBound, bbhitsc1, intersection.t);

            // Did we hit both nodes?
            if(hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni + 1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if(bbhitsc1.x < bbhitsc0.x)
                    swap(closer, other, int);

                // It's possible that the nearest object is still in the other side, but we'll
                // check the further-awar node later...

                // Push the farther first
                todo[++stackptr] = other;

                // And now the closer (with overlap test)
                todo[++stackptr] = closer;
            }
            else if (hitc0)
                todo[++stackptr] = ni + 1;
            else if(hitc1)
                todo[++stackptr] = ni + int(node.rightOffset);

        }
    }
    return intersection.t < tLast;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene(Ray ray, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    int todo[64];
    int stackptr = 0;
    float tLast = intersection.t;

    // "Push" on the root node to the working set
    todo[stackptr] = 0;

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr];
        stackptr--;
        BVHNode node = sceneBVH[ni];

        // Is leaf -> Intersect
        if (node.nPrims > 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Mesh mesh = meshes[node.rightOffset + o];

                if (IntersectMesh(ray, mesh, occlusion, intersection) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
            BVHNode leftChild = sceneBVH[ni + 1];
            BVHNode rightChild = sceneBVH[ni + node.rightOffset];

            bool hitc0 = IntersectAABB(ray, leftChild.minBound, leftChild.maxBound, bbhitsc0, intersection.t);
            bool hitc1 = IntersectAABB(ray, rightChild.minBound, rightChild.maxBound, bbhitsc1, intersection.t);

            // Did we hit both nodes?
            if(hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni + 1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if(bbhitsc1.x < bbhitsc0.x)
                    swap(closer, other, int);

                // It's possible that the nearest object is still in the other side, but we'll
                // check the further-awar node later...

                // Push the farther first
                todo[++stackptr] = other;

                // And now the closer (with overlap test)
                todo[++stackptr] = closer;
            }
            else if (hitc0)
                todo[++stackptr] = ni + 1;
            else if(hitc1)
                todo[++stackptr] = ni + int(node.rightOffset);
        }
    }
    return intersection.t < tLast;
}



// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene2(Ray ray, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    int todo[64];
    int stackptr = 0;
    float tLast = intersection.t;

    // "Push" on the root node to the working set
    todo[stackptr] = 0;

    int intStackPtr = 0;
    uint toIntersect[64];

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr];
        stackptr--;
        BVHNode node = sceneBVH[ni];

        // Is leaf -> Intersect
        if (node.nPrims > 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
                toIntersect[intStackPtr++] = node.rightOffset + o;
        }
        else
        { // Not a leaf
            BVHNode leftChild = sceneBVH[ni + 1];
            BVHNode rightChild = sceneBVH[ni + node.rightOffset];

            bool hitc0 = IntersectAABB(ray, leftChild.minBound, leftChild.maxBound, bbhitsc0, intersection.t);
            bool hitc1 = IntersectAABB(ray, rightChild.minBound, rightChild.maxBound, bbhitsc1, intersection.t);

            // Did we hit both nodes?
            if(hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni + 1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if(bbhitsc1.x < bbhitsc0.x)
                    swap(closer, other, int);

                // It's possible that the nearest object is still in the other side, but we'll
                // check the further-awar node later...

                // Push the farther first
                todo[++stackptr] = other;

                // And now the closer (with overlap test)
                todo[++stackptr] = closer;
            }
            else if (hitc0)
                todo[++stackptr] = ni + 1;
            else if(hitc1)
                todo[++stackptr] = ni + int(node.rightOffset);
        }
    }

    for (int i = 0; i < intStackPtr; i++)
        if (IntersectMesh(ray, meshes[toIntersect[i]], occlusion, intersection) && occlusion)
            return true;

    return intersection.t < tLast;
}



// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene3(Ray ray, bool occlusion, inout RayHit intersection)
{
    float tLast = intersection.t;
    for (int i = 0; i < meshes.length(); i++)
        if (IntersectMesh(ray, meshes[i], occlusion, intersection) && occlusion)
            return true;

    return intersection.t < tLast;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene4(Ray ray, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    float tLast = intersection.t;
    for (int i = 0; i < meshes.length(); i++)
    {
        Mesh mesh = meshes[i];
        BVHNode meshNode = meshBVHs[mesh.bvhOffset];
        if (IntersectAABB(ray, meshNode.minBound, meshNode.maxBound, bbhitsc0, intersection.t))
        {
            if (IntersectMesh(ray, mesh, occlusion, intersection) && occlusion)
                return true;
        }
    }

    return intersection.t < tLast;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene5(Ray ray, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    float tLast = intersection.t;
    int intStackPtr = 0;
    uint toIntersect[64];
    for (int i = 0; i < meshes.length(); i++)
    {
        Mesh mesh = meshes[i];
        BVHNode meshNode = meshBVHs[mesh.bvhOffset];
        if (IntersectAABB(ray, meshNode.minBound, meshNode.maxBound, bbhitsc0, intersection.t))
            toIntersect[intStackPtr++] = i;
    }
    
    for (int i = 0; i < intStackPtr; i++)
        if (IntersectMesh(ray, meshes[toIntersect[i]], occlusion, intersection) && occlusion)
            return true;
    return intersection.t < tLast;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene6(Ray ray, bool occlusion, inout RayHit intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    float tLast = intersection.t;
    int intStackPtr = 0;
    Mesh toIntersect[64];
    for (int i = 0; i < meshes.length(); i++)
    {
        Mesh mesh = meshes[i];
        BVHNode meshNode = meshBVHs[mesh.bvhOffset];
        if (IntersectAABB(ray, meshNode.minBound, meshNode.maxBound, bbhitsc0, intersection.t))
            toIntersect[intStackPtr++] = mesh;
    }
    
    for (int i = 0; i < intStackPtr; i++)
        if (IntersectMesh(ray, toIntersect[i], occlusion, intersection) && occlusion)
            return true;
    return intersection.t < tLast;
}
