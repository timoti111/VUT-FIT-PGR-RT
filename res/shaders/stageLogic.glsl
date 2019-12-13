#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 32) in;
uniform bool firstIteration;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID == 0)
    {
        currentPixelCount += queueCounters[NEW_PATH_QUEUE];
        queueCounters[NEW_PATH_QUEUE] = 0;
        queueCounters[MATERIAL_QUEUE] = 0;
        queueCounters[EXTENSION_RAY_QUEUE] = 0;
        queueCounters[SHADOW_RAY_QUEUE] = 0;
    }
    ivec2 texSize = imageSize(destTex);

    if (globalInvocationID >= NUM_PATHS)
        return;

    RayHit hit = ReadHit(globalInvocationID);
    Ray ray = CreateRay(pathStates[globalInvocationID].orig, pathStates[globalInvocationID].dir);
    vec4 T = pathStates[globalInvocationID].T;
    

    uint pathLen = pathStates[globalInvocationID].pathLen;
    bool terminated = false;
    bool newEiWritten = false;

    if (hit.matID < 0)
    {
        float weight = 1.0f;
        bool lastSpecular = pathStates[globalInvocationID].lastSpecular;
        vec4 bg;

        if (renderParameters.useEnvironmentMap)
            bg = clamp(sampleEnviroment(renderParameters.environmentMapTextureID, ray.direction, 2.0f), 0.0f, 10.0f);
        else
            bg = renderParameters.backgroundColor;

        vec4 newEi = pathStates[globalInvocationID].Ei + weight * T * bg * renderParameters.backgroundIntensity;
        pathStates[globalInvocationID].Ei = newEi;
        newEiWritten = true;
        terminated = true;
    }
    else if (pathStates[globalInvocationID].lightHit)
    {
        vec4 newEi = pathStates[globalInvocationID].Ei + T;
        pathStates[globalInvocationID].Ei = newEi;
        newEiWritten = true;
        terminated = true;
    }
    else
    {
        terminated = terminated || pathLen - 1 >= renderParameters.maxBounces;
        // Russian roulette
        if (terminated && renderParameters.useRussianRoulette) // OPRAVIT RULETU
        {
//            float contProb = clamp(max3(T.xyz), 0.01f, 0.5f);
            float contProb = max3(T.xyz);
            terminated = (rand(pathStates[globalInvocationID].seed) > contProb);
            T /= contProb;
            pathStates[globalInvocationID].T = T;
        }

        terminated = terminated || T == vec4(0.0f) || pathStates[globalInvocationID].lastPdfW == 0.0f;
    }

    // Explicit light sample (NEE), if non-occluded
    bool blocked = pathStates[globalInvocationID].shadowRayBlocked;
    if (!blocked)
    {
        vec4 emission = pathStates[globalInvocationID].lastEmission;
        vec4 bsdf = pathStates[globalInvocationID].lastBsdf;
        float cosTh = pathStates[globalInvocationID].lastCosTh; // cos at surface
        float directPdfW = pathStates[globalInvocationID].lastPdfDirect;
        float bsdfPdfW = pathStates[globalInvocationID].lastPdfImplicit;
        float lightPickProb = pathStates[globalInvocationID].lastLightPickProb;

        // Only do MIS weighting if other samplers (bsdf-sampling) could have generated the sample
        float weight = 1.0f;
//        if (params->sampleImpl)
//        {
            weight = (directPdfW * lightPickProb) / (directPdfW * lightPickProb + bsdfPdfW);
//        }

        vec4 T = pathStates[globalInvocationID].lastT;
        vec4 contrib = bsdf * T * emission * weight * cosTh / (lightPickProb * directPdfW);
        vec4 newEi = pathStates[globalInvocationID].Ei + contrib;
        pathStates[globalInvocationID].Ei = newEi;
        newEiWritten = true;
    }

    if(terminated)
    {
        if (newEiWritten)
        {
            ivec2 pixelIndex = pathStates[globalInvocationID].pixelIndex;
            vec4 result = imageLoad(destTex, pixelIndex);
            result += vec4(pathStates[globalInvocationID].Ei.xyz, 1.0f);
            imageStore(destTex, pixelIndex, result);
        }
        uint index = atomicWarpAdd(NEW_PATH_QUEUE, 1);
        if (firstIteration)
        {
            if (currentPixelCount + index >= texSize.x * texSize.y)
            {
                uint index = atomicWarpAdd(NEW_PATH_QUEUE, -1);
                return;
            }
        }
        newPathQueue[index] = globalInvocationID;
        return;
    }
    
    uint index = atomicWarpAdd(MATERIAL_QUEUE, 1);
    materialQueue[index] = globalInvocationID;
}


