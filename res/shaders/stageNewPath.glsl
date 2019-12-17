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
    uint pathPixelIndex = (currentPixelCount + globalInvocationID) % (size.x * size.x);
    ivec2 storePos = ivec2(pathPixelIndex % size.x, pathPixelIndex / size.x);
    pathStates[pathIndex].pixelIndex = storePos;
    vec2 pixel = vec2(storePos) + vec2(rand(pathStates[pathIndex].seed), rand(pathStates[pathIndex].seed));
    Camera camera = renderParameters.camera;
    float sensorHalfWidth = camera.sensorHalfWidth * camera.focusDistance;
    float sensorHalfHeight = (size.y * sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 zeroVec = vec4(0.0f);
    vec4 pixelPos = camera.position + camera.direction * camera.focusDistance + camera.left * (sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight);
    vec2 uniformDiscSample = sampleUniformDisc(pathStates[pathIndex].seed);
    vec4 randomOffset = camera.left * uniformDiscSample.x + camera.up * uniformDiscSample.y;
    vec4 randomPosOnAperture = camera.position + 0.005 * camera.aperture * randomOffset;
    pathStates[pathIndex].dir = normalize(vec4((pixelPos - randomPosOnAperture).xyz, 0.0f));
    pathStates[pathIndex].orig = randomPosOnAperture;
    pathStates[pathIndex].Ei = zeroVec;
    pathStates[pathIndex].T = vec4(1.0f);
    pathStates[pathIndex].pathLen = 0;
    pathStates[pathIndex].lastSpecular = false;
    pathStates[pathIndex].lastPdfDirect = 1.0f;
    pathStates[pathIndex].lastPdfIndirect = 1.0f;
    pathStates[pathIndex].lastLightPickProb = 1.0f;
    pathStates[pathIndex].shadowRayBlocked = true;
    pathStates[pathIndex].lightHit = false;

    uint extensionRayIndex = atomicWarpAdd(EXTENSION_RAY_QUEUE, 1);
    extensionRayQueue[extensionRayIndex] = pathIndex;
}