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
};

Ray CreateRay(vec4 origin, vec4 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

struct RayHit
{
    vec4 position;
    vec4 normal;
    vec2 uv;
    float t;
    int matID;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.t = FLT_MAX;
    hit.matID = -1;
    return hit;
};

struct PathState
{
    vec4 primOrig;
    vec4 primDir;
    vec4 shadOrig;
    vec4 shadDir;
    vec4 color;
    vec4 hitPos;
    vec4 hitDir;
    vec4 hitNorm;
    vec2 hitUV;
    ivec2 pixelIndex;

    float t;
    int state;
    int matID;
    int pad1;
};

struct RenderParameters
{
    Camera camera;
    int maxBounces;
    bool useEnvironmentMap;
};
