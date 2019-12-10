// UNIFORMS
layout(binding = 0) uniform RenderParametersBuffer
{
    RenderParameters renderParameters;
};
layout(rgba32f, binding = 1) uniform image2D destTex;
layout(binding = 2) uniform sampler2D hdriTexture;

// BUFFERS
layout(std430, binding = 0) buffer QueueLengthsBuffer
{
    QueueLengths queueLengths;
};
layout(std430, binding = 1) buffer PathStatesBuffer
{
    PathState pathStates[];
};
layout(std430, binding = 2) buffer NewPathBuffer
{
    uint newPathIndices[NUM_PATHS];
    uint extRayCastPaths[NUM_PATHS];
    uint shadowRayCastPaths[NUM_PATHS];
    uint basicMaterialPaths[NUM_PATHS];
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
layout(std430, binding = 13) buffer MaterialsBuffer
{
    Material materials[];
};
