
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
layout(location = 4) in vec3 vTangent;
layout(location = 5) in vec3 vBitangent;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

out vec2 texCoord;
out vec3 fragPosWorld;


void main()
{
	// --- Your code here ---
	// Prepare any extra information you need to pass to the fragment shader
	// E.g. working out the camera position in tangent space.

	texCoord = vTex;
	gl_Position = worldToClip *  modelToWorld * vec4(vPos, 1.0f);
	fragPosWorld = (modelToWorld * vec4(vPos, 1.0)).xyz;
}

