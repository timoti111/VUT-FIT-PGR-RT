#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl

layout(local_size_x = 256) in;
uniform float colorMultiplier;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID == 0)
    {
        atomicCounterExchange(newPathCounter, 0);
        atomicCounterExchange(extensionRayCounter, 0);
        atomicCounterExchange(shadowRayCounter, 0);
        atomicCounterExchange(basicMaterialCounter, 0);
    }
    if (globalInvocationID >= pathStates.length())
        return;

    int pathState = pathStates[globalInvocationID].state;

    if(pathState == UNINITIALIZED)
    {
        uint index = atomicCounterIncrement(newPathCounter);
        newPathIndices[index] = globalInvocationID;
    }

    if(pathState == TRACING)
    {
        if (pathStates[globalInvocationID].matID < 100)
        {
            uint index = atomicCounterIncrement(basicMaterialCounter);
            basicMaterialPaths[index] = globalInvocationID;
        }
    }
    
    if(pathState == TERMINATED)
    {
        ivec2 pixelIndex = pathStates[globalInvocationID].pixelIndex;
        vec4 result = imageLoad(destTex, pixelIndex) * colorMultiplier;
        result += vec4(pathStates[globalInvocationID].color.xyz, 1.0f);
        imageStore(destTex, pixelIndex, result);
        uint index = atomicCounterIncrement(newPathCounter);
        newPathIndices[index] = globalInvocationID;
    }
}


