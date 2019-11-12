#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

layout(local_size_x = 64) in;

struct Ray
{
    vec3 origin;
    int rayID;
    vec3 direction;
};
layout(std430, binding = 1) buffer RaysBuffer
{
    Ray rays[];
};

struct IntersectionResult
{
    vec3 normal;
    int materialID;
    vec3 position;
    int rayID;
    vec2 textureUV;
};
layout(std430, binding = 1) buffer IntersectionResultsBuffer
{
    IntersectionResult intersections[];
};

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

struct Primitive 
{
    int type;
    int index;
    int meshIndex;
};
layout(std430, binding = 5) buffer PrimitivesBuffer
{
    Primitive primitives[];
};

struct Triangle
{
    ivec4 vertices;
    ivec4 normals;
    ivec4 coords;
};
layout(std430, binding = 6) buffer TrianglesBuffer
{
    Triangle triangles[];
};

layout(std430, binding = 7) buffer VerticesBuffer
{
    vec4 vertices[];
};

layout(std430, binding = 8) buffer NormalsBuffer
{
    vec4 normals[];
};

layout(std430, binding = 9) buffer CoordsBuffer
{
    vec2 coords[];
};

IntersectionResult createIntersectionResult(int rayID)
{
    IntersectionResult intersection;
    intersection.normal = vec3(0.0f);
    intersection.materialID = -1;
    intersection.position = vec3(0.0f);
    intersection.rayID = rayID;
    intersection.textureUV = vec2(0.0f);
    return intersection;
}

bool IntersectGroundPlane(Ray ray, inout float t,
                          out IntersectionResult intersection)
{
//     Calculate distance along the ray where the ground plane is intersected
    float tNew = -ray.origin.y / ray.direction.y;
    if (tNew > 0.0f && tNew < t)
    {
        t = tNew;
        intersection.normal = vec3(0.0f, 1.0f, 0.0f);
        intersection.materialID = 1;
        return true;
    }
    return false;
}

bool IntersectSphere(Ray ray, vec4 sphere, inout float t,
                     out IntersectionResult intersection)
{
    // Calculate distance along the ray where the sphere is intersected
    vec3 d = ray.origin - sphere.xyz;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + pow(sphere.w, 2);
    if (p2sqr <  0.0f)
        return false;
    float p2 = sqrt(p2sqr);
    float tNew = p1 - p2 > 0.0f ? p1 - p2 : p1 + p2;
    // TODO UV
    if (tNew > 0.0f && tNew < t)
    {
        t = tNew;
        // todo UV
        // todo normal
        return true;
    }
    return false;
    return t > 0.0f;
}

const float EPSILON = 1e-8;
bool IntersectTriangle(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2, inout float t,
                       out IntersectionResult intersection)
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
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f)
        return false;
    // prepare to test V parameter
    vec3 qvec = cross(tvec, edge1);
    // calculate V parameter and test bounds
    float v = dot(ray.direction.xyz, qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f)
        return false;
    // calculate t, ray intersects triangle
    float tNew = dot(edge2, qvec) * inv_det;
    if (tNew > 0.0f && tNew < t)
    {
        t = tNew;
        intersection.textureUV = vec2(u,v);
        return true;
    }
    return false;
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
    vec3 t0 = (minBound.xyz - ray.origin.xyz) * invDir;
    vec3 t1 = (maxBound.xyz - ray.origin.xyz) * invDir;

    nearFar = vec2(max3(minVec3(t0,t1)), min3(maxVec3(t0,t1)));

    return nearFar.x <= nearFar.y && nearFar.y >= 0.0f;
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

// - Compute the nearest intersection of all objects within the tree.
// - Return true if hit was found, false otherwise.
// - In the case where we want to find out of there is _ANY_ intersection at all,
//   set occlusion == true, in which case we exit on the first hit, rather
//   than find the closest.
void IntersectScene(Ray ray, inout float t, inout IntersectionResult intersection)
{
    vec2 bbhitsc0, bbhitsc1;
    int closer, other;

    // Working set
    BVHTraversal todo[64];
    int stackptr = 0;

    // "Push" on the root node to the working set
    todo[stackptr].i = 0;
    todo[stackptr].mint = -FLT_MAX;

    Primitive lastPrimitive;

    while(stackptr >= 0)
    {
        // Pop off the next node to work on.
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;
        BVHNode node = sceneBVH[ni];

        // If this node is further than the closest found intersection, continue
        if(near > t)
            continue;

        // Is leaf -> Intersect
        if (node.rightOffset == 0)
        {
            for(int o = 0; o < node.nPrims; ++o)
            {
                Primitive primitive = primitives[node.start + o];
                Triangle triangle = triangles[primitive.index];
                ivec3 vIndexes = triangle.vertices.xyz;
                vec3 v0 = vertices[vIndexes.x].xyz;
                vec3 v1 = vertices[vIndexes.y].xyz;
                vec3 v2 = vertices[vIndexes.z].xyz;
                bool hit = IntersectTriangle(ray, v0, v1, v2, t, intersection);

                if (hit)
                {
                    lastPrimitive = primitive;
                    intersection.normal = normalize(cross(v1 - v0, v2 - v0));
                }
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
}

void Trace(Ray ray, inout IntersectionResult intersection)
{
    float t = FLT_MAX;
    IntersectGroundPlane(ray, t, intersection);
    IntersectScene(ray, t, intersection);

    if (t != FLT_MAX)
        intersection.position = ray.origin + ray.direction * t;
}

void main()
{   
    Ray ray = rays[int(gl_GlobalInvocationID.x)];
    IntersectionResult intersection = createIntersectionResult(ray.rayID);
    Trace(ray, intersection);
    intersections[gl_GlobalInvocationID.x] = intersection;
}
