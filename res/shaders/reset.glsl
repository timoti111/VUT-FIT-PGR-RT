#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl

layout(local_size_x = 256) in;
uniform bool firstIteration;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    ivec2 texSize = imageSize(destTex);
    ivec2 storePos = ivec2(globalInvocationID % texSize.x, globalInvocationID / texSize.x);
    if (storePos.y < texSize.y)
        imageStore(destTex, storePos, vec4(0.0f));

    uint maxID = firstIteration ? min(texSize.x * texSize.y, pathStates.length()) : pathStates.length();
    if (globalInvocationID >= maxID)
        return;
    pathStates[globalInvocationID].seed = globalInvocationID;

    newPathIndices[globalInvocationID] = globalInvocationID;
    if (globalInvocationID == 0)
        queueLengths.newPathCounter = maxID;
}


