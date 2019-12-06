#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include bvhTraversal.glsl

layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= atomicCounter(extensionRayCounter))
        return;

    uint pathIndex = extRayCastPaths[globalInvocationID];
    Ray ray = CreateRay(pathStates[pathIndex].primOrig, pathStates[pathIndex].primDir);
    RayHit rayHit = CreateRayHit();
    rayHit.t = pathStates[pathIndex].t;
    IntersectScene(ray, false, rayHit);
//    if (IntersectScene(ray, false, rayHit))
//    {
        pathStates[pathIndex].hitPos = rayHit.position;
        pathStates[pathIndex].hitNorm = rayHit.normal;
        pathStates[pathIndex].hitUV = rayHit.uv;
        pathStates[pathIndex].t = rayHit.t;
        pathStates[pathIndex].matID = rayHit.matID;
//    }
}
