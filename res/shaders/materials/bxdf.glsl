#include diffuse.glsl
#include idealReflection.glsl
#include idealDielectric.glsl
#include emissive.glsl


// Note: In these formulas, dirIn points towards the surface, not away from it!

// Generate outgoing direction and pdf given invoming direction
vec4 bxdfSample(
    RayHit hit,
    Material material,
    vec4 dirIn,
    inout vec4 dirOut,
    inout float pdfW,
    inout uint randSeed)
{
    switch(material.type)
    {
        case BXDF_DIFFUSE:
            return sampleDiffuse(hit, material, dirOut, pdfW, randSeed);
        case BXDF_IDEAL_REFLECTION:
            return sampleIdealReflection(hit, material, dirIn, dirOut, pdfW, randSeed);
        case BXDF_IDEAL_DIELECTRIC:
            return sampleIdealDielectric(hit, material, dirIn, dirOut, pdfW, randSeed);
        case BXDF_EMISSIVE:
            return sampleEmissive(hit, material, dirOut, pdfW);
    }

    return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

// Evaluate bxdf value given invoming and outgoing directions.
vec4 bxdfEval(
	RayHit hit,
	Material material,
	vec4 dirIn,
	vec4 dirOut,
    inout float pdfW)
{
	switch(material.type)
	{
		case BXDF_DIFFUSE:
			return evalDiffuse(hit, material, dirOut, pdfW);
		case BXDF_IDEAL_REFLECTION:
			return evalIdealReflection(pdfW);
		case BXDF_IDEAL_DIELECTRIC:
			return evalIdealDielectric(pdfW);
		case BXDF_EMISSIVE:
			return evalEmissive(pdfW);
	}
	
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}
