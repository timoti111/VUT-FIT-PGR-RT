#version 460
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

layout(local_size_x = 8, local_size_y = 8) in;

struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 left;
    float sensorHalfWidth;
};
uniform Camera camera;

layout(rgba32f, binding = 1) uniform image2D destTex;
layout(binding = 2) uniform sampler2D hdriTexture;

struct BVHNode
{
    vec4 minBound;
    vec4 maxBound;
    uint start;
    uint nPrims;
    uint rightOffset;
};
layout(std430, binding = 3) buffer SceneBVHBuffer
{
    BVHNode sceneBVH[];
};
layout(std430, binding = 4) buffer MeshBVHsBuffer
{
    BVHNode meshBVHs[];
};

struct Mesh
{
    mat4x4 objectToWorld;
    int primitiveOffset;
    int bvhOffset;
    int materialID;
    bool smoothing;
};
layout(std430, binding = 5) buffer MeshesBuffer
{
    Mesh meshes[];
};

struct Primitive 
{
    int type;
    int index;
};
layout(std430, binding = 6) buffer PrimitivesBuffer
{
    Primitive primitives[];
};

struct Triangle
{
    ivec4 vertices;
    ivec4 normals;
    ivec4 coords;
};
layout(std430, binding = 7) buffer TrianglesBuffer
{
    Triangle triangles[];
};

layout(std430, binding = 8) buffer VerticesBuffer
{
    vec4 vertices[];
};

layout(std430, binding = 9) buffer NormalsBuffer
{
    vec4 normals[];
};

layout(std430, binding = 10) buffer CoordsBuffer
{
    vec2 coords[];
};

struct Sphere
{
    vec4 sphere;
};
layout(std430, binding = 11) buffer SpheresBuffer
{
    Sphere spheres[];
};

struct Cylinder
{
    vec4 begin;
    vec4 end;
    float radius;
};
layout(std430, binding = 12) buffer CylindersBuffer
{
    Cylinder cylinders[];
};

struct Ray
{
    vec4 origin;
    vec4 direction;
    vec4 energy;
};

Ray CreateRay(vec4 origin, vec4 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = vec4(1.0f);
    return ray;
}

Ray CreateCameraRay(vec2 pixel, ivec2 size)
{
//    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
//    float pixelSize = camera.sensorHalfWidth / (size.x * 0.5f);
//    vec4 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - (pixel.x + 0.5f) * pixelSize) + camera.up * ((pixel.y + 0.5f) * pixelSize - sensorHalfHeight));
//    return CreateRay(camera.position, direction);
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    return CreateRay(camera.position, direction);
}

struct RayHit
{
    vec4 position;
    vec4 normal;
    vec4 albedo;
    vec3 specular;
    float t;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = vec4(0.0f);
    hit.t = FLT_MAX;
    hit.normal = vec4(0.0f);
    hit.albedo = vec4(0.0f);
    hit.specular = vec3(0.0f);
    return hit;
};

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
//     Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.t)
    {
        bestHit.t = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
        bestHit.albedo = vec4(0.9f);
        bestHit.specular = vec3(0.1f);
    }
}

bool IntersectSphere(Ray ray, vec4 sphere, inout RayHit bestHit)
{
    // Calculate distance along the ray where the sphere is intersected
//    vec3 d = ray.origin.xyz - sphere.xyz;
//    float p1 = -dot(ray.direction.xyz, d);
//    float p2sqr = p1 * p1 - dot(d, d) + sphere.w * sphere.w;
//    if (p2sqr < 0)
//        return false;
//    float p2 = sqrt(p2sqr);
//    float t = p1 - p2 > 0.0f ? p1 - p2 : p1 + p2;
//    if (t > 0 && t < bestHit.t)
//    {
//        bestHit.t = t;
//        bestHit.position = ray.origin + t * ray.direction;
//        bestHit.normal = normalize(bestHit.position - vec4(sphere.xyz, 1.0f));
//        bestHit.albedo = vec4(1.0f);
//        bestHit.specular = vec3(0.3f);
//        return true;
//    }
//    return false;
        
    vec3 sphereDir = sphere.xyz - ray.origin.xyz;
    float tca = dot(sphereDir, ray.direction.xyz);
    float d2 = dot(sphereDir, sphereDir) - tca * tca;
    float radius2 = pow(sphere.w, 2);
    if (d2 > radius2)
        return false; 
    float thc = sqrt(radius2 - d2); 
    float t0 = tca - thc; 
    float t1 = tca + thc; 

    if (t0 < 0)
        t0 = t1; // if t0 is negative, let's use t1 instead 
 
    if (t0 > 0.0f && t0 < bestHit.t)
    {
        bestHit.t = t0;
        bestHit.position = ray.origin + t0 * ray.direction;
        bestHit.normal = normalize(bestHit.position - vec4(sphere.xyz, 1.0f));
        return true;
    }
    return false;
}

const float EPSILON = 1e-8;

bool IntersectCylinder(Ray ray, inout RayHit bestHit, vec3 A, vec3 B, float R)
{
//    vec3 AB = B - A;
//    vec3 AO = ray.origin.xyz - A;
//    vec3 AOxAB = cross(AO, AB); // cross product
//    vec3 VxAB = cross(ray.direction.xyz, AB); // cross product
//    float ab2 = dot(AB, AB); // dot product
//    float a = dot(VxAB, VxAB); // dot product
//    float b = 2 * dot(VxAB, AOxAB); // dot product
//    float c = dot(AOxAB, AOxAB) - (R * R * ab2);
//    float D = b * b - 4 * a * c;
//
//    if (D < 0)
//        return false;
//
//    D = sqrt(D);
//    float denom = 1 / (2 * a);
//    float t0 = (-b + D) * denom;
//    float t1 = (-b - D) * denom;
//
//    if (t0 < 0)
//        t0 = t1; // if t0 is negative, let's use t1 instead
//        
//    vec4 P = ray.origin + t0 * ray.direction;
//    vec3 AP = P.xyz - A;
//    float s = dot(AP, AB) / dot(AB, AB);
//    if (t0 > 0.0f && t0 < bestHit.t && s >= 0.0f && s <= 1.0f) 
//    {
//        bestHit.t = t0;
//        bestHit.position = P;
//        bestHit.normal = normalize(P - (vec4(A, 1.0f) + s * vec4(AB, 0.0f)));
//        bestHit.albedo = vec4(1.0f);
//        bestHit.specular = vec3(0.3f);
//        return true;
//    }
//    return false;
    
    vec3 V = normalize(B - A);
    vec3 X = ray.origin.xyz - A;
    float DD = dot(ray.direction.xyz, ray.direction.xyz);
    float DV = dot(ray.direction.xyz, V);
    float DX = dot(ray.direction.xyz, X);
    float XX = dot(X, X);
    float XV = dot(X, V);
    float a = DD - pow(DV, 2); // dot product
    float b = 2 * (DX - DV * XV); // dot product
    float c = XX - pow(XV, 2) - pow(R, 2);
    
    float D = pow(b, 2) - 4 * a * c;

    if (D < 0)
        return false;

    D = sqrt(D);
    float denom = 1 / (2 * a);
    float x0 = (-b - D) * denom;
    float x1 = (-b - D) * denom;
    float t0 = min(x0, x1);
    float t1 = max(x0, x1);

    if (t0 < 0.0f)
        t0 = t1;

    float s = DV * t0 + XV;
    if (t0 > 0.0f && t0 < bestHit.t && s >= 0.0f && s <= length(B - A)) 
    {
        bestHit.t = t0;
        bestHit.position = ray.origin + t0 * ray.direction;
        bestHit.normal = normalize(bestHit.position - vec4(A, 1.0f) - s * vec4(V, 0.0f));
        bestHit.albedo = vec4(1.0f);
        bestHit.specular = vec3(0.3f);
        return true;
    }
    return false;

}

bool IntersectSelectedObject(Ray ray, inout RayHit bestHit, vec4 minBound, vec4 maxBound, float radius)
{
//    vec3 p[8];
//    p[0] = minBound.xyz;
//    p[1] = vec3(maxBound.x, minBound.y, minBound.z);
//    p[2] = vec3(maxBound.x, minBound.y, maxBound.z);
//    p[3] = vec3(minBound.x, minBound.y, maxBound.z);
//    p[4] = vec3(minBound.x, maxBound.y, minBound.z);
//    p[5] = vec3(maxBound.x, maxBound.y, minBound.z);
//    p[6] = vec3(maxBound.x, maxBound.y, maxBound.z);
//    p[7] = vec3(minBound.x, maxBound.y, maxBound.z);
//    vec3 lastBotPoint = p[3];
//    vec3 lastTopPoint = p[7];
//    bool hit = false;
//    for (int i = 0; i < 4; i++)
//    {
////        hit = hit || IntersectCylinder(ray, bestHit, p[i], lastBotPoint, radius);
////        hit = hit || IntersectCylinder(ray, bestHit, p[i + 4], lastTopPoint, radius);
////        hit = hit || IntersectCylinder(ray, bestHit, p[i], p[i + 4], radius);
////        hit = hit || IntersectSphere(ray, bestHit, vec4(p[i], radius));
////        hit = hit || IntersectSphere(ray, bestHit, vec4(p[i + 4], radius));
//
//        lastBotPoint = p[i];
//        lastTopPoint = p[i + 4];
//    }
    vec3 p[8];
    p[0] = minBound.xyz;
    p[1] = vec3(maxBound.x, minBound.y, minBound.z);
    p[2] = vec3(maxBound.x, minBound.y, maxBound.z);
    p[3] = vec3(minBound.x, minBound.y, maxBound.z);
    p[4] = vec3(minBound.x, maxBound.y, minBound.z);
    p[5] = vec3(maxBound.x, maxBound.y, minBound.z);
    p[6] = vec3(maxBound.x, maxBound.y, maxBound.z);
    p[7] = vec3(minBound.x, maxBound.y, maxBound.z);
    vec3 lastBotPoint = p[3];
    vec3 lastTopPoint = p[7];
    for (int i = 0; i < 4; i++)
    {
        IntersectCylinder(ray, bestHit, p[i], lastBotPoint, radius);
        IntersectCylinder(ray, bestHit, p[i + 4], lastTopPoint, radius);
        IntersectCylinder(ray, bestHit, p[i], p[i + 4], radius);
        IntersectSphere(ray, vec4(lastBotPoint, radius), bestHit);
        IntersectSphere(ray, vec4(lastTopPoint, radius), bestHit);

        lastBotPoint = p[i];
        lastTopPoint = p[i + 4];
    }
    return true;
}

vec3 minVec3(vec3 a, vec3 b)
{
    return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

vec3 maxVec3(vec3 a, vec3 b)
{
    return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

float min3(vec3 v)
{
    return min(min(v.x, v.y), v.z);
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

bool IntersectAABB(Ray ray, vec4 minBound, vec4 maxBound, out vec2 nearFar)
{
    vec3 invDir = 1.0f / ray.direction.xyz;
    vec3 originMinBound = minBound.xyz - ray.origin.xyz;
    vec3 originMaxBound = maxBound.xyz - ray.origin.xyz;
    vec3 t0 = originMinBound * invDir;
    vec3 t1 = originMaxBound * invDir;

    nearFar = vec2(max3(minVec3(t0,t1)), min3(maxVec3(t0,t1)));

    return nearFar.x <= nearFar.y && nearFar.y >= 0.0f;
}

bool IntersectTriangle(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2,
                       inout float t, inout float u, inout float v)
{
    // find vectors for two edges sharing vert0
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    // begin calculating determinant - also used to calculate U parameter
    vec3 pvec = cross(ray.direction.xyz, edge2);
    // if determinant is near zero, ray lies in plane of triangle
    float det = dot(edge1, pvec);
    // use backface culling
    if (det < EPSILON)
        return false;
    float inv_det = 1.0f / det;
    // calculate distance from vert0 to ray origin
    vec3 tvec = ray.origin.xyz - vert0;
    // calculate U parameter and test bounds
    u = dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f)
        return false;
    // prepare to test V parameter
    vec3 qvec = cross(tvec, edge1);
    // calculate V parameter and test bounds
    v = dot(ray.direction.xyz, qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f)
        return false;
    // calculate t, ray intersects triangle
    t = dot(edge2, qvec) * inv_det;
    return true;
}

struct Triangle2 {
    vec3 v0;
    vec3 v1;
    vec3 v2;
};
Triangle2 createDummyTriangle()
{
    Triangle2 t;
    t.v0 = vec3(FLT_MAX);
    t.v1 = vec3(FLT_MAX);
    t.v2 = vec3(FLT_MAX);
    return t;
}

bool IntersectTriangles(Ray ray, Triangle2 triangles[4],
                        inout float t, inout float u,
                        inout float v, inout int i)
{
    t = FLT_MAX;
    vec3 e11 = triangles[0].v1 - triangles[0].v0;
    vec3 e21 = triangles[0].v2 - triangles[0].v0;
    vec3 e12 = triangles[1].v1 - triangles[1].v0;
    vec3 e22 = triangles[1].v2 - triangles[1].v0;
    vec3 e13 = triangles[2].v1 - triangles[2].v0;
    vec3 e23 = triangles[2].v2 - triangles[2].v0;
    vec3 e14 = triangles[3].v1 - triangles[3].v0;
    vec3 e24 = triangles[3].v2 - triangles[3].v0;

    vec4 v0x = vec4(triangles[0].v0.x, triangles[1].v0.x, triangles[2].v0.x, triangles[3].v0.x);
    vec4 v0y = vec4(triangles[0].v0.y, triangles[1].v0.y, triangles[2].v0.y, triangles[3].v0.y);
    vec4 v0z = vec4(triangles[0].v0.z, triangles[1].v0.z, triangles[2].v0.z, triangles[3].v0.z);
    vec4 e1x = vec4(e11.x, e12.x, e13.x, e14.x);
    vec4 e1y = vec4(e11.y, e12.y, e13.y, e14.y);
    vec4 e1z = vec4(e11.z, e12.z, e13.z, e14.z);
    vec4 e2x = vec4(e21.x, e22.x, e23.x, e24.x);
    vec4 e2y = vec4(e21.y, e22.y, e23.y, e24.y);
    vec4 e2z = vec4(e21.z, e22.z, e23.z, e24.z);
    vec4 dir4x = ray.direction.xxxx;
    vec4 dir4y = ray.direction.yyyy;
    vec4 dir4z = ray.direction.zzzz;
    vec4 pvecx = dir4y * e2z - dir4z * e2y;
    vec4 pvecy = dir4z * e2x - dir4x * e2z;
    vec4 pvecz = dir4x * e2y - dir4y * e2x;
    vec4 divisor = pvecx * e1x + pvecy * e1y + pvecz * e1z;
    vec4 invDivisor = vec4(1.0f, 1.0f, 1.0f, 1.0f) / divisor;
    vec4 orig4x = ray.origin.xxxx;
    vec4 orig4y = ray.origin.yyyy;
    vec4 orig4z = ray.origin.zzzz;
    vec4 tvecx = orig4x - v0x;
    vec4 tvecy = orig4y - v0y;
    vec4 tvecz = orig4z - v0z;
    vec4 uInternal = tvecx * pvecx + tvecy * pvecy + tvecz * pvecz;
    uInternal *= invDivisor;
    vec4 qvecx = tvecy * e1z - tvecz * e1y;
    vec4 qvecy = tvecz * e1x - tvecx * e1z;
    vec4 qvecz = tvecx * e1y - tvecy * e1x;
    vec4 vInternal = dir4x * qvecx + dir4y * qvecy + dir4z * qvecz;
    vInternal *= invDivisor;
    vec4 tInternal = e2x*qvecx + e2y*qvecy + e2z*qvecz;
    tInternal *= invDivisor;
    
    // uv conditions
    bvec4 cond0 = greaterThanEqual(uInternal, vec4(0.0f));
    bvec4 cond1 = greaterThanEqual(vInternal, vec4(0.0f));
    bvec4 cond2 = lessThanEqual(uInternal + vInternal, vec4(1.0f));
    // t > 0.0f
    bvec4 cond3 = greaterThan(tInternal, vec4(0.0f));
    // BackFace Culling (1 - on)
    ivec4 cond4 = ivec4(greaterThanEqual(divisor, vec4(EPSILON))) | ivec4(1);
    bvec4 finalCond = bvec4(ivec4(cond0) & ivec4(cond1) & ivec4(cond2) & ivec4(cond3) & cond4);

    if(finalCond.x && tInternal.x < t)
    {
        t = tInternal.x;
        u = uInternal.x;
        v = vInternal.x;
        i = 0;
    }
    
    if(finalCond.y && tInternal.y < t)
    {
        t = tInternal.y;
        u = uInternal.x;
        v = vInternal.x;
        i = 1;
    }
    
    if(finalCond.z && tInternal.z < t)
    {
        t = tInternal.z;
        u = uInternal.z;
        v = vInternal.z;
        i = 2;
    }
    
    if(finalCond.w && tInternal.w < t)
    {
        t = tInternal.w;
        u = uInternal.w;
        v = vInternal.w;
        i = 3;
    }

    return t != FLT_MAX;
}

// Node for storing state information during traversal.
struct BVHTraversal {
  int i; // Node
  float mint;
};

BVHTraversal createBVHTraversal(int i, float mint)
{
    BVHTraversal ret;
    ret.i = i;
    ret.mint = mint;
    return ret;
}

#define TRIANGLE 1
#define SPHERE 2
#define CYLINDER 3
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
            float t,u,v;

            if (IntersectTriangle(ray, v0, v1, v2, t, u, v))
            {
                // If we're only looking for occlusion, then any hit is good enough
                if(occlusion)
                    return true;
                // Otherwise, keep the closest intersection only
                if (t < intersection.t)
                {
                    intersection.t = t;
                    if (mesh.smoothing)
                    {
                        ivec3 nIndices = triangle.normals.xyz;
                        intersection.normal = (1 - u - v) * normals[nIndices.x] + u * normals[nIndices.y] + v * normals[nIndices.z];
                    }
                    else
                        intersection.normal = vec4(normalize(cross(v1 - v0, v2 - v0)), 0.0f);
                    intersection.albedo = vec4(0.5, 0.8, 0.9, 1.0);
                    intersection.specular = vec3(0.3f);
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
                intersection.albedo = vec4(1.0f);
                intersection.specular = vec3(0.3f);
                return true;
            }
            break;
        case CYLINDER:
            Cylinder cylinder = cylinders[primitive.index];
            if (IntersectCylinder(ray, intersection, cylinder.begin.xyz, cylinder.end.xyz, cylinder.radius))
            {
                // If we're only looking for occlusion, then any hit is good enough
                if(occlusion)
                    return true;
                intersection.albedo = vec4(1.0f);
                intersection.specular = vec3(0.3f);
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
bool IntersectMesh(Mesh mesh, Ray ray, inout RayHit intersection, bool occlusion)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = mesh.bvhOffset;
    todo[stackptr].mint = -FLT_MAX;
    float tLast = intersection.t;

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        BVHNode node = meshBVHs[ni];

        // If this node is further than the closest found intersection, continue
        if(near > intersection.t)
            continue;

        // Is leaf -> Intersect
        if (node.rightOffset == 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Primitive primitive = primitives[mesh.primitiveOffset + node.start + o];
                if (IntersectPrimitive(ray, primitive, mesh, occlusion, intersection) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
            BVHNode leftChild = meshBVHs[ni + 1];
            BVHNode rightChild = meshBVHs[ni + node.rightOffset];

            bool hitc0 = IntersectAABB(ray, leftChild.minBound, leftChild.maxBound, bbhitsc0);
            bool hitc1 = IntersectAABB(ray, rightChild.minBound, rightChild.maxBound, bbhitsc1);

            // Did we hit both nodes?
            if(hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni+1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if(bbhitsc1.x < bbhitsc0.x)
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
                todo[++stackptr] = createBVHTraversal(other, bbhitsc1.x);

                // And now the closer (with overlap test)
                todo[++stackptr] = createBVHTraversal(closer, bbhitsc0.x);
            }
            else if (hitc0)
                todo[++stackptr] = createBVHTraversal(ni + 1, bbhitsc0.x);
            else if(hitc1)
                todo[++stackptr] = createBVHTraversal(ni + int(node.rightOffset), bbhitsc1.x);

        }
    }
    return intersection.t < tLast;
}

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
bool IntersectScene(Ray ray, inout RayHit intersection, bool occlusion)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = 0;
    todo[stackptr].mint = -FLT_MAX;
    float tLast = intersection.t;

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        BVHNode node = sceneBVH[ni];

        // If this node is further than the closest found intersection, continue
        if(near > intersection.t)
            continue;

        // Is leaf -> Intersect
        if (node.rightOffset == 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Mesh mesh = meshes[node.start + o];

                if (IntersectMesh(mesh, ray, intersection, occlusion) && occlusion)
                    return true;
            }
        }
        else
        { // Not a leaf
            BVHNode leftChild = sceneBVH[ni + 1];
            BVHNode rightChild = sceneBVH[ni + node.rightOffset];

            bool hitc0 = IntersectAABB(ray, leftChild.minBound, leftChild.maxBound, bbhitsc0);
            bool hitc1 = IntersectAABB(ray, rightChild.minBound, rightChild.maxBound, bbhitsc1);

            // Did we hit both nodes?
            if(hitc0 && hitc1)
            {
                // We assume that the left child is a closer hit...
                closer = ni+1;
                other = ni + int(node.rightOffset);

                // ... If the right child was actually closer, swap the relavent values.
                if(bbhitsc1.x < bbhitsc0.x)
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
                todo[++stackptr] = createBVHTraversal(other, bbhitsc1.x);

                // And now the closer (with overlap test)
                todo[++stackptr] = createBVHTraversal(closer, bbhitsc0.x);
            }
            else if (hitc0)
                todo[++stackptr] = createBVHTraversal(ni + 1, bbhitsc0.x);
            else if(hitc1)
                todo[++stackptr] = createBVHTraversal(ni + int(node.rightOffset), bbhitsc1.x);

        }
    }

//    // If we hit something,
    if(intersection.t < tLast)
        intersection.position = ray.origin + ray.direction * intersection.t;

    return intersection.t < tLast;
}

float localSeed = 0.0;
uniform float globalSeed;
float rand(vec2 seed)
{
    float result = fract(sin((globalSeed + localSeed) / 100.0f * dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
    localSeed += 1.0f + sin(globalSeed);
    return result;
}

RayHit Trace(Ray ray, bool occlusion)
{
    RayHit hit = CreateRayHit();
//    IntersectSelectedObject(ray, hit, sceneBVH[0].minBound, sceneBVH[0].maxBound, 0.01f);
    IntersectScene(ray, hit, occlusion);
    return hit;
}

vec3 correctGamma(vec3 color, float gamma)
{
    vec3 mapped = color / (color + vec3(1.0));
    return pow(mapped, vec3(1.0 / gamma));
}

const float PI = 3.14159265f;
const float invPI = 1.0f / PI;
const float GAMMA = 2.2;
vec3 sampleEnviroment(vec3 direction, float lod)
{
        float theta = acos(direction.y) * invPI;
        float phi = atan(direction.x, -direction.z) * -invPI * 0.5f;
        return textureLod(hdriTexture, vec2(phi, theta), lod).xyz;
}

vec3 pick_random_point_in_sphere(){
  float x0,x1,x2,x3,d2;
  do{
    x0=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x1=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x2=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x3=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    d2=x0*x0+x1*x1+x2*x2+x3*x3;
  } while(d2>1.0f);
  float scale = 1.0f / d2;
  return vec3(2*(x1*x3+x0*x2)*scale,
                  2*(x2*x3+x0*x1)*scale,
                  (x0*x0+x3*x3-x1*x1-x2*x2)*scale);
}

vec3 pick_random_point_in_semisphere(vec3 v){
  vec3 result=pick_random_point_in_sphere();
  if(dot(v, result)<0){
    result.x=-result.x;
    result.y=-result.y;
    result.z=-result.z;
  }
  return normalize(result);
}


vec3 Shade(inout Ray ray, RayHit hit)
{
    vec3 color;
    if (hit.t < FLT_MAX)
    {
        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);

//        vec4 directionalLight = vec4(-2.0, 5.0, 10.0, 1.0);
//        vec3 lightPosition = vec3(2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.x, 2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.y, directionalLight.z);
//        vec3 shadowRayDirection = normalize(lightPosition - ray.origin.xyz);
        vec3 lightDirection = pick_random_point_in_semisphere(hit.normal.xyz);
        vec3 shadowRayDirection = lightDirection;

        // Return nothing// Shadow test ray
//        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, vec4(shadowRayDirection, 0.0));
//        RayHit shadowRayHit = CreateRayHit();
//        if (Trace(shadowRay, shadowRayHit, true))
//        {
//            return vec3(0.0f);
//        }
//        return hit.albedo.xyz;
        vec3 lightColor = sampleEnviroment(lightDirection, 2.0);
////        vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
        color = ray.energy.xyz * clamp(dot(hit.normal.xyz, shadowRayDirection), 0.0, 1.0) * lightColor * hit.albedo.xyz;
        ray.energy *= vec4(hit.specular, 1.0f);
//        return clamp(dot(hit.normal.xyz, shadowRayDirection), 0.0, 1.0) * directionalLight.w * hit.albedo.xyz;
    }
    else
    {
        color = sampleEnviroment(ray.direction.xyz, 0);
    }
    return correctGamma(color, GAMMA);
}

uniform float colorMultiplier;
void main()
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);

    if (storePos.x < size.x && storePos.y < size.y)
    {
        Ray ray = CreateCameraRay(vec2(storePos) + vec2(rand(vec2(storePos)), rand(vec2(storePos))), size);
        vec4 result = imageLoad(destTex, storePos) * colorMultiplier;

        for (int i = 0; i < 1; i++)
        {
            RayHit hit = Trace(ray, false);
            result += vec4(Shade(ray, hit), 0.0f);
            if (hit.t == FLT_MAX || all(equal(ray.energy, vec4(0.0f))))
            {
                break;
            }
        }
        imageStore(destTex, storePos, result + vec4(0.0f, 0.0f, 0.0f, 1.0f));
//        RayHit bestHit = CreateRayHit();
//        if (IntersectSelectedObject(ray, bestHit, vec3(0,0,0), vec3(1,1,1), 0.01f))
//            imageStore(destTex, storePos, vec4(1.0f, 1.0f, 1.0f, 1.0f));
//        else
//            imageStore(destTex, storePos, vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }

}
