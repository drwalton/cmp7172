#version 410

in vec3 fragPosWorld;
in vec2 texCoord;

uniform sampler2D albedoTex;
uniform sampler2D depthTex;
uniform sampler2D normalTex;
uniform float depthScale;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec4 color;
uniform int parallaxMode;
uniform int normalMapMode;
uniform vec3 lightPos;

void main()
{
	float depth = texture(depthTex, texCoord).r;

	vec2 offsetTexCoord = texCoord;
	if(parallaxMode == 0) {
		// --- Your code here ---
		// Implement parallax mapping
		// Find the offset texture coordinate and save it to offsetTexCoord.

	} else if(parallaxMode == 1) {
		// --- Your code here ---
		// Implement parallax mapping with offset limiting
		// Find the offset texture coordinate and save it to offsetTexCoord.

	} else if(parallaxMode == 2) {
		// --- Your code here ---
		// Implement steep parallax mapping
		// Find the offset texture coordinate and save it to offsetTexCoord.
	} 

	// --- Your code here ---
	// Here you may want to discard fragments where offsetTexCoord goes outside
	// the bounds of the texture (0,1)x(0,1)

	// Hacky normal mapping code
	// Note this is not real normal mapping!
	// Really you should transform this normal to world space
	// or alternatively compute the lighting in tangent space (so transform
	// the light and camera locations to tangent space).
	// This is a special case where we're texturing an upward-facing plane
	// so I can just load the normals from the normal map directly.
	vec3 normalWorld;
	if (normalMapMode == 0) {
		// Normal mapping off, so just set normal to straight up in world space.
		normalWorld = vec3(0,1,0);
	} else {
		vec4 normal4 = texture(normalTex, offsetTexCoord);
		normalWorld = normalize(vec3(2.0f * (normal4.r - 0.5f), normal4.b, 2.0f * (normal4.g - 0.5f)));
	}

	// Lighting code
	vec3 lightDir = normalize(lightPos - fragPosWorld);
	vec4 albedo = texture(albedoTex, offsetTexCoord);
	vec4 diffuseColor = vec4(albedo.xyz * dot(normalWorld, lightDir), 1.0f);
	float specularPower = 0.2f * pow(clamp(dot(reflect(lightDir, normalWorld), cameraDir.xyz), 0, 1), 10.0);
	vec4 specularColor = vec4(specularPower * vec3(1,1,1), 1.0);
	color = diffuseColor + specularColor;
}

