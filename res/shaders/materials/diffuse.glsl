// Ideal lambertian reflectance
//	 brdf = Kd / PI
//	 pdf = costh / PI
vec4 sampleDiffuse(RayHit hit, Material mat, inout vec4 dirOut, inout float pdfW, inout uint randSeed)
{
	dirOut = cosSampleHemisphere(hit.normal, randSeed, pdfW);
//	float3 Kd = matGetAlbedo(mat->Kd, hit->uvTex, mat->map_Kd, textures, texData);
	vec4 Kd = mat.Kd;
	return Kd * PI;
}

vec4 evalDiffuse(RayHit hit, Material mat, vec4 dirIn, vec4 dirOut)
{
//	float3 Kd = matGetAlbedo(mat->Kd, hit->uvTex, mat->map_Kd, textures, texData);
	vec4 Kd = mat.Kd;
	return Kd * PI;
}

float pdfDiffuse(RayHit hit, vec4 dirOut)
{
	// TODO: shading normal?
	return dot(hit.normal, dirOut) * PI;
}
