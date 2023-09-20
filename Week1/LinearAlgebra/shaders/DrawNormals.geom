
#version 410

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in VertexData {
	vec3 worldNorm;
} VertexIn[];


layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

uniform float lineWidth, lineLength;

void main()
{
	vec3 norm = VertexIn[0].worldNorm;
	vec3 pointPos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(vec3(cameraDir));

	vec3 across = normalize(cross(norm, toCamera));
	//vec3 across = vec3(1.0, 0.0, 0.0);

	// Bottom left vertex
	gl_Position = worldToClip * vec4(pointPos + (-across*lineWidth*0.5), 1.0);
	EmitVertex();

	// Top left vertex
	gl_Position = worldToClip * vec4(pointPos + norm*lineLength + (-across*lineWidth*0.5), 1.0);
	EmitVertex();
	
	// Bottom right vertex
	gl_Position = worldToClip * vec4(pointPos + (across*lineWidth*0.5), 1.0);
	EmitVertex();

	// Top right vertex
	gl_Position = worldToClip * vec4(pointPos + norm*lineLength + (across*lineWidth*0.5), 1.0);
	EmitVertex();

	EndPrimitive();
}

