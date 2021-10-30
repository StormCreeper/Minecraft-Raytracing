#version 430

layout (location = 0) in vec2 aVertexPos;

out vec2 fragPos;
out float log2;
void main() {
	gl_Position = vec4(aVertexPos, 0.0, 1.0);
	fragPos = aVertexPos.xy;
	log2 = log(2.);
}