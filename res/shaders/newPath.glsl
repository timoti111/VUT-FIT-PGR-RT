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
    vec2 pixel = vec2(storePos) + vec2(rand(pathStates[pathIndex].seed), rand(pathStates[pathIndex].seed));
    Camera camera = renderParameters.camera;
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 zeroVec = vec4(0.0f);
    pathStates[pathIndex].dir = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    pathStates[pathIndex].orig = camera.position;
    pathStates[pathIndex].Ei = zeroVec;
    pathStates[pathIndex].T = vec4(1.0f);
    pathStates[pathIndex].pathLen = 0;
    pathStates[pathIndex].firstDiffuseHit = false;
    pathStates[pathIndex].lastSpecular = true;
    pathStates[pathIndex].lastPdfW = 1.0f;
    pathStates[pathIndex].lastPdfImplicit = 0.0f;
    pathStates[pathIndex].lastCosTh = 0.0f;
    pathStates[pathIndex].lastLightPickProb = 1.0f;
    pathStates[pathIndex].shadowRayLen = FLT_MAX;
    pathStates[pathIndex].backfaceHit = false;
    pathStates[pathIndex].shadowRayBlocked = true;
    pathStates[pathIndex].lightHit = false;
    pathStates[pathIndex].lastEmission = zeroVec;
    pathStates[pathIndex].lastBsdf = zeroVec;
    WriteHit(EmptyHit(FLT_MAX), pathIndex);

    uint extensionRayIndex = atomicAdd(queueLengths.extensionRayCounter, 1);
    extRayCastPaths[extensionRayIndex] = pathIndex;
}