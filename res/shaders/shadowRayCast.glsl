#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include bvhTraversal.glsl

layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueLengths.shadowRayCounter)
        return;

    uint pathIndex = shadowRayCastPaths[globalInvocationID];
    Ray ray = CreateRay(pathStates[pathIndex].primOrig, pathStates[pathIndex].primDir);
    RayHit rayHit = CreateRayHit();
    rayHit.t = pathStates[pathIndex].t;
    pathStates[pathIndex].shadowRayBlocked = IntersectScene(ray, true, rayHit);
}
