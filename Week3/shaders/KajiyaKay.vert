
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
layout(location = 4) in vec3 vTangent;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

out vec3 worldNorm;
out vec3 fragPosWorld;
out vec2 texCoord;
out vec3 worldTangent;

void main()
{
	gl_Position = worldToClip *  modelToWorld * vec4(vPos, 1.0);
	fragPosWorld = (modelToWorld * vec4(vPos, 1.0)).xyz;
	texCoord = vTex;
	worldNorm = normToWorld * vNorm;

	// --- Your Code Here ---
	// Also pass the tangent along the hair strands to the fragment shader.
}

