#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl

layout(local_size_x = 32) in;

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
    bool blocked = pathStates[globalInvocationID].shadowRayBlocked;
    if (renderParameters.sampleDirect)
    {
        if (!blocked)
        {
            vec4 emission = pathStates[globalInvocationID].lastEmission;
            vec4 bsdfDirect = pathStates[globalInvocationID].lastBsdfDirect;
            float pdfDirect = pathStates[globalInvocationID].lastPdfDirect;
            float pdfIndirect = pathStates[globalInvocationID].lastPdfIndirect;
            float cosThDirect = pathStates[globalInvocationID].lastCosThDirect;
            float lightPickProb = pathStates[globalInvocationID].lastLightPickProb;
            float maxDistance = pathStates[globalInvocationID].maxShadowRayLen;
            vec4 lastT = pathStates[globalInvocationID].lastT;
            vec4 contribution = bsdfDirect * emission * cosThDirect / (lightPickProb * pdfDirect + pdfIndirect);
            pathStates[globalInvocationID].Ei += lastT * contribution;
            pathStates[globalInvocationID].shadowRayBlocked = true;
            newEiWritten = true;
        }
    }

    if (hit.matID < 0)
    {
        bool lastSpecular = pathStates[globalInvocationID].lastSpecular;
        vec4 bg = vec4(0.0f);

        if (renderParameters.useEnvironmentMap)
        {
            if (lastSpecular || pathLen == 1)
                bg = sampleEnviroment(renderParameters.environmentMapTextureID, ray.direction, 0.0f);
            else
                bg = sampleEnviroment(renderParameters.environmentMapTextureID, ray.direction, 7.0f);
        }
        else
            bg = renderParameters.backgroundColor;

        pathStates[globalInvocationID].Ei += bg * T * renderParameters.backgroundIntensity;
        newEiWritten = true;
        terminated = true;
    }
    else if (pathStates[globalInvocationID].lightHit)
    {
        float lightPickProb = pathStates[globalInvocationID].lastLightPickProb;
        float pdfDirect = pathStates[globalInvocationID].lastPdfDirect;
        float pdfIndirect = pathStates[globalInvocationID].lastPdfIndirect;
        float weight = 1.0f;
        Material mat = materials[hit.matID];
        vec4 emission = mat.Ke * mat.Ns * INVPI;
        float maxDistance = pathStates[globalInvocationID].maxShadowRayLen;
        if (!blocked)
            weight = pdfIndirect / (pdfDirect * lightPickProb + pdfIndirect);
        pathStates[globalInvocationID].Ei += T * weight * emission;
        newEiWritten = true;
        terminated = true;
    }
    else
    {
        terminated = terminated || pathLen - 1 >= renderParameters.maxBounces;
        // Russian roulette
        if (terminated && renderParameters.useRussianRoulette)
        {
            float contProb = max3(T.xyz);
            terminated = (rand(pathStates[globalInvocationID].seed) > contProb);
            T /= contProb;
            pathStates[globalInvocationID].T = T;
        }

        terminated = terminated || T == vec4(0.0f);
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
        newPathQueue[index] = globalInvocationID;
        return;
    }

    if (renderParameters.sampleDirect)
    {
        uint numOfLights = renderParameters.numberOfLights;
        if (numOfLights != 0)
        {
            float randomPick = rand(pathStates[globalInvocationID].seed);
            float pickProbability = 1.0f / numOfLights;
            int lightIndex = int(floor(randomPick / pickProbability));
            Light light = lights[lightIndex];
            Material mat = materials[light.materialID];
            float pdf;
            vec4 lightPos = smapleLightSurface(light, hit.position, pdf, pathStates[globalInvocationID].seed);
            vec4 fromHitToLight = lightPos - hit.position;
            float maxShadowLen = length(fromHitToLight) * 0.999f;
            pathStates[globalInvocationID].maxShadowRayLen = maxShadowLen;
            fromHitToLight = normalize(fromHitToLight);
            pathStates[globalInvocationID].shadowDir = fromHitToLight;
            pathStates[globalInvocationID].shadowOrig = hit.position + 1e-4f * fromHitToLight;
//            pathStates[globalInvocationID].lastPdfDirect = 0.2387324146378 * pow(maxShadowLen, 2) / (abs(dot(fromHitToLight, normal)) * 0.5f * INVPI);//pow(maxShadowLen, 2) / (pdf * 4.0f * PI * pow(light.sphere.w, 2));
            pathStates[globalInvocationID].lastPdfDirect = pow(maxShadowLen, 2) / (pdf * 4.0f * PI * pow(light.sphere.w, 2));
            pathStates[globalInvocationID].lastEmission = mat.Ke * mat.Ns * INVPI;
            pathStates[globalInvocationID].lastLightPickProb = pickProbability;
        }
    }
    
    uint index = atomicWarpAdd(MATERIAL_QUEUE, 1);
    materialQueue[index] = globalInvocationID;
}
