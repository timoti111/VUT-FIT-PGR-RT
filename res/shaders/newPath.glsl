#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;
uniform uint newPixelIndex;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueLengths.newPathCounter)
        return;

    uint pathIndex = newPathIndices[globalInvocationID];
    
    ivec2 size = imageSize(destTex);
    uint pathPixelIndex = (newPixelIndex + globalInvocationID) % (size.x * size.x);
    ivec2 storePos = ivec2(pathPixelIndex % size.x, pathPixelIndex / size.x);
    pathStates[pathIndex].pixelIndex = storePos;
    vec2 pixel = vec2(storePos) + vec2(rand(vec2(storePos)), rand(vec2(storePos)));
    Camera camera = renderParameters.camera;
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 zeroVec = vec4(0.0f);
    vec4 oneVec = vec4(1.0f);
    pathStates[pathIndex].primDir = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    pathStates[pathIndex].primOrig = camera.position;
    pathStates[pathIndex].color = zeroVec;
    pathStates[pathIndex].throughput = oneVec;
    pathStates[pathIndex].hitPos = zeroVec;
    pathStates[pathIndex].hitNorm = zeroVec;
    pathStates[pathIndex].hitUV = vec2(0.0f);
    pathStates[pathIndex].matID = -1;
    pathStates[pathIndex].t = FLT_MAX;
    pathStates[pathIndex].shadowRayT = FLT_MAX;
    pathStates[pathIndex].shadowRayBlocked = false;
    pathStates[pathIndex].pathLen = 0;
    pathStates[pathIndex].meshIndex = -1;

    uint extensionRayIndex = atomicAdd(queueLengths.extensionRayCounter, 1);
    extRayCastPaths[extensionRayIndex] = pathIndex;
}