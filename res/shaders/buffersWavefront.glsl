// UNIFORMS
layout(binding = 1) uniform RenderParametersBuffer
{
    RenderParameters renderParameters;
};
layout(rgba32f, binding = 2) uniform image2D destTex;
layout(binding = 3) uniform sampler2D textures[32];

#define NEW_PATH_QUEUE 0
#define MATERIAL_QUEUE 1
#define EXTENSION_RAY_QUEUE 2
#define SHADOW_RAY_QUEUE 3
layout(binding = 0) buffer PathsDataBuffer
{
    uint queueCounters[4];
    uint currentPixelCount;
    uint newPathQueue[NUM_PATHS];
    uint materialQueue[NUM_PATHS];
    uint extensionRayQueue[NUM_PATHS];
    uint shadowRayQueue[NUM_PATHS];
    // AOS
//    PathState pathStates[];
    // SOA
    vec4 orig[NUM_PATHS];
    vec4 dir[NUM_PATHS];
    vec4 hitP[NUM_PATHS];
    vec4 hitN[NUM_PATHS];
    vec4 shadowOrig[NUM_PATHS];
    vec4 shadowDir[NUM_PATHS];
    vec4 T[NUM_PATHS];
    vec4 lastT[NUM_PATHS];
    vec4 Ei[NUM_PATHS];
    vec4 lastBsdfDirect[NUM_PATHS];
    vec4 lastEmission[NUM_PATHS];
    ivec2 pixelIndex[NUM_PATHS];
    vec2 hitUV[NUM_PATHS];
    float t[NUM_PATHS];
    int matID[NUM_PATHS];
    int triIndex[NUM_PATHS];
    float maxShadowRayLen[NUM_PATHS];
    bool shadowRayBlocked[NUM_PATHS];
    bool lightHit[NUM_PATHS];
    bool lastSpecular[NUM_PATHS];
    float lastPdfDirect[NUM_PATHS];
    float lastCosThDirect[NUM_PATHS];
    float lastPdfIndirect[NUM_PATHS];
    float lastLightPickProb[NUM_PATHS];
    uint seed[NUM_PATHS];
    uint pathLen[NUM_PATHS];
};


// AOS
//#define GetPathInfo(index, name) pathStates[index].name
//#define IncPathInfo(index, name) pathStates[index].name++
//#define IncByPathInfo(index, name, value) pathStates[index].name += value
//#define MulByPathInfo(index, name, value) pathStates[index].name *= value
//#define SetPathInfo(index, name, value) pathStates[index].name = value

// SOA
#define GetPathInfo(index, name) name[index]
#define IncPathInfo(index, name) name[index]++
#define IncByPathInfo(index, name, value) name[index] += value
#define MulByPathInfo(index, name, value) name[index] *= value
#define SetPathInfo(index, name, value) name[index] = value

layout(std430, binding = 1) buffer MaterialsBuffer
{
    Material materials[];
};
layout(std430, binding = 2) buffer SceneBVHBuffer
{
    BVHNode sceneBVH[];
};
layout(std430, binding = 3) buffer MeshBVHsBuffer
{
    BVHNode meshBVHs[];
};
layout(std430, binding = 4) buffer MeshesBuffer
{
    Mesh meshes[];
};
layout(std430, binding = 5) buffer PrimitivesBuffer
{
    Primitive primitives[];
};
layout(std430, binding = 6) buffer TrianglesBuffer
{
    Triangle triangles[];
};
layout(std430, binding = 7) buffer SpheresBuffer
{
    Sphere spheres[];
};
layout(std430, binding = 8) buffer CylindersBuffer
{
    Cylinder cylinders[];
};
layout(std430, binding = 9) buffer LightsBuffer
{
    Light lights[];
};
