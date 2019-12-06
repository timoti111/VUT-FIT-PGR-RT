#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 256) in;

vec4 sampleEnviroment(vec4 direction, float lod)
{
    float theta = acos(direction.y) * INVPI;
    float phi = atan(direction.x, -direction.z) * -INVPI * 0.5f;
    return textureLod(hdriTexture, vec2(phi, theta), lod);
}

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= atomicCounter(basicMaterialCounter))
        return;

    uint pathIndex = basicMaterialPaths[globalInvocationID];
    vec4 color;

    switch(pathStates[pathIndex].matID)
    {
        case -1:
            color = sampleEnviroment(pathStates[pathIndex].primDir, 0.0f);
            break;
        case 0:
            vec4 lightDirection = pick_random_point_in_semisphere(pathStates[pathIndex].hitNorm.xyz);
            vec4 lightColor = sampleEnviroment(lightDirection, 2.0);
            color = clamp(dot(pathStates[pathIndex].hitNorm.xyz, lightDirection.xyz), 0.0, 1.0) * lightColor * vec4(1.0f);
            break;
    }
    pathStates[pathIndex].color = correctGamma(color, GAMMA);
    pathStates[pathIndex].state = TERMINATED;
}
