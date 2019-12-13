vec3 minVec3(vec3 a, vec3 b)
{
    return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}

vec3 maxVec3(vec3 a, vec3 b)
{
    return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}

float min3(vec3 v)
{
    return min(min(v.x, v.y), v.z);
}

float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

uint hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

// sRGB luminance
float luminance(vec3 v)
{
    return 0.212671f * v.x + 0.715160f * v.y + 0.072169f * v.z;
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

vec4 sampleEnviroment(uint textureID, vec4 direction, float lod)
{
    float theta = acos(direction.y) * INVPI;
    float phi = atan(direction.x, -direction.z) * -INVPI * 0.5f;
    return textureLod(textures[textureID], vec2(phi, theta), lod);
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
    hit.backfaceHit = false;
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
    hit.backfaceHit = pathStates[pathIndex].backfaceHit;
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
    pathStates[pathIndex].backfaceHit = hit.backfaceHit;
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

    vec3 dir = normalize(u + v + w);
//    p = HEMISPHERE_PDF; //pdf
    p = dot(n.xyz, dir) * INVPI; //pdf
	return vec4(dir, 0.0f);
}

vec4 getNormalFromMap(RayHit hit, int idx)
{
    if (idx == -1)
        return hit.normal;
        
    vec2 uvCor = vec2(hit.uv.x, -hit.uv.y);
    vec3 texNormal = texture(textures[idx], uvCor).xyz;
    texNormal = 2.0f * texNormal - vec3(1.0f);
    
    ivec4 p = triangles[hit.triIndex].vertices;
    ivec4 t = triangles[hit.triIndex].coords;
    vec4 v0 = vertices[p.x];
    vec4 v1 = vertices[p.y];
    vec4 v2 = vertices[p.z];
    vec2 t0 = coords[t.x];
    vec2 t1 = coords[t.y];
    vec2 t2 = coords[t.z];
    vec3 e1d = (v1 - v0).xyz;
    vec3 e2d = (v2 - v0).xyz;
    vec2 t1d = t1 - t0;
    vec2 t2d = t2 - t0;

    // Detect invalid normal map
    float det = (t1d.x * t2d.y - t1d.y * t2d.x);
    if (det == 0.0)
        return hit.normal;

    // Compute T, B using inverse of [t1.x t1.y; t2.x t2.y]
    float invDet = 1.0f / det;
    vec3 T = normalize(invDet * (e1d * t2d.y - e2d * t1d.y));
    vec3 B = normalize(invDet * (e2d * t1d.x - e1d * t2d.x));

    // Expanded matrix multiply M * hit.N
    vec3 N;
    N.x = T.x * texNormal.x + B.x * texNormal.y + hit.normal.x * texNormal.z;
    N.y = T.y * texNormal.x + B.y * texNormal.y + hit.normal.y * texNormal.z;
    N.z = T.z * texNormal.x + B.z * texNormal.y + hit.normal.z * texNormal.z;

    return vec4(normalize(N), 0.0f);
}

vec4 readMaterial(vec4 fallback, vec2 uv, int idx)
{
    vec2 uvCor = vec2(uv.x, -uv.y);
    return (idx != -1) ? texture(textures[idx], uvCor) : fallback;
}

uint atomicWarpAdd(uint counterIndex, int number)
{
    uint mask = uint(ballotARB(true));
    int leader = findLSB(mask);
    uint res = 0;
    if (gl_SubGroupInvocationARB == leader)
        res = atomicAdd(queueCounters[counterIndex], number * bitCount(mask));
    res = readInvocationARB(res, leader);
    return res + bitCount(mask & ((1 << gl_SubGroupInvocationARB) - 1));
}


