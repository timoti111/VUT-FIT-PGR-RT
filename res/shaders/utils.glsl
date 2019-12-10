uint hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

float rand(inout uint seed)
{
    seed = hash(seed);
    return float(seed) * UINTMAXINV; // 1.0f / 2^32
}

vec4 pick_random_point_in_sphere(inout uint seed)
{
    float x0, x1, x2, x3, d2;
    do
    {
        x0 = rand(seed);
        x1 = rand(seed);
        x2 = rand(seed);
        x3 = rand(seed);
        d2 = x0 * x0 + x1 * x1 + x2 * x2 + x3 * x3;
    } while(d2 > 1.0f);
    float scale = 1.0f / d2;
    return vec4(
        2 * (x1 * x3 + x0 * x2) * scale,
        2 * (x2 * x3 + x0 * x1) * scale,
        (x0 * x0 + x3 * x3 - x1 * x1 - x2 * x2) * scale,
        1.0f
    );
}

vec4 pick_random_point_in_semisphere(vec3 v, inout uint seed)
{
    vec4 result = pick_random_point_in_sphere(seed);
    if (dot(v, result.xyz) < 0)
    {
        result.x = -result.x;
        result.y = -result.y;
        result.z = -result.z;
    }
    return normalize(result);
}

vec2 getEnvMapUV(vec4 direction)
{
    float theta = acos(direction.y) * INVPI;
    float phi = atan(direction.x, -direction.z) * -INVPI * 0.5f;
    return vec2(phi, theta);
}

// sRGB luminance
float luminance(vec3 v)
{
	return 0.212671f * v.x + 0.715160f * v.y + 0.072169f * v.z;
}

float envPdf(vec4 direction)
{
    vec2 uv = getEnvMapUV(direction);
    float sinTh = sin(uv.y * PI);
    if (sinTh == 0.0f)
        return 0.0f;
    return luminance(textureLod(hdriTexture, getEnvMapUV(direction), 0.0f).xyz) / (2 * PI * PI * sinTh);
}

vec4 sampleEnviroment(vec4 direction, float lod)
{
    return textureLod(hdriTexture, getEnvMapUV(direction), lod);
}

RayHit EmptyHit(float t)
{
    RayHit hit;
    hit.position = vec4(0.0f);
    hit.normal = vec4(0.0f);
    hit.uv = vec2(0.0f);
    hit.t = t;
    hit.matID = -1;
    hit.triIndex = -1;
    return hit;
};

RayHit ReadHit(uint pathIndex)
{
    RayHit hit;
    hit.position = pathStates[pathIndex].hitP;
    hit.normal = pathStates[pathIndex].hitN;
    hit.uv = pathStates[pathIndex].hitUV;
    hit.t = pathStates[pathIndex].t;
    hit.matID = pathStates[pathIndex].matID;
    hit.triIndex = pathStates[pathIndex].triIndex;
    return hit;
};

void WriteHit(RayHit hit, uint pathIndex)
{
    pathStates[pathIndex].hitP = hit.position;
    pathStates[pathIndex].hitN = hit.normal;
    pathStates[pathIndex].hitUV = hit.uv;
    pathStates[pathIndex].t = hit.t;
    pathStates[pathIndex].matID = hit.matID;
    pathStates[pathIndex].triIndex = hit.triIndex;
};

vec4 cosSampleHemisphere(vec4 n, inout uint seed, inout float p)
{
    float r1 = 2.0f * PI * rand(seed);
    float r2 = rand(seed);
    float r2s = sqrt(r2);

    vec3 w = n.xyz;

    vec3 u;
    if (abs(w.x) > 0.1f) {
        vec3 a = vec3(0.0f, 1.0f, 0.0f);
        u = cross(a, w);
    }
    else {
        vec3 a = vec3(1.0f, 0.0f, 0.0f);
        u = cross(a, w);
    }
    u = normalize(u);

    vec3 v = cross(w, u);
    
    u *= (cos(r1) * r2s);
    v *= (sin(r1) * r2s);
    w *= (sqrt(1 - r2));

    vec3 dir = u + v + w;
    float costh = dot(n.xyz, dir);
    p = 0.5f * PI; //pdf
	return vec4(dir, 0.0f);
}
