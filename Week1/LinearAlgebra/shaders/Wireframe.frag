#version 410

in vec3 barycentric;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 fragColor;

uniform vec4 faceColor;
uniform vec4 lineColor;

void main()
{
	float minBary = min(barycentric[0], min(barycentric[1], barycentric[2]));
	float lineFace = smoothstep(0.0, 0.05, minBary);
	fragColor = mix(lineColor, faceColor, lineFace);
	if(fragColor.a < 0.001) discard;
}
