bool IntersectSphere(Ray ray, vec4 sphere, inout RayHit bestHit)
{        
    vec3 sphereDir = sphere.xyz - ray.origin.xyz;
    float tca = dot(sphereDir, ray.direction.xyz);
    float d2 = dot(sphereDir, sphereDir) - tca * tca;
    float radius2 = pow(sphere.w, 2);
    if (d2 > radius2)
        return false; 
    float thc = sqrt(radius2 - d2); 
    float t0 = tca - thc; 
    float t1 = tca + thc; 

    if (t0 < 0)
    {
        t0 = t1; // if t0 is negative, let's use t1 instead 
        bestHit.backfaceHit = true;
    }
 
    if (t0 > 0.0f && t0 < bestHit.t)
    {
        bestHit.t = t0;
        bestHit.position = ray.origin + t0 * ray.direction;
        bestHit.normal = bestHit.position - vec4(sphere.xyz, 1.0f);
        return true;
    }
    return false;
}

bool IntersectCylinder(Ray ray, vec3 A, vec3 B, float R, inout RayHit bestHit)
{
    vec3 V = normalize(B - A);
    vec3 X = ray.origin.xyz - A;
    float DD = dot(ray.direction.xyz, ray.direction.xyz);
    float DV = dot(ray.direction.xyz, V);
    float DX = dot(ray.direction.xyz, X);
    float XX = dot(X, X);
    float XV = dot(X, V);
    float a = DD - pow(DV, 2); // dot product
    float b = 2 * (DX - DV * XV); // dot product
    float c = XX - pow(XV, 2) - pow(R, 2);
    
    float D = pow(b, 2) - 4 * a * c;

    if (D < 0)
        return false;

    D = sqrt(D);
    float denom = 1 / (2 * a);
    float x0 = (-b - D) * denom;
    float x1 = (-b - D) * denom;
    float t0 = min(x0, x1);
    float t1 = max(x0, x1);

    if (t0 < 0.0f)
        t0 = t1;

    float s = DV * t0 + XV;
    if (t0 > 0.0f && t0 < bestHit.t && s >= 0.0f && s <= length(B - A)) 
    {
        bestHit.t = t0;
        bestHit.position = ray.origin + t0 * ray.direction;
        bestHit.normal = bestHit.position - vec4(A, 1.0f) - s * vec4(V, 0.0f);
        return true;
    }
    return false;

}

bool IntersectAABB(Ray ray, vec3 minBound, vec3 maxBound, out vec2 nearFar, float maxT)
{
    vec3 invDir = 1.0f / ray.direction.xyz;
    vec3 originMinBound = minBound - ray.origin.xyz;
    vec3 originMaxBound = maxBound - ray.origin.xyz;
    vec3 t0 = originMinBound * invDir;
    vec3 t1 = originMaxBound * invDir;

    float near = max3(minVec3(t0,t1));
    float far = min3(maxVec3(t0,t1));

    if (near > far || far < 0)
        return false;

    nearFar.x = near;
    nearFar.y = far;

    return near < maxT;
}

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
    if (abs(det) < EPSILON)
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
    return t >= 0.0f;
}
