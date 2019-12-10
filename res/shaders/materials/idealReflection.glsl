// Ideal conductive specular reflection (mirror)
// Check PBRT 8.2 (p.516)

vec4 sampleIdealReflection(RayHit hit, Material material, bool backface, vec4 dirIn, inout vec4 dirOut, inout float pdfW, inout uint randSeed)
{
	float len = length(dirIn);
	dirOut = len * reflect(normalize(dirIn), hit.normal);
	pdfW = 1.0f;

	// Fresnel ignored in ideal spacular case

	// PBRT eq. 8.8
	// cosTh of geometry term needs to be cancelled out
//	float3 ks = matGetFloat3(material->Ks, hit->uvTex, material->map_Ks, textures, texData);
	vec4 Ks = material.Ks;
	float cosO = dot(normalize(dirOut), hit.normal);
	return (cosO != 0.0f) ? Ks / cosO : vec4(0.0f);
}

// BSDF (dirac delta) is non-zero with zero probability for two given directions
vec4 evalIdealReflection()
{
	return vec4(0.0f);
}

// Probability of supplying a correct refl/refr direction pair is zero
float pdfIdealReflection()
{
	return 0.0f;
}
