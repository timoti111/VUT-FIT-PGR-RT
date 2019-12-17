vec4 sampleEmissive(RayHit hit, Material mat, inout vec4 dirOut, inout float pdfW)
{
    dirOut = hit.normal;
    pdfW = SPHERE_PDF;
    return mat.Ns * mat.Ke * INVPI;
}

// BSDF (dirac delta) is non-zero with zero probability for two given directions
vec4 evalEmissive(inout float pdf)
{
    pdf = 0.0f;
    return vec4(0.0f);
}
