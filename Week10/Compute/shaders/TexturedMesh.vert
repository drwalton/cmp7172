
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 2) in vec2 vTex;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;

smooth out vec2 texCoord;

void main()
{
	texCoord = vTex;
	gl_Position = worldToClip *  modelToWorld * vec4(vPos, 1.0f);
}

