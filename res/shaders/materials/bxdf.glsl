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
	vec4 dirOut)
{
	switch(material.type)
	{
		case BXDF_DIFFUSE:
			return evalDiffuse(hit, material, dirIn, dirOut);
		case BXDF_IDEAL_REFLECTION:
			return evalIdealReflection();
		case BXDF_IDEAL_DIELECTRIC:
			return evalIdealDielectric();
		case BXDF_EMISSIVE:
			return vec4(1.0f, 1.0f, 1.0f, 0.0f);
	}
	
	return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

// Get pdf given incoming, outgoing directions (mainly for MIS)
float bxdfPdf(
	RayHit hit,
	Material material,
	vec4 dirIn,
	vec4 dirOut)
{
	switch(material.type)
	{
		case BXDF_DIFFUSE:
			return pdfDiffuse(hit, dirOut);
		case BXDF_IDEAL_REFLECTION:
			return pdfIdealReflection();
		case BXDF_IDEAL_DIELECTRIC:
			return pdfIdealDielectric();
		case BXDF_EMISSIVE:
			return 0.0f;
	}

	return 0.0f;
}
