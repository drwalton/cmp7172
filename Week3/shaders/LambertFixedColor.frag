
#version 410

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
in vec3 worldNorm;
in vec3 fragPosWorld;
in vec2 texCoord;

out vec4 colorOut;
uniform vec4 color;
uniform vec3 lightPosWorld;

void main()
{
	vec3 albedo = color.xyz;

	vec3 lightDir = normalize(lightPosWorld - fragPosWorld);
	float dotProd = clamp(dot(lightDir, normalize(worldNorm)), 0, 1);

	colorOut.xyz = albedo * dotProd;
	colorOut.a = 1.0;
}

