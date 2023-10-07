
#version 410

out vec4 colorOut;
uniform vec4 color;
in vec3 fragPosWorld;
in vec3 worldNorm;

uniform samplerCube shadowMap;
uniform vec3 lightPosWorld;

uniform float nearPlane, farPlane;

uniform float lightRadius;
uniform int sampleRadius;
uniform float bias;

void main()
{
	// Your code here
	// Implement shadow mapping with PCF
	// Perform multiple shadow tests for different locations within the light source
	// Average the results and darken the fragment as appropriate.

	vec3 lightDir = normalize(lightPosWorld - fragPosWorld);
	float lightDot = clamp(dot(lightDir, normalize(worldNorm)), 0, 1);
	vec3 colorRgb = color.rgb * lightDot;


	colorOut.rgb = colorRgb;
	colorOut.a = 1.0;
}

