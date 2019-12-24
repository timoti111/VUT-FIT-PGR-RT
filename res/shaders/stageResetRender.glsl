#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl

layout(local_size_x = 32) in;
uniform bool firstIteration;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID == 0)
    {
        queueCounters[NEW_PATH_QUEUE] = 0;
        queueCounters[MATERIAL_QUEUE] = 0;
        queueCounters[EXTENSION_RAY_QUEUE] = 0;
        queueCounters[SHADOW_RAY_QUEUE] = 0;
    }
    ivec2 texSize = imageSize(destTex);
    ivec2 storePos = ivec2(globalInvocationID % texSize.x, globalInvocationID / texSize.x);
    if (storePos.y < texSize.y)
        imageStore(destTex, storePos, vec4(0.0f));

    if (globalInvocationID >= NUM_PATHS)
        return;
    SetPathInfo(globalInvocationID, seed, globalInvocationID);

    newPathQueue[globalInvocationID] = globalInvocationID;
    if (globalInvocationID == 0)
    {
        queueCounters[NEW_PATH_QUEUE] = NUM_PATHS;
        currentPixelCount = 0;
    }
}



