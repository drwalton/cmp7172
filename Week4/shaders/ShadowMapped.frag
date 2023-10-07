
#version 410

out vec4 colorOut;
uniform vec4 color;
in vec3 fragPosWorld;
in vec3 worldNorm;

uniform samplerCube shadowMap;
uniform vec3 lightPosWorld;

uniform float nearPlane, farPlane;

void main()
{
	// Your code here
	// Perform a shadow test here - compare the real distance from light to fragment
	// with the value you load from your depth texture.
	// Don't forget to scale your depth back to the right range using the farPlane
	// and nearPlane values.
	// If the point is in shadow, scale down the light intensity

	vec3 lightDir = normalize(lightPosWorld - fragPosWorld);
	float lightDot = clamp(dot(lightDir, normalize(worldNorm)), 0, 1);
	vec3 colorRgb = color.rgb * lightDot;

	colorOut.rgb = colorRgb;
	colorOut.a = 1.0;
}

