#version 430
layout(binding = 0) uniform sampler2D srcTex;
in vec2 texCoord;
out vec4 fragColor;

#define GAMMA 2.2
vec4 correctGamma(vec4 color, float gamma)
{
    vec4 mapped = color / (color + vec4(1.0));
    return pow(mapped, vec4(1.0 / gamma));
}

void main()
{
    vec4 color = texture(srcTex, texCoord);
    fragColor = correctGamma(color / color.w, GAMMA);
}
