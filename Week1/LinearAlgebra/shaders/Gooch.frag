#version 410

smooth in vec3 norm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 fragColor;

const vec4 warmColor = vec4(0.8, 0.2, 0.2, 1.0);
const vec4 coolColor = vec4(0.2, 0.2, 0.8, 1.0);

void main()
{
	float vDotN = clamp(dot(-cameraDir.xyz, normalize(norm)), 0.0, 1.0);
	fragColor = mix(coolColor, warmColor, vDotN);
}
