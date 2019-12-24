#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueCounters[NEW_PATH_QUEUE])
        return;

    uint pathIndex = newPathQueue[globalInvocationID];

    ivec2 size = imageSize(destTex);
    uint localSeed = GetPathInfo(pathIndex, seed);
    uint pathPixelIndex = (currentPixelCount + globalInvocationID) % (size.x * size.x);
    ivec2 storePos = ivec2(pathPixelIndex % size.x, pathPixelIndex / size.x);
    SetPathInfo(pathIndex, pixelIndex, storePos);
    vec2 pixel = vec2(storePos) + vec2(rand(localSeed), rand(localSeed));
    Camera camera = renderParameters.camera;
    float sensorHalfWidth = camera.sensorHalfWidth * camera.focusDistance;
    float sensorHalfHeight = (size.y * sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 zeroVec = vec4(0.0f);
    vec4 pixelPos = camera.position + camera.direction * camera.focusDistance + camera.left * (sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight);
    vec2 uniformDiscSample = sampleUniformDisc(localSeed);
    vec4 randomOffset = camera.left * uniformDiscSample.x + camera.up * uniformDiscSample.y;
    vec4 randomPosOnAperture = camera.position + 0.005 * camera.aperture * randomOffset;
    SetPathInfo(pathIndex, dir, normalize(vec4((pixelPos - randomPosOnAperture).xyz, 0.0f)));
    SetPathInfo(pathIndex, orig, randomPosOnAperture);
    SetPathInfo(pathIndex, Ei, zeroVec);
    SetPathInfo(pathIndex, T, vec4(1.0f));
    SetPathInfo(pathIndex, pathLen, 0);
    SetPathInfo(pathIndex, lastSpecular, false);
    SetPathInfo(pathIndex, lastPdfDirect, 1.0f);
    SetPathInfo(pathIndex, lastPdfIndirect, 1.0f);
    SetPathInfo(pathIndex, lastLightPickProb, 1.0f);
    SetPathInfo(pathIndex, shadowRayBlocked, true);
    SetPathInfo(pathIndex, lightHit, false);
    SetPathInfo(pathIndex, seed, localSeed);

    uint extensionRayIndex = atomicWarpAdd(EXTENSION_RAY_QUEUE, 1);
    extensionRayQueue[extensionRayIndex] = pathIndex;
}