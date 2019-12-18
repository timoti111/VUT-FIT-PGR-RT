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
    uint seed = pathStates[pathIndex].seed;

    RayHit hit = ReadHit(pathIndex);
    Material mat = materials[hit.matID];
    hit.normal = getNormalFromMap(hit, mat.map_N);

    // Generate incoming direction and probability of acquiring light sample
    vec4 dirOut = pathStates[pathIndex].dir;
    float pdfIndirect;
    vec4 dirIn;
    vec4 bsdfIndirect = bxdfSample(hit, mat, dirOut, dirIn, pdfIndirect, seed);
    float cosThIndirect = abs(dot(hit.normal, dirIn));
    pathStates[pathIndex].lastPdfIndirect = pdfIndirect;

    // Update throughput
	vec4 oldT = pathStates[pathIndex].T;
    vec4 newT;
    if (pdfIndirect == 0.0f || bsdfIndirect == vec4(0.0f))
		newT = vec4(0.0f);
    else
        newT = oldT * bsdfIndirect * cosThIndirect / pdfIndirect;
        
    // Avoid self-shadowing
    vec4 orig = hit.position + 1e-4f * dirIn;

    float pdfDirect;
    vec4 bsdfDirect;
    if (renderParameters.sampleDirect)
    {
        // Evaluate direct light sample and probability of acquiring light sample
        vec4 lightIn = pathStates[pathIndex].shadowDir;
        pdfDirect;
        bsdfDirect = bxdfEval(hit, mat, dirOut, lightIn, pdfDirect);
        pathStates[pathIndex].lastPdfDirect *= pdfDirect;
        pathStates[pathIndex].lastBsdfDirect = bsdfDirect;
        pathStates[pathIndex].lastCosThDirect = abs(dot(hit.normal, lightIn));
    }

	// Update path state
    bool isEmissive = mat.type == BXDF_EMISSIVE;
    bool isSpecular = BXDF_IS_SPECULAR(mat.type);
	pathStates[pathIndex].lastT = oldT;
    pathStates[pathIndex].T = newT;
	pathStates[pathIndex].orig = orig;
	pathStates[pathIndex].dir = dirIn;
	pathStates[pathIndex].seed = seed;
	pathStates[pathIndex].lastSpecular = isSpecular;
	pathStates[pathIndex].lightHit = isEmissive;

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
