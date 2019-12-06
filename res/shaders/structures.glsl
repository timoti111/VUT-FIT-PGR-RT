struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 left;
    float sensorHalfWidth;
};

struct BVHNode
{
    vec4 minBound;
    vec4 maxBound;
    uint start;
    uint nPrims;
    uint rightOffset;
};

struct Mesh
{
    mat4x4 objectToWorld;
    int primitiveOffset;
    int bvhOffset;
    int materialID;
    bool smoothing;
};

struct Primitive 
{
    int type;
    int index;
};

struct Triangle
{
    ivec4 vertices;
    ivec4 normals;
    ivec4 coords;
};

struct Sphere
{
    vec4 sphere;
};

struct Cylinder
{
    vec4 begin;
    vec4 end;
    float radius;
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
