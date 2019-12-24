#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl
#include materials/bxdf.glsl

layout(local_size_x = 32) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueCounters[MATERIAL_QUEUE])
        return;
    uint pathIndex = materialQueue[globalInvocationID];
    uint localSeed = GetPathInfo(pathIndex, seed);

    RayHit hit = ReadHit(pathIndex);
    Material mat = materials[hit.matID];
    hit.normal = getNormalFromMap(hit, mat.map_N);

    // Generate incoming direction and probability of acquiring light sample
    vec4 dirOut = GetPathInfo(pathIndex, dir);
    float pdfIndirect;
    vec4 dirIn;
    vec4 bsdfIndirect = bxdfSample(hit, mat, dirOut, dirIn, pdfIndirect, localSeed);
    float cosThIndirect = abs(dot(hit.normal, dirIn));
    SetPathInfo(pathIndex, lastPdfIndirect, pdfIndirect);

    // Update throughput
	vec4 oldT = GetPathInfo(pathIndex, T);
    vec4 newT;
    if (pdfIndirect == 0.0f || bsdfIndirect == vec4(0.0f))
		newT = vec4(0.0f);
    else
        newT = oldT * bsdfIndirect * cosThIndirect / pdfIndirect;
        
    // Avoid self-shadowing
    vec4 origin = hit.position + 1e-4f * dirIn;

    float pdfDirect;
    vec4 bsdfDirect;
    if (renderParameters.sampleDirect)
    {
        // Evaluate direct light sample and probability of acquiring light sample
        vec4 lightIn = GetPathInfo(pathIndex, shadowDir);
        pdfDirect;
        bsdfDirect = bxdfEval(hit, mat, dirOut, lightIn, pdfDirect);
        MulByPathInfo(pathIndex, lastPdfDirect, pdfDirect);
        SetPathInfo(pathIndex, lastBsdfDirect, bsdfDirect);
        SetPathInfo(pathIndex, lastCosThDirect, abs(dot(hit.normal, lightIn)));
    }

    // Update path state
    bool isEmissive = mat.type == BXDF_EMISSIVE;
    bool isSpecular = BXDF_IS_SPECULAR(mat.type);
    SetPathInfo(pathIndex, lastT, oldT);
    SetPathInfo(pathIndex, T, newT);
    SetPathInfo(pathIndex, orig, origin);
    SetPathInfo(pathIndex, dir, dirIn);
    SetPathInfo(pathIndex, seed, localSeed);
    SetPathInfo(pathIndex, lastSpecular, isSpecular);
    SetPathInfo(pathIndex, lightHit, isEmissive);

    if (!isEmissive)
    {
        uint extensionRayIndex = atomicWarpAdd(EXTENSION_RAY_QUEUE, 1);
        extensionRayQueue[extensionRayIndex] = pathIndex;
        if (!isSpecular && pdfDirect != 0.0f && bsdfDirect != vec4(0.0f) && renderParameters.sampleDirect && renderParameters.numberOfLights > 0)
        {
            uint shadowRayIndex = atomicWarpAdd(SHADOW_RAY_QUEUE, 1);
            shadowRayQueue[shadowRayIndex] = pathIndex;
        }
    }
}
