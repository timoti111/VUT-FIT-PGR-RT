// UNIFORMS
layout(binding = 0) uniform RenderParametersBuffer
{
    RenderParameters renderParameters;
};
layout(rgba32f, binding = 1) uniform image2D destTex;
layout(binding = 2) uniform sampler2D textures[20];
//layout(binding = 2) uniform sampler2D hdriTexture[5];

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
    PathState pathStates[];
};
layout(std430, binding = 1) buffer MaterialsBuffer
{
    Material materials[];
};
layout(std430, binding = 3) buffer SceneBVHBuffer
{
    BVHNode sceneBVH[];
};
layout(std430, binding = 4) buffer MeshBVHsBuffer
{
    BVHNode meshBVHs[];
};
layout(std430, binding = 5) buffer MeshesBuffer
{
    Mesh meshes[];
};
layout(std430, binding = 6) buffer PrimitivesBuffer
{
    Primitive primitives[];
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
layout(std430, binding = 11) buffer SpheresBuffer
{
    Sphere spheres[];
};
layout(std430, binding = 12) buffer CylindersBuffer
{
    Cylinder cylinders[];
};
