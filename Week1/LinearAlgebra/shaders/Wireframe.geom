
#version 410

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

out vec3 barycentric;

void main()
{
	barycentric = vec3(1,0,0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	barycentric = vec3(0,1,0);
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	barycentric = vec3(0,0,1);
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	EndPrimitive();
}

