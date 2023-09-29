
#version 410

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
in vec3 worldNorm;
in vec3 worldTangent;
in vec3 fragPosWorld;
in vec2 texCoord;

out vec4 colorOut;
uniform vec4 color;
uniform vec3 lightPosWorld;
uniform sampler2D texture;
uniform vec3 hairColor;
uniform float specularIntensity;
uniform float specularExponent;

uniform float selfShadowMin;
uniform float selfShadowFadeoutWidth;

void main()
{
	vec3 albedo = texture2D(texture, texCoord).rgb;

	colorOut.rgb = albedo * hairColor;

	// --- Your Code Here ---
	// Implement the Kajiya-Kay anisotropic shader.

	colorOut.a = 1.0;
}

