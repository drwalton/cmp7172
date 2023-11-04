
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

const float flameParticleSize = 0.5;

out float flameTime;
out vec2 texCoords;
out float texXOffset;

void main()
{
	// Write your geometry shader here!
	// This should generate a quad (4 vertices, forming a triangle_strip).
	// Set the data for each vertex, then call EmitVertex() to add it to the strip.
	// Call EndPrimitive() once you've added all 4 vertices.
	// Remember to use cross products to find your across and up vectors for the quad.
	// The size of teh quad should be based on flameParticleSize.
	// Don't forget to pass through the other important info, like flameTime, texCoords,
	// texXOffset.

}

