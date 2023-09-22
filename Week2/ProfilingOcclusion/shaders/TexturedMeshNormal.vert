#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
// --- Your code here ---
// Add vertex input for the tangent and bitangent here.
// The mesh class sets the tangent to location 4 and the
// bitangent to location 5.

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
out mat3 TBN;
out vec3 normWorld;

void main()
{
	texCoord = vTex;
	gl_Position = worldToClip *  modelToWorld * vec4(vPos, 1.0f);
	fragPosWorld = (modelToWorld * vec4(vPos, 1.0f)).xyz;
	normWorld = normToWorld * vNorm;
}

