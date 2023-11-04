
#version 410

layout(location = 0) in vec4 vPos;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

void main()
{
	gl_Position = vec4(vPos.xyz, 1.0f);
}

