
#version 410

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

in VertexData {
	float texXOffset;
	float flameTime;
} VertexIn[];


layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

uniform float particleSize;

out vec2 texCoords;

void main()
{
	vec3 pointPos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(vec3(cameraPos) - vec3(pointPos));

	vec3 across = normalize(cross(toCamera, vec3(0.0, -1.0, 0.0)));
	vec3 up = normalize(cross(toCamera, across));

	// Bottom left vertex
	gl_Position = worldToClip * vec4(pointPos + ((-across + -up)*particleSize), 1.0);
	texCoords = vec2(0,0);
	EmitVertex();

	// Top left vertex
	gl_Position = worldToClip * vec4(pointPos + ((-across + up)*particleSize), 1.0);
	texCoords = vec2(0,1);
	EmitVertex();
	
	// Bottom right vertex
	gl_Position = worldToClip * vec4(pointPos + ((across + -up)*particleSize), 1.0);
	texCoords = vec2(1,0);
	EmitVertex();

	// Top right vertex
	gl_Position = worldToClip * vec4(pointPos + ((across + up)*particleSize), 1.0);
	texCoords = vec2(1,1);
	EmitVertex();

	EndPrimitive();
}

