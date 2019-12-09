#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;
uniform bool firstIteration;
uniform uint currentPixelCount;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    ivec2 texSize = imageSize(destTex);

    uint maxID = firstIteration ? min(pathStates.length(), texSize.x * texSize.y) : pathStates.length();
    if (globalInvocationID >= maxID)
        return;

    int pathLen = pathStates[globalInvocationID].pathLen;
    int maxBounces = renderParameters.maxBounces;
    bool terminated = pathStates[globalInvocationID].pathLen - 1 >= maxBounces;
    terminated = terminated || pathStates[globalInvocationID].throughput == vec4(0.0f); 

    if (!terminated && pathStates[globalInvocationID].meshIndex < 0)
    {
        pathStates[globalInvocationID].color = sampleEnviroment(pathStates[globalInvocationID].primDir, 0.0f);
        terminated = true;
    }
//    if (pathStates[globalInvocationID].meshIndex >= 0/* && !pathStates[globalInvocationID].shadowRayBlocked*/)
//    {
//        vec4 lightDirection = pick_random_point_in_semisphere(pathStates[globalInvocationID].hitNorm.xyz);
//        vec4 lightColor = sampleEnviroment(lightDirection, 2.0);
//        pathStates[globalInvocationID].color += clamp(dot(pathStates[globalInvocationID].hitNorm.xyz, lightDirection.xyz), 0.0, 1.0) * lightColor * vec4(1.0f);
//        pathStates[globalInvocationID].throughput = vec4(0.0f);
//    }
//    
    if(terminated)
    {
        ivec2 pixelIndex = pathStates[globalInvocationID].pixelIndex;
        vec4 result = imageLoad(destTex, pixelIndex);
            
        if (firstIteration && result.w == 0.0f)
            result = vec4(pathStates[globalInvocationID].color.xyz, 1.0f);
        else
            result += vec4(pathStates[globalInvocationID].color.xyz, 1.0f);
        imageStore(destTex, pixelIndex, result);
        if (firstIteration)
        {
            uint currentIndex = atomicAdd(queueLengths.newPathCounter, 0);
            if (currentPixelCount + currentIndex >= texSize.x * texSize.y)
                return;
        }
        uint index = atomicAdd(queueLengths.newPathCounter, 1);
        newPathIndices[index] = globalInvocationID;
        return;
    }

    if (pathStates[globalInvocationID].matID < 100)
    {
        uint index = atomicAdd(queueLengths.basicMaterialCounter, 1);
        basicMaterialPaths[index] = globalInvocationID;
    }
}


