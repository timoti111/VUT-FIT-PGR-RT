#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include bvhTraversal.glsl
#include utils.glsl


layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueLengths.extensionRayCounter)
        return;

    uint pathIndex = extRayCastPaths[globalInvocationID];
    Ray ray = CreateRay(pathStates[pathIndex].orig, pathStates[pathIndex].dir);
    RayHit rayHit = EmptyHit(pathStates[pathIndex].t);
    IntersectScene(ray, false, rayHit);
    WriteHit(rayHit, pathIndex);
    pathStates[pathIndex].pathLen++;
}
