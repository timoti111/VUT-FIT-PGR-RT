#version 460
#include defines.glsl
#include structuresWavefront.glsl
#include buffersWavefront.glsl
#include utils.glsl
#include materials/bxdf.glsl

layout(local_size_x = 256) in;

void main()
{
    uint globalInvocationID = uint(gl_GlobalInvocationID.x);
    if (globalInvocationID >= queueLengths.basicMaterialCounter)
        return;
    uint pathIndex = basicMaterialPaths[globalInvocationID];
    uint seed = pathStates[pathIndex].seed;

    RayHit hit = ReadHit(pathIndex);
    Material mat = materials[hit.matID];
    bool backface = pathStates[pathIndex].backfaceHit;

    vec4 dirIn = pathStates[pathIndex].dir; // points toward surface!
    vec4 L = pathStates[pathIndex].shadowDir;

    vec4 bsdfNEE = bxdfEval(hit, mat, backface, dirIn, L);
    float bsdfPdfW = max(0.0f, bxdfPdf(hit, mat, backface, dirIn, L));
    pathStates[pathIndex].lastBsdf = bsdfNEE;
    pathStates[pathIndex].lastPdfImplicit =  bsdfPdfW;
    
    // Generate continuation ray by sampling BSDF
    float pdfW;
    vec4 newDir;
    vec4 bsdf = bxdfSample(hit, mat, backface, dirIn, newDir, pdfW, seed);
    float costh = dot(hit.normal, normalize(newDir));
	
    // Update throughput * pdf
	vec4 oldT = pathStates[pathIndex].T;
    vec4 newT;
    if (pdfW == 0.0f || bsdf == vec4(0.0f))
		newT = vec4(0.0f);
    else
        newT = oldT * bsdf * costh / pdfW;
        
    // Avoid self-shadowing
    vec4 orig = hit.position + 1e-4f * newDir;

	// Update path state
    bool isEmissive = mat.type == BXDF_EMISSIVE;
	pathStates[pathIndex].lastT = oldT;
    pathStates[pathIndex].T = newT;
	pathStates[pathIndex].orig = orig;
	pathStates[pathIndex].dir = newDir;
	pathStates[pathIndex].lastPdfW = pdfW;
	pathStates[pathIndex].seed = seed;
	pathStates[pathIndex].lastSpecular = BXDF_IS_SINGULAR(mat.type);
	pathStates[pathIndex].lightHit = isEmissive;

    if (!isEmissive)
    {
        uint extensionRayIndex = atomicAdd(queueLengths.extensionRayCounter, 1);
        extRayCastPaths[extensionRayIndex] = pathIndex;
    }
}
