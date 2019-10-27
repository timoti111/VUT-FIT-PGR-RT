#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define TRIANGLES 256
layout(local_size_x = 16, local_size_y = 16) in;

struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 left;
    float sensorHalfWidth;
};

uniform Camera camera;

struct MeshObject
{
    mat4 localToWorldMatrix;
    int indices_offset;
    int indices_count;
};

layout(rgba32f, binding = 1) uniform image2D destTex;
layout(binding = 2) uniform sampler2D hdriTexture;
layout(std430, binding = 3) buffer VerticeBuffer
{
    vec4 vertices[];
};
layout(std430, binding = 4) buffer IndiceBuffer
{
    ivec4 indices[];
};
layout(std430, binding = 5) buffer MeshBuffer
{
    MeshObject meshes[];
};
layout(std430, binding = 6) buffer NormalBuffer
{
    vec4 normals[];
};

struct Ray
{
    vec4 origin;
    vec4 direction;
    vec4 energy;
};

Ray CreateRay(vec4 origin, vec4 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = vec4(1.0f);
    return ray;
}

Ray CreateCameraRay(vec2 pixel, ivec2 size)
{
//    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
//    float pixelSize = camera.sensorHalfWidth / (size.x * 0.5f);
//    vec4 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - (pixel.x + 0.5f) * pixelSize) + camera.up * ((pixel.y + 0.5f) * pixelSize - sensorHalfHeight));
//    return CreateRay(camera.position, direction);
    float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
    float pixelSize = sensorHalfHeight / (size.y * 0.5f);
    vec4 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - pixel.x * pixelSize) + camera.up * (pixel.y * pixelSize - sensorHalfHeight));
    return CreateRay(camera.position, direction);
}

struct RayHit
{
    vec4 position;
    vec4 normal;
    vec4 albedo;
    vec3 specular;
    float t;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = vec4(0.0f);
    hit.t = FLT_MAX;
    hit.normal = vec4(0.0f);
    hit.albedo = vec4(0.0f);
    hit.specular = vec3(0.0f);
    return hit;
};

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
//     Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.t)
    {
        bestHit.t = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec4(0.0f, 1.0f, 0.0f, 0.0f);
        bestHit.albedo = vec4(0.9f);
        bestHit.specular = vec3(0.1f);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, vec4 sphere, float radius)
{
    // Calculate distance along the ray where the sphere is intersected
    vec4 d = ray.origin - sphere;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + radius * radius;
    if (p2sqr < 0)
        return;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0.0f ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.t)
    {
        bestHit.t = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere);
        bestHit.albedo = vec4(0.5f);
        bestHit.specular = vec3(0.3f);
    }
}

const float EPSILON = 1e-8;
bool IntersectTriangle(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2,
                       inout float t, inout float u, inout float v)
{
    // find vectors for two edges sharing vert0
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    // begin calculating determinant - also used to calculate U parameter
    vec3 pvec = cross(ray.direction.xyz, edge2);
    // if determinant is near zero, ray lies in plane of triangle
    float det = dot(edge1, pvec);
    // use backface culling
    if (det < EPSILON)
        return false;
    float inv_det = 1.0f / det;
    // calculate distance from vert0 to ray origin
    vec3 tvec = ray.origin.xyz - vert0;
    // calculate U parameter and test bounds
    u = dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f)
        return false;
    // prepare to test V parameter
    vec3 qvec = cross(tvec, edge1);
    // calculate V parameter and test bounds
    v = dot(ray.direction.xyz, qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f)
        return false;
    // calculate t, ray intersects triangle
    t = dot(edge2, qvec) * inv_det;
    return true;
}

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
};
bool IntersectTriangles(Ray ray, Triangle triangles[4],
                        inout float tOut, inout float uOut,
                        inout float vOut, inout int iOut)
{
    vec3 e11 = triangles[0].v1 - triangles[0].v0;
    vec3 e21 = triangles[0].v2 - triangles[0].v0;
    vec3 e12 = triangles[1].v1 - triangles[1].v0;
    vec3 e22 = triangles[1].v2 - triangles[1].v0;
    vec3 e13 = triangles[2].v1 - triangles[2].v0;
    vec3 e23 = triangles[2].v2 - triangles[2].v0;
    vec3 e14 = triangles[3].v1 - triangles[3].v0;
    vec3 e24 = triangles[3].v2 - triangles[3].v0;

    vec4 v0x = vec4(triangles[0].v0.x, triangles[1].v0.x, triangles[2].v0.x, triangles[3].v0.x);
    vec4 v0y = vec4(triangles[0].v0.y, triangles[1].v0.y, triangles[2].v0.y, triangles[3].v0.y);
    vec4 v0z = vec4(triangles[0].v0.z, triangles[1].v0.z, triangles[2].v0.z, triangles[3].v0.z);
    vec4 e1x = vec4(e11.x, e12.x, e13.x, e14.x);
    vec4 e1y = vec4(e11.y, e12.y, e13.y, e14.y);
    vec4 e1z = vec4(e11.z, e12.z, e13.z, e14.z);
    vec4 e2x = vec4(e21.x, e22.x, e23.x, e24.x);
    vec4 e2y = vec4(e21.y, e22.y, e23.y, e24.y);
    vec4 e2z = vec4(e21.z, e22.z, e23.z, e24.z);
    vec4 dir4x = ray.direction.xxxx;
    vec4 dir4y = ray.direction.yyyy;
    vec4 dir4z = ray.direction.zzzz;
    vec4 pvecx = dir4y * e2z - dir4z * e2y;
    vec4 pvecy = dir4z * e2x - dir4x * e2z;
    vec4 pvecz = dir4x * e2y - dir4y * e2x;
    vec4 divisor = pvecx * e1x + pvecy * e1y + pvecz * e1z;
    vec4 invDivisor = vec4(1.0f, 1.0f, 1.0f, 1.0f) / divisor;
    bvec4 backFace = lessThan(invDivisor, vec4(EPSILON));
    vec4 orig4x = ray.origin.xxxx;
    vec4 orig4y = ray.origin.yyyy;
    vec4 orig4z = ray.origin.zzzz;
    vec4 tvecx = orig4x - v0x;
    vec4 tvecy = orig4y - v0y;
    vec4 tvecz = orig4z - v0z;
    vec4 u = tvecx * pvecx + tvecy * pvecy + tvecz * pvecz;
    u *= invDivisor;
    vec4 qvecx = tvecy * e1z - tvecz * e1y;
    vec4 qvecy = tvecz * e1x - tvecx * e1z;
    vec4 qvecz = tvecx * e1y - tvecy * e1x;
    vec4 v = dir4x * qvecx + dir4y * qvecy + dir4z * qvecz;
    v *= invDivisor;
    vec4 t = e2x*qvecx + e2y*qvecy + e2z*qvecz;
    t *= invDivisor;
    tOut = FLT_MAX;

    if(t.x > 0.0f && t.x < tOut)
    {
        if(u.x >= 0.0f && v.x >= 0.0f && u.x + v.x <= 1)
        {
            tOut = t.x;
            uOut = u.x;
            vOut = v.x;
            iOut = 0;
        }
    }
    if(t.y > 0 && t.y < tOut)
    {
        if(u.y >= 0.0f && v.y >= 0.0f && u.y + v.y <= 1)
        {
            tOut = t.y;
            uOut = u.y;
            vOut = v.y;
            iOut = 1;
        }
    }
    if(t.z > 0 && t.z < tOut)
    {
        if(u.z >= 0.0f && v.z >= 0.0f && u.z + v.z <= 1)
        {
            tOut = t.z;
            uOut = u.z;
            vOut = v.z;
            iOut = 2;
        }
    }
    if(t.w > 0 && t.w < tOut)
    {
        if(u.w >= 0.0f && v.w >= 0.0f && u.w + v.w <= 1)
        {
            tOut = t.w;
            uOut = u.w;
            vOut = v.w;
            iOut = 3;
        }
    }

    return tOut != FLT_MAX;
} 
shared vec3 v0s[TRIANGLES];
shared vec3 v1s[TRIANGLES];
shared vec3 v2s[TRIANGLES];
void IntersectMeshObject(Ray ray, inout RayHit bestHit, MeshObject meshObject)
{
    bool hasNormals = normals.length() != 0;
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);
    bool inImageBounds = storePos.x < size.x && storePos.y < size.y;
    int offset = meshObject.indices_offset;
    int numOfIndices = meshObject.indices_count + offset;
    int workSize = int(gl_WorkGroupSize.x * gl_WorkGroupSize.y);
    mat4 transform = meshObject.localToWorldMatrix;
    for (; offset < numOfIndices; offset += TRIANGLES) {
        barrier();
        int trianglesToLoad = min(numOfIndices - offset, TRIANGLES);
        for (int i = int(gl_LocalInvocationIndex); i < trianglesToLoad; i += workSize) {
            ivec3 actualIndex = indices[i + offset].xyz;
            v0s[int(mod(i, TRIANGLES))] = (transform * vertices[actualIndex.x]).xyz;
            v1s[int(mod(i, TRIANGLES))] = (transform * vertices[actualIndex.y]).xyz;
            v2s[int(mod(i, TRIANGLES))] = (transform * vertices[actualIndex.z]).xyz;
        }
        barrier();
        if (inImageBounds)
        {
            for (int j = 0; j < trianglesToLoad; j++)
            {
                vec3 v0 = v0s[j];
                vec3 v1 = v1s[j];
                vec3 v2 = v2s[j];
                float t, u, v;
                if (IntersectTriangle(ray, v0, v1, v2, t, u, v))
                {
                    if (t > 0 && t < bestHit.t)
                    {
                        bestHit.t = t;
                        bestHit.position = ray.origin + t * ray.direction;
                        if (hasNormals) {
                            ivec3 normalIndices = indices[offset + j].xyz;
                            bestHit.normal = (1 - u - v) * normals[normalIndices.x] + u * normals[normalIndices.y] + v * normals[normalIndices.z];
                        }
                        else
                            bestHit.normal = vec4(normalize(cross(v1 - v0, v2 - v0)), 0.0f);
                        bestHit.albedo = vec4(0.5f);
                        bestHit.specular = vec3(0.3f);
                    }
                }
            }
//            for (int j = 0; j < trianglesToLoad; j += 4)
//            {
//                Triangle triangles[4];
//                triangles[0].v0 = v0s[j];
//                triangles[0].v1 = v1s[j];
//                triangles[0].v2 = v2s[j];
//                triangles[1].v0 = v0s[j + 1];
//                triangles[1].v1 = v1s[j + 1];
//                triangles[1].v2 = v2s[j + 1];
//                triangles[2].v0 = v0s[j + 2];
//                triangles[2].v1 = v1s[j + 2];
//                triangles[2].v2 = v2s[j + 2];
//                triangles[3].v0 = v0s[j + 3];
//                triangles[3].v1 = v1s[j + 3];
//                triangles[3].v2 = v2s[j + 3];
//                float t, u, v;
//                int n;
//                if (IntersectTriangles(ray, triangles, t, u, v, n))
//                {
//                    if (j + n < trianglesToLoad)
//                    {
//                        if (t > 0 && t < bestHit.t)
//                        {
//                            bestHit.t = t;
//                            bestHit.position = ray.origin + t * ray.direction;
//                            bestHit.normal = vec4(normalize(cross(triangles[n].v1 - triangles[n].v0, triangles[n].v2 - triangles[n].v0)), 0.0);
//                            bestHit.albedo = vec4(0.5f);
//                            bestHit.specular = vec3(0.3f);
//                        }
//                    }
//                }
//            }
        }
    }
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
    IntersectGroundPlane(ray, bestHit);
    for (int i = 0; i < meshes.length(); i++)
        IntersectMeshObject(ray, bestHit, meshes[i]);
    return bestHit;
}

float localSeed = 0.0;
uniform float globalSeed;
float rand(vec2 seed)
{
    float result = fract(sin((globalSeed + localSeed) / 100.0f * dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
    localSeed += 1.0f + sin(globalSeed);
    return result;
}

const float PI = 3.14159265f;
const float invPI = 1.0f / PI;
vec3 Shade(inout Ray ray, RayHit hit)
{
    if (hit.t < FLT_MAX)
    {
        vec4 directionalLight = vec4(-2.0, 5.0, 10.0, 1.0);
        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);
        vec3 lightPosition = vec3(2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.x, 2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.y, directionalLight.z);
        vec3 rayDirection = normalize(lightPosition - ray.origin.xyz);
        // Return nothing// Shadow test ray
        bool shadow = false;
        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, vec4(rayDirection, 0.0));
        RayHit shadowHit = Trace(shadowRay);
        if (shadowHit.t != FLT_MAX)
        {
            return vec3(0.0f);
        }
        return clamp(dot(hit.normal.xyz, rayDirection), 0.0, 1.0) * directionalLight.w * hit.albedo.xyz;
    }
    else
    {
        float theta = acos(-ray.direction.y) * -invPI;
        float phi = atan(ray.direction.x, -ray.direction.z) * -invPI * 0.5f;
        vec3 image = texture(hdriTexture, vec2(phi, theta)).xyz;
        return sqrt(image);
    }
}

uniform float colorMultiplier;
void main()
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(destTex);
    bool inImageBounds = storePos.x < size.x && storePos.y < size.y;
    
    vec4 result = imageLoad(destTex, storePos) * colorMultiplier;
    Ray ray = CreateCameraRay(vec2(storePos) + vec2(rand(vec2(storePos)), rand(vec2(storePos))), size);

    for (int i = 0; i < 2; i++)
    {
        RayHit hit = Trace(ray);
        result += ray.energy * vec4(Shade(ray, hit), 0.0f);//           
        if (hit.t == FLT_MAX || all(equal(ray.energy, vec4(0.0f))))
        {
            break;
        }
        ray.energy *= vec4(hit.specular, 1.0f);
    }
    if (inImageBounds)
        imageStore(destTex, storePos, result + vec4(0.0f, 0.0f, 0.0f, 1.0f));
}
