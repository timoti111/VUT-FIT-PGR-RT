#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= atomicCounter(newPathCounter))
        return;

    uint pathIndex = newPathIndices[globalInvocationID];
    
    ivec2 size = imageSize(destTex);
    uint pathPixelIndex = atomicCounterIncrement(newPixelIndex) % (size.x * size.x);
    ivec2 storePos = ivec2(pathPixelIndex % size.x, pathPixelIndex / size.x);
    pathStates[pathIndex].pixelIndex = storePos;
    vec2 pixel = vec2(storePos) + vec2(rand(vec2(storePos)), rand(vec2(storePos)));
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    pathStates[pathIndex].primDir = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    pathStates[pathIndex].primOrig = camera.position;
    pathStates[pathIndex].color = vec4(0.0f);
    pathStates[pathIndex].t = FLT_MAX;
    pathStates[pathIndex].matID = -1;
    pathStates[pathIndex].state = TRACING;

    uint extensionRayIndex = atomicCounterIncrement(extensionRayCounter);
    extRayCastPaths[extensionRayIndex] = pathIndex;
}