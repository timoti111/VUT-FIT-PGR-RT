uniform Camera camera;

layout(rgba32f, binding = 1) uniform image2D destTex;
layout(binding = 2) uniform sampler2D hdriTexture;

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

layout(binding = 0, offset = 0) uniform atomic_uint newPathCounter;
layout(std430, binding = 13) buffer NewPathBuffer
{
    int newPathIndices[];
};
layout(binding = 0, offset = 4) uniform atomic_uint extensionRayCounter;
layout(std430, binding = 14) buffer ExtRayCastBuffer
{
    int extRayCastPaths[];
};
layout(binding = 0, offset = 8) uniform atomic_uint shadowRayCounter;
layout(std430, binding = 15) buffer ShadowRayCastBuffer
{
    int shadowRayPaths[];
};
layout(binding = 0, offset = 12) uniform atomic_uint material0Counter;
layout(std430, binding = 16) buffer MaterialBuffer
{
    int material0Paths[];
};
