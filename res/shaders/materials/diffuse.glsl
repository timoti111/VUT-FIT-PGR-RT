// Ideal lambertian reflectance
//	 brdf = Kd / PI
//	 pdf = costh / PI
vec4 sampleDiffuse(RayHit hit, Material mat, inout vec4 dirOut, inout float pdfW, inout uint randSeed)
{
	dirOut = cosSampleHemisphere(hit.normal, randSeed);
    pdfW = HEMISPHERE_PDF;
	vec4 Kd = readMaterial(mat.Kd, hit.uv, mat.map_Kd);
	return Kd * INVPI;
}

vec4 evalDiffuse(RayHit hit, Material mat, inout vec4 dirOut, inout float pdf)
{
    pdf = dot(hit.normal, dirOut) >= 1e-4f ? HEMISPHERE_PDF : 0.0f;
    vec4 Kd = readMaterial(mat.Kd, hit.uv, mat.map_Kd);
    return Kd * INVPI;
}
