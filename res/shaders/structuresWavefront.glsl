struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 left;
    float sensorHalfWidth;
    float focusDistance;
    float aperture;
};

struct BVHNode
{
    vec3 minBound;
    uint nPrims;
    vec3 maxBound;
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
    vec4 vertices[3];
    vec4 normals[3];
    vec2 coords[3];
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
    int triIndex;
};

struct PathState
{
    vec4 orig;
    vec4 dir;
    vec4 hitP;
    vec4 hitN;
    vec4 shadowOrig;
    vec4 shadowDir;
    vec4 T;
    vec4 lastT;
    vec4 Ei;
    vec4 lastBsdfDirect;
    vec4 lastEmission;
    ivec2 pixelIndex;
    vec2 hitUV;
    float t;
    int matID;
    int triIndex;
    float maxShadowRayLen;
    bool shadowRayBlocked;
    bool lightHit;
    bool lastSpecular;
    float lastPdfDirect;
    float lastCosThDirect;
    float lastPdfIndirect;
    float lastLightPickProb;
    uint seed;
    uint pathLen;
    int pad[3];
};

struct RenderParameters
{
    Camera camera;
    vec4 backgroundColor;
    float backgroundIntensity;
    int maxBounces;
    bool sampleDirect;
    bool useEnvironmentMap;
    bool useRussianRoulette;
    uint environmentMapTextureID;
    uint numberOfLights;
};

struct QueueLengths
{
    uint newPathQueueLen;
    uint materialQueueLen;
    uint extensionRayQueueLen;
    uint shadowRayQueueLen;
};

struct Material
{
    vec4 Kd;     // diffuse reflectivity
    vec4 Ks;     // specular reflectivity 
    vec4 Ke;     // emission
    float Ns;   // specular exponent (shininess), normally in [0, 1000]
    float Ni;   // index of refraction
    int map_Kd; // diffuse texture descriptor idx
    int map_Ks; // specular texture descriptor idx
    int map_N;  // normal texture descriptor idx
    int type;   // BXDF type, defined in bxdf.cl
};

struct Light
{
    vec4 sphere;
    int materialID;
};
