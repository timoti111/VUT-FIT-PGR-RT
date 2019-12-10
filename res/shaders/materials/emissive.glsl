vec4 sampleEmissive(RayHit hit, Material mat, inout vec4 dirOut, inout float pdfW)
{
    dirOut = hit.normal;
    pdfW = 0.5f * PI;
    vec4 Ke = mat.Ke;
    return mat.Ns * Ke * PI;
}
