
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

uniform mat4 myNormToWorld;

out VertexData {
	vec3 worldNorm;
} VertexOut;

void main()
{
	VertexOut.worldNorm = (myNormToWorld * vec4(vNorm, 1.0f)).xyz;
	gl_Position = modelToWorld * vec4(vPos.xyz, 1.0f);
}

