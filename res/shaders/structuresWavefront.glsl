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
    vec4 shadowOrig;
    vec4 shadowDir;
    vec4 T;
    vec4 Ei;
    vec4 lastBsdf;
    vec4 lastEmission;
    vec4 lastT;
    bool shadowRayBlocked;
    float shadowRayLen;
    bool lastSpecular; // prevents NEE
    // Previously evaluated light sample
    float lastPdfW; // prev. brdf pdf, for MIS (implicit light samples)
    float lastPdfDirect;    // pdfW of sampled NEE sample
    float lastPdfImplicit;  // pdfW of implicit NEE sample
    float lastCosTh;
    float lastLightPickProb;
    
    ivec2 pixelIndex;
    uint seed;
    bool firstDiffuseHit;


    vec4 hitP;
    vec4 hitN;
    vec2 hitUV;
    float t;
    int matID;    // index of hit material
    int triIndex;    
    uint pathLen; // number of segments in path
        // index of hit triangle, -1 by default
    bool lightHit;
};

struct RenderParameters
{
    Camera camera;
    vec4 backgroundColor;
    float backgroundIntensity;
    int maxBounces;
    bool useEnvironmentMap;
    bool useRussianRoulette;
    uint environmentMapTextureID;
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
