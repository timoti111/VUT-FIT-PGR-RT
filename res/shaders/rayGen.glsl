#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
layout(local_size_x = 16, local_size_y = 16) in;

struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 left;
    float sensorHalfWidth;
};
layout(binding = 0) uniform Camera camera;

struct Ray
{
    vec4 origin;
    vec4 direction;
};
layout(std430, binding = 1) buffer PathStateBuffer
{
    Ray rays[];
};

Ray CreateRay(vec4 origin, vec4 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

Ray CreateCameraRay(vec2 pixel, ivec2 size)
{
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5);
    vec4 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    return CreateRay(camera.position, direction);
}

float localSeed = 0.0;
uniform float globalSeed;
float rand(vec2 seed)
{
    float result = fract(sin((globalSeed + localSeed) / 100.0f * dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
    localSeed += 1.0f + globalSeed;
    return result;
}

uniform ivec2 resolution;
void main()
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    if (storePos.x < resolution.x && storePos.y < resolution.y)
        rays[storePos.y * resolution.x + storePos.x] = CreateCameraRay(vec2(storePos) + vec2(rand(vec2(storePos)), rand(vec2(storePos))), resolution);
}