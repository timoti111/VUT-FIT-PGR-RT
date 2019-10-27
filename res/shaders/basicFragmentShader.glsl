#version 430
layout(binding = 0) uniform sampler2D srcTex;
in vec2 texCoord;
out vec4 fragColor;
void main()
{
    vec4 color = texture(srcTex, texCoord);
    fragColor = color / color.w;
}
