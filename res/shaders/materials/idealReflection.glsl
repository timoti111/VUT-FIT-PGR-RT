// Ideal conductive specular reflection (mirror)
// Check PBRT 8.2 (p.516)

vec4 sampleIdealReflection(RayHit hit, Material material, vec4 dirIn, inout vec4 dirOut, inout float pdf, inout uint randSeed)
{
    dirOut = normalize(reflect(dirIn, hit.normal));
    pdf = 1.0f;

    // Fresnel ignored in ideal spacular case

    // PBRT eq. 8.8
    // cosTh of geometry term needs to be cancelled out
    vec4 Ks = readMaterial(material.Ks, hit.uv, material.map_Ks);
    float cosO = dot(dirOut, hit.normal);
    return Ks / abs(cosO);
}

// BSDF (dirac delta) is non-zero with zero probability for two given directions
vec4 evalIdealReflection(inout float pdf)
{
    pdf = 0.0f;
    return vec4(0.0f);
}
