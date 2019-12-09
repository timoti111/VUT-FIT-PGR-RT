float localSeed = 0.0;
uniform float globalSeed;
float rand(vec2 seed)
{
    float result = fract(sin((globalSeed + localSeed) / 100.0f * dot(seed, vec2(12.9898f, 78.233f))) * 43758.5453f);
    localSeed += 1.0f + sin(globalSeed);
    return result;
}

vec4 pick_random_point_in_sphere(){
  float x0,x1,x2,x3,d2;
  do{
    x0=rand(2 * vec2(gl_GlobalInvocationID.xy) - 1);
    x1=rand(2 * vec2(gl_GlobalInvocationID.xy) - 1);
    x2=rand(2 * vec2(gl_GlobalInvocationID.xy) - 1);
    x3=rand(2 * vec2(gl_GlobalInvocationID.xy) - 1);
    d2=x0*x0+x1*x1+x2*x2+x3*x3;
  } while(d2>1.0f);
  float scale = 1.0f / d2;
  return vec4(2*(x1*x3+x0*x2)*scale,
                  2*(x2*x3+x0*x1)*scale,
                  (x0*x0+x3*x3-x1*x1-x2*x2)*scale, 1.0f);
}

vec4 pick_random_point_in_semisphere(vec3 v){
  vec4 result=pick_random_point_in_sphere();
  if(dot(v, result.xyz)<0){
    result.x=-result.x;
    result.y=-result.y;
    result.z=-result.z;
  }
  return normalize(result);
}

vec4 sampleEnviroment(vec4 direction, float lod)
{
    float theta = acos(direction.y) * INVPI;
    float phi = atan(direction.x, -direction.z) * -INVPI * 0.5f;
    return textureLod(hdriTexture, vec2(phi, theta), lod);
}