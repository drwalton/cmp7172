
#version 410

layout(location = 0) in vec3 vPos;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 shadowWorldToClip;
uniform mat4 modelToWorld;

out vec3 fragPosWorld;

void main()
{
	fragPosWorld = (modelToWorld * vec4(vPos, 1.0)).xyz;
	gl_Position = shadowWorldToClip *  modelToWorld * vec4(vPos, 1.0);
}

