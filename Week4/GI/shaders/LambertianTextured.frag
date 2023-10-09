
#version 410

uniform sampler2D albedoTexture;
uniform vec3 lightPosWorld;

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

void main()
{
	vec3 albedo = texture(albedoTexture, texCoord).xyz;


	vec3 lightDir = normalize(fragPosWorld - lightPosWorld);
	float lightDot = clamp(dot(lightDir, -normalize(worldNorm)), 0, 1);

	// Add your illumination code here!

	colorOut.xyz = albedo * lightDot;
	colorOut.a = 1.0;
}

