#version 430
layout(binding=0) uniform sampler2D srcTex;
in vec2 texCoord;
out vec4 fragColor;
void main() {
    fragColor = texture(srcTex, texCoord);
}
