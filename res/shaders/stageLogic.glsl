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
    Ray ray = CreateRay(GetPathInfo(globalInvocationID, orig), GetPathInfo(globalInvocationID, dir));
    vec4 localT = GetPathInfo(globalInvocationID, T);
    uint pathLen = GetPathInfo(globalInvocationID, pathLen);
    bool terminated = false;
    bool blocked = GetPathInfo(globalInvocationID, shadowRayBlocked);
    if (renderParameters.sampleDirect)
    {
        if (!blocked)
        {
            vec4 emission = GetPathInfo(globalInvocationID, lastEmission);
            vec4 bsdfDirect = GetPathInfo(globalInvocationID, lastBsdfDirect);
            float pdfDirect = GetPathInfo(globalInvocationID, lastPdfDirect);
            float pdfIndirect = GetPathInfo(globalInvocationID, lastPdfIndirect);
            float cosThDirect = GetPathInfo(globalInvocationID, lastCosThDirect);
            float lightPickProb = GetPathInfo(globalInvocationID, lastLightPickProb);
            vec4 lastT = GetPathInfo(globalInvocationID, lastT);
            vec4 contribution = bsdfDirect * emission * cosThDirect / (lightPickProb * pdfDirect + pdfIndirect);
            IncByPathInfo(globalInvocationID, Ei, lastT * contribution);
            SetPathInfo(globalInvocationID, shadowRayBlocked, true);
        }
    }

    if (hit.matID < 0)
    {
        bool lastSpecular = GetPathInfo(globalInvocationID, lastSpecular);
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

        IncByPathInfo(globalInvocationID, Ei, bg * localT * renderParameters.backgroundIntensity);
        terminated = true;
    }
    else if (GetPathInfo(globalInvocationID, lightHit))
    {
        float lightPickProb = GetPathInfo(globalInvocationID, lastLightPickProb);
        float pdfDirect = GetPathInfo(globalInvocationID, lastPdfDirect);
        float pdfIndirect = GetPathInfo(globalInvocationID, lastPdfIndirect);
        float weight = 1.0f;
        Material mat = materials[hit.matID];
        vec4 emission = mat.Ke * mat.Ns * INVPI;
        if (!blocked)
            weight = pdfIndirect / (pdfDirect * lightPickProb + pdfIndirect);
        IncByPathInfo(globalInvocationID, Ei, localT * weight * emission);
        terminated = true;
    }
    else
    {
        terminated = terminated || pathLen - 1 >= renderParameters.maxBounces;
        // Russian roulette
        if (terminated && renderParameters.useRussianRoulette)
        {
            float contProb = max3(localT.xyz);
            terminated = (rand(GetPathInfo(globalInvocationID, seed)) > contProb);
            localT /= contProb;
            SetPathInfo(globalInvocationID, T, localT);
        }

        terminated = terminated || localT == vec4(0.0f);
    }

    if(terminated)
    {
        ivec2 pixelIndex = GetPathInfo(globalInvocationID, pixelIndex);
        vec4 result = imageLoad(destTex, pixelIndex);
        result += vec4(GetPathInfo(globalInvocationID, Ei).xyz, 1.0f);
        imageStore(destTex, pixelIndex, result);
        uint index = atomicWarpAdd(NEW_PATH_QUEUE, 1);
        newPathQueue[index] = globalInvocationID;
        return;
    }

    if (renderParameters.sampleDirect)
    {
        uint numOfLights = renderParameters.numberOfLights;
        if (numOfLights != 0)
        {
            float randomPick = rand(GetPathInfo(globalInvocationID, seed));
            float pickProbability = 1.0f / numOfLights;
            int lightIndex = int(floor(randomPick / pickProbability));
            Light light = lights[lightIndex];
            Material mat = materials[light.materialID];
            float pdf;
            vec4 lightPos = smapleLightSurface(light, hit.position, pdf, GetPathInfo(globalInvocationID, seed));
            vec4 fromHitToLight = lightPos - hit.position;
            float maxShadowLen = length(fromHitToLight) * 0.999f;
            SetPathInfo(globalInvocationID, maxShadowRayLen, maxShadowLen);
            fromHitToLight = normalize(fromHitToLight);
            SetPathInfo(globalInvocationID, shadowDir, fromHitToLight);
            SetPathInfo(globalInvocationID, shadowOrig, hit.position + 1e-4f * fromHitToLight);
            SetPathInfo(globalInvocationID, lastPdfDirect, pow(maxShadowLen, 2) / (pdf * 4.0f * PI * pow(light.sphere.w, 2)));
            SetPathInfo(globalInvocationID, lastEmission, mat.Ke * mat.Ns * INVPI);
            SetPathInfo(globalInvocationID, lastLightPickProb, pickProbability);
        }
    }
    
    uint index = atomicWarpAdd(MATERIAL_QUEUE, 1);
    materialQueue[index] = globalInvocationID;
}
