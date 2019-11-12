#version 430

layout(local_size_x = 64) in;

struct Ray
{
    vec3 origin;
    int rayID;
    vec3 direction;
};
layout(std430, binding = 1) buffer RaysBuffer
{
    Ray rays[];
};

struct Ray
{
    vec3 origin;
    int rayID;
    vec3 direction;
};
layout(std430, binding = 1) buffer ShadowRaysBuffer
{
    Ray shadowRays[];
};

struct IntersectionResult
{
    vec3 normal;
    int materialID;
    vec3 position;
    int rayID;
    vec2 textureUV;
};
layout(std430, binding = 1) buffer IntersectionResultsBuffer
{
    IntersectionResult intersections[];
};
vec3 correctGamma(vec3 color, float gamma)
{
    vec3 mapped = color / (color + vec3(1.0));
    return pow(mapped, vec3(1.0 / gamma));
}

const float PI = 3.14159265f;
const float invPI = 1.0f / PI;
const float GAMMA = 2.2;
vec3 sampleEnviroment(vec3 direction, float lod)
{
        float theta = acos(-direction.y) * -invPI;
        float phi = atan(direction.x, -direction.z) * -invPI * 0.5f;
        vec3 image = textureLod(hdriTexture, vec2(phi, theta), lod).xyz;
        return image;
}

vec3 pick_random_point_in_sphere(){
  float x0,x1,x2,x3,d2;
  do{
    x0=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x1=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x2=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    x3=rand(2 * ivec2(gl_GlobalInvocationID.xy) - 1);
    d2=x0*x0+x1*x1+x2*x2+x3*x3;
  } while(d2>1.0f);
  float scale = 1.0f / d2;
  return vec3(2*(x1*x3+x0*x2)*scale,
                  2*(x2*x3+x0*x1)*scale,
                  (x0*x0+x3*x3-x1*x1-x2*x2)*scale);
}

vec3 pick_random_point_in_semisphere(vec3 v){
  vec3 result=pick_random_point_in_sphere();
  if(dot(v, result)<0){
    result.x=-result.x;
    result.y=-result.y;
    result.z=-result.z;
  }
  return normalize(result);
}

vec3 Shade(inout Ray ray, IntersectionResult hit)
{
    if (hit.materialID > -1)
    {
        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);

//        vec4 directionalLight = vec4(-2.0, 5.0, 10.0, 1.0);
//        vec3 lightPosition = vec3(2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.x, 2 * rand(ivec2(gl_GlobalInvocationID.xy)) + directionalLight.y, directionalLight.z);
//        vec3 shadowRayDirection = normalize(lightPosition - ray.origin.xyz);
        vec3 lightDirection = pick_random_point_in_semisphere(hit.normal.xyz);
        vec3 shadowRayDirection = lightDirection;

        // Return nothing// Shadow test ray
//        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, vec4(shadowRayDirection, 0.0));
//        RayHit shadowHit = Trace(shadowRay, true);
//        if (shadowHit.t != FLT_MAX)
//        {
//            return vec3(0.0f);
//        }
//        return hit.albedo.xyz;
        vec3 lightColor = sampleEnviroment(lightDirection, 2.0);
        return clamp(dot(hit.normal.xyz, shadowRayDirection), 0.0, 1.0) * lightColor * hit.albedo.xyz;
//        return clamp(dot(hit.normal.xyz, shadowRayDirection), 0.0, 1.0) * directionalLight.w * hit.albedo.xyz;
    }
    else
    {
        return sampleEnviroment(ray.direction.xyz, 0.0);
    }
}

uniform float colorMultiplier;
void main()
{   
    Ray ray = rays[int(gl_GlobalInvocationID.x)];
    IntersectionResult intersection = intersections[int(gl_GlobalInvocationID.x)];
    Shade(ray, intersection);
    intersections[gl_GlobalInvocationID.x] = intersection;
}
