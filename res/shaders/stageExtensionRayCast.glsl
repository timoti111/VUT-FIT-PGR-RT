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
    if (globalInvocationID >= queueCounters[EXTENSION_RAY_QUEUE])
        return;

    uint pathIndex = extensionRayQueue[globalInvocationID];
    Ray ray = CreateRay(GetPathInfo(pathIndex, orig), GetPathInfo(pathIndex, dir));
    RayHit rayHit = EmptyHit(FLT_MAX);
    IntersectScene4(ray, false, rayHit);
    SetPathInfo(pathIndex, lightHit, IntersectLights(ray, false, rayHit));
    rayHit.position = ray.origin + rayHit.t * ray.direction;
    rayHit.normal = normalize(rayHit.normal);
    WriteHit(rayHit, pathIndex);
    IncPathInfo(pathIndex, pathLen);
}
