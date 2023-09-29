
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

uniform vec4 albedo;
uniform float specularExponent;
uniform float specularIntensity;

uniform int lightingMode;
uniform vec3 lightPosWorld;
uniform float lightIntensity;

void main()
{
	vec3 lightDir = normalize(lightPosWorld - fragPosWorld);
	vec3 viewDir = normalize(cameraPos.xyz - fragPosWorld);
	float lightDistance = length(lightPosWorld - fragPosWorld);

	colorOut = albedo;

	if(lightingMode == 0) {
		// Render with original Phong lighting model
		// --- Your Code Here ---
	} else if(lightingMode == 1) {
		// Render with Blinn-Phong
		// --- Your Code Here ---
	} else if(lightingMode == 2) {
		// Render with Modified Blinn-Phong
		// --- Your Code Here ---
	} else if(lightingMode == 3) {
		// Render with Modified Blinn-Phong, Normalized
		// --- Your Code Here ---
	}
}

