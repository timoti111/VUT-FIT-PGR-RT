#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;
uniform bool firstIteration;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueLengths.basicMaterialCounter)
        return;

    uint pathIndex = basicMaterialPaths[globalInvocationID];

    switch(pathStates[pathIndex].matID)
    {
        case 0:
            vec4 lightDirection = pick_random_point_in_semisphere(pathStates[pathIndex].hitNorm.xyz);
            vec4 lightColor = sampleEnviroment(lightDirection, 2.0);
            pathStates[pathIndex].color = clamp(dot(pathStates[pathIndex].hitNorm.xyz, lightDirection.xyz), 0.0, 1.0) * lightColor * vec4(1.0f);
            pathStates[pathIndex].throughput = vec4(0.0f);
            break;
    }

    if (firstIteration)
    {
        if (pathStates[pathIndex].pathLen == renderParameters.maxBounces)
        {
            pathStates[pathIndex].pathLen++;
        }
    }
    else
    {
        uint extensionRayIndex = atomicAdd(queueLengths.extensionRayCounter, 1);
        extRayCastPaths[extensionRayIndex] = pathIndex;
    }
}
