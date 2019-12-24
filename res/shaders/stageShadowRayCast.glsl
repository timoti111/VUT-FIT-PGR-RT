#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl
#include bvhTraversal.glsl

layout(local_size_x = 32) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueCounters[SHADOW_RAY_QUEUE])
        return;

    uint pathIndex = shadowRayQueue[globalInvocationID];
    Ray ray = CreateRay(GetPathInfo(pathIndex, shadowOrig), GetPathInfo(pathIndex, shadowDir));
    RayHit rayHit = EmptyHit(GetPathInfo(pathIndex, maxShadowRayLen));
    SetPathInfo(pathIndex, shadowRayBlocked, IntersectScene4(ray, true, rayHit) || IntersectLights(ray, true, rayHit));
}
