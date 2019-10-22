#version 430
layout(location = 0) in vec2 pos;
out vec2 texCoord;
void main() {
	texCoord = pos * 0.5 + vec2(0.5, 0.5);
	gl_Position = vec4(pos, 0.0, 1.0);
}
