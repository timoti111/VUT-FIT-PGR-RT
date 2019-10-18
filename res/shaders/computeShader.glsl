#version 430
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

struct Camera {
	vec3 position;
	vec3 direction;
	vec3 up;
	vec3 left;
	float sensorHalfWidth;
};

uniform Camera camera;

struct Ray {
	vec3 origin;
	vec3 direction;
	vec3 energy;
};

Ray CreateRay(vec3 origin, vec3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = vec3(1.0, 1.0, 1.0);
    return ray;
}

Ray CreateCameraRay(vec2 pixel, ivec2 size)
{
	float sensorHalfHeight = (size.y * camera.sensorHalfWidth) / size.x;
	float pixelSize = camera.sensorHalfWidth / (size.x * 0.5);
	vec3 direction = normalize(camera.direction + camera.left * (camera.sensorHalfWidth - (pixel.x + 0.5) * pixelSize) + camera.up * ((pixel.y + 0.5) * pixelSize - sensorHalfHeight));
    return CreateRay(camera.position, direction);
}

struct RayHit {
	float t;
	vec3 position;
	vec3 normal;
    vec3 albedo;
    vec3 specular;
};

RayHit CreateRayHit()
{
    RayHit hit;
    hit.position = vec3(0.0f, 0.0f, 0.0f);
    hit.t = FLT_MAX;
    hit.normal = vec3(0.0f, 0.0f, 0.0f);
    hit.albedo = vec3(0.0f, 0.0f, 0.0f);
    hit.specular = vec3(0.0f, 0.0f, 0.0f);
    return hit;
};

void IntersectGroundPlane(Ray ray, inout RayHit bestHit)
{
    // Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.t)
    {
        bestHit.t = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec3(0.0f, 1.0f, 0.0f);
        bestHit.albedo = vec3(0.9, 0.9, 0.9);
        bestHit.specular = vec3(0.1f, 0.1f, 0.1f);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, vec4 sphere) {
    // Calculate distance along the ray where the sphere is intersected
    vec3 d = ray.origin - sphere.xyz;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.w * sphere.w;
    if (p2sqr < 0)
        return;
    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.t)
    {
        bestHit.t = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.xyz);
        bestHit.albedo = vec3(0.5, 0.5, 0.5);
        bestHit.specular = vec3(0.3, 0.3, 0.3);
	}
}

RayHit Trace(Ray ray)
{
    RayHit bestHit = CreateRayHit();
//    IntersectGroundPlane(ray, bestHit);
	IntersectSphere(ray, bestHit, vec4(0, 1.0, -2.0, 1.0f));
	IntersectSphere(ray, bestHit, vec4(-2, 1.0, -4.0, 1.0f));
	IntersectSphere(ray, bestHit, vec4(2, 1.0, -4.0, 1.0f));
	IntersectSphere(ray, bestHit, vec4(0, 1.0, -6.0, 1.0f));
    return bestHit;
}

layout(binding=2) uniform sampler2D hdriTexture;
const float PI = 3.14159265f;
vec3 Shade(inout Ray ray, RayHit hit)
{
    if (hit.t < FLT_MAX)
    {
		vec4 directionalLight = vec4(-2.0, 5.0, 10.0, 1.0);
        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);
		vec3 rayDirection = normalize(directionalLight.xyz - ray.origin);
        // Return nothing// Shadow test ray
		bool shadow = false;
		Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, rayDirection);
		RayHit shadowHit = Trace(shadowRay);
		if (shadowHit.t != FLT_MAX)
		{
			return vec3(0.0f, 0.0f, 0.0f);
		}
        return clamp(dot(hit.normal, rayDirection), 0.0, 1.0) * directionalLight.w * hit.albedo;
    }
    else
    {
		float theta = acos(ray.direction.y) / -PI;
		float phi = atan(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
		vec3 image = texture(hdriTexture, vec2(1 - phi, 1 - theta)).xyz;
		return vec3(sqrt(image.x),sqrt(image.y),sqrt(image.z));
    }
}

//vec3 color(Ray r) {
//	HitRecord rec;
//	bool hit_anything = false;
//	float closest_so_far = FLT_MAX;
//	if (hitSphere(r, vec3(0, 0, -1), 0.5, 0.0, closest_so_far, rec)) {
//		hit_anything = true;
//		closest_so_far = rec.t;
//	}
//	if (hitSphere(r, vec3(0, -100.5, -1), 100, 0.0, closest_so_far, rec)) {
//		hit_anything = true;
//		closest_so_far = rec.t;	
//	}
//	if (hit_anything) {
//		return 0.5 * vec3(rec.normal.x + 1, rec.normal.y + 1, rec.normal.z + 1);
//	}
//	float t = 0.5 * (r.direction.y + 1.0);
//	return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
//}

float seed = 0.0;
float rand(ivec2 pixel)
{
    float result = fract(sin(seed / 100.0f * dot(vec2(pixel), vec2(12.9898f, 78.233f))) * 43758.5453f);
    seed += 1.0f;
    return result;
}

//float rand()
//{
//    seed = (seed ^ 61) ^ (seed >> 16);
//    seed *= 9;
//    seed = seed ^ (seed >> 4);
//    seed *= 0x27d4eb2d;
//    seed = seed ^ (seed >> 15);
//    return float(seed) * (1.0 / 4294967296.0);
//}

//vec3 randomInUnitSphere() {
//	vec3 p;
//	do {
//	   p = 2.0*vec3(rand(), rand(), rand()) - vec3(1,1,1);
//	} while (pow(p.x, 2) + pow(p.y, 2) + pow(p.z, 2) >= 1.0);
//	return p;
//
//}

layout(binding=1) writeonly uniform image2D destTex;
layout (local_size_x = 8, local_size_y = 8) in;
void main() {
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
	ivec2 size = imageSize(destTex);
	if (storePos.x >= size.x || storePos.y >= size.y) {
		return;
	}
	
	vec3 result = vec3(0, 0, 0);
	int samples = 1;
	for (int i = 0; i < samples; i++)
	{
		Ray ray = CreateCameraRay(vec2(storePos.x + rand(storePos) - 0.5, storePos.y + rand(storePos) - 0.5), size);
		for (int i = 0; i < 8; i++)
		{
			RayHit hit = Trace(ray);
			result += ray.energy * Shade(ray, hit);
			if (hit.t == FLT_MAX || all(equal(ray.energy, vec3(0.0, 0.0, 0.0)))) {
				break;
			}
			ray.energy *= hit.specular;
		}
	}
	imageStore(destTex, storePos, vec4(result / samples, 1.0));
}
