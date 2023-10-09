
#version 410

uniform sampler2D albedoTexture;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
in vec3 worldNorm;
in vec3 worldPos;
in vec2 texCoord;

out vec4 colorOut;
uniform vec4 color;

void main()
{
	vec3 albedo = texture(albedoTexture, texCoord).xyz;

	// Add your illumination code here!

	colorOut.xyz = albedo;
	colorOut.a = 1.0;
}

