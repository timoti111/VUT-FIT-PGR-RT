#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

struct Camera
{
    vec3 position;
    vec3 direction;
    vec3 up;
    vec3 left;
    float sensorHalfWidth;
};

uniform Camera camera;

struct Ray
{
    vec3 origin;
    float maxT;
    vec3 direction;
    int primitiveID;
    vec3 color;
};

Ray CreateRay(vec3 origin, vec3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.maxT = FLT_MAX;
    ray.direction = direction;
    ray.color = vec3(1.0);
    ray.primitiveID = -1;
    return ray;
}

Ray CreateCameraRay(vec2 pixel, ivec2 size)
{
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = camera.sensorHalfWidth / (size.x * 0.5);
    vec3 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - (pixel.x + 0.5) * pixelSize) + camera.up * ((pixel.y + 0.5) * pixelSize - sensorHalfHeight));
    return CreateRay(camera.position, direction);
}

layout(binding = 1) buffer RayBuffer
{
    Ray rays[];
};
layout(binding = 1) writeonly uniform image2D destTex;
layout(binding = 2) uniform sampler2D hdriTexture;
layout(local_size_x = 8, local_size_y = 8) in;
void main()
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);
    if (storePos.x >= size.x || storePos.y >= size.y)
    {
        return;
    }
    int index = storePos.y * size.x + storePos.x;
    Ray ray = CreateCameraRay(vec2(storePos.x, storePos.y), size);
    rays[index] = ray;
}
