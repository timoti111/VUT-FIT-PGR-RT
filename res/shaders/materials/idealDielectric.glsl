// Ideal dielectric
// Check PBRT 8.2 (p.516)

// Fresnel for dielectrics, unpolarized light (PBRT p.519)
float fresnelDielectric(float cosThI, float etaI, float etaT)
{
	float sinThetaI = sqrt(max(0.0f, 1.0f - cosThI * cosThI));
	float sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1.0f)
		return 1.0f;
	float cosThetaT = sqrt(max(0.0f, 1.0f - sinThetaT * sinThetaT));


	float parl = ((etaT * cosThI) - (etaI * cosThetaT)) /
				 ((etaT * cosThI) + (etaI * cosThetaT));
	float perp = ((etaI * cosThI) - (etaT * cosThetaT)) /
				 ((etaI * cosThI) + (etaT * cosThetaT));
	
	return 0.5f * (parl * parl + perp * perp);
}

vec4 sampleIdealDielectric(RayHit hit, Material material, vec4 dirIn, inout vec4 dirOut, inout float pdfW, inout uint randSeed)
{
    vec4 bsdf = vec4(1.0f);

    float cosI = dot(-dirIn, hit.normal);
    bool entering = cosI > 0.0f;
    float n1 = 1.0f, n2 = material.Ni;
    if (!entering)
    {
        swap(n1, n2, float); // inside of material
        cosI = abs(cosI);
        hit.normal *= -1.0f;
    }
    float eta = n1 / n2;

    float fr = fresnelDielectric(cosI, n1, n2);
    if (rand(randSeed) < fr)
    {
//         Reflection
        dirOut = reflect(dirIn, hit.normal);
    }
    else
    {
        // Refraction
        dirOut = refract(dirIn, hit.normal, eta);
        bsdf *= eta * eta; // eta^2 applied in case of radiance transport (16.1.3)
		
        // Simulate absorption
//        vec4 Ks = matGetFloat3(material->Ks, hit->uvTex, material->map_Ks, textures, texData);
        vec4 Ks = material.Ks;
        bsdf *= Ks;
    }

    // (1-fr) or (fr) in pdf and BSDF cancel out
    pdfW = 1.0f;

    dirOut = normalize(dirOut);

    // PBRT eq. 8.8
    // cosTh of geometry term needs to be cancelled out
    float cosO = dot(dirOut, hit.normal);
    return bsdf / abs(cosO);
}

// BSDF (dirac delta) is non-zero with zero probability for two given directions
vec4 evalIdealDielectric()
{
    return vec4(0.0f);
}

// Probability of supplying a correct refl/refr direction pair is zero
float pdfIdealDielectric()
{
    return 0.0f;
}
