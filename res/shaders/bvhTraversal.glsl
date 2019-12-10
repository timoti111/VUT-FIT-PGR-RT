#include intersections.glsl
bool IntersectPrimitive(Ray ray, Primitive primitive, Mesh mesh, bool occlusion, inout RayHit intersection)
{
    switch (primitive.type)
    {
        case TRIANGLE:                
            Triangle triangle = triangles[primitive.index];
            ivec3 vIndexes = triangle.vertices.xyz;
            vec3 v0 = (mesh.objectToWorld * vertices[vIndexes.x]).xyz;
            vec3 v1 = (mesh.objectToWorld * vertices[vIndexes.y]).xyz;
            vec3 v2 = (mesh.objectToWorld * vertices[vIndexes.z]).xyz;
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
                    intersection.position = ray.origin + t * ray.direction;
                    if (mesh.smoothing)
                    {
                        ivec3 nIndices = triangle.normals.xyz;
                        vec4 n0 = (mesh.objectToWorld * normals[nIndices.x]);
                        vec4 n1 = (mesh.objectToWorld * normals[nIndices.y]);
                        vec4 n2 = (mesh.objectToWorld * normals[nIndices.z]);
                        intersection.normal = normalize((1 - u - v) * n0 + u * n1 + v * n2);
                    }
                    else
                        intersection.normal = vec4(normalize(cross(v1 - v0, v2 - v0)), 0.0f);
//                    ivec3 uvIndices = triangle.uvs.xyz;
//                    vec2 uv0 = coords[uvIndices.x]);
//                    vec4 uv1 = coords[uvIndices.y]);
//                    vec4 uv2 = coords[uvIndices.z]);
//                    intersection.uv = (1.0f - u - v) * triangles[primitive.index].v0uv + u * triangles[primitive.index].v1uv + v * triangles[primitive.index].v2uv;
                    
                    intersection.matID = mesh.materialID;
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
        if (node.rightOffset == 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Primitive primitive = primitives[mesh.primitiveOffset + node.start + o];
//                if (IntersectPrimitive(ray, primitive, mesh, occlusion, intersection))
//                {
//                    if (occlusion)
//                        return true;
////                     intersection.primIndex = int(mesh.primitiveOffset + node.start + o);
//                }
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
        if (node.rightOffset == 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Mesh mesh = meshes[node.start + o];

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
