#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl
#include bvhTraversal.glsl


layout(local_size_x = 128) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueCounters[EXTENSION_RAY_QUEUE])
        return;

    uint pathIndex = extensionRayQueue[globalInvocationID];
    Ray ray = CreateRay(pathStates[pathIndex].orig, pathStates[pathIndex].dir);
    RayHit rayHit = EmptyHit(FLT_MAX);
    IntersectScene4(ray, false, rayHit);
    if (IntersectSphere(ray, vec4(0.0f, 3.0f, 0.0f, 0.5f), rayHit))
        rayHit.matID = 0;
    rayHit.position = ray.origin + rayHit.t * ray.direction;
    rayHit.normal = normalize(rayHit.normal);
    WriteHit(rayHit, pathIndex);
    pathStates[pathIndex].pathLen++;
}
