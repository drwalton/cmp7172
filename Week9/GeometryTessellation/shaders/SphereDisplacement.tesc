#version 410

layout (vertices=3) out;

in vec2 cTex[];

out vec2 eTex[];

uniform float tessLevel = 16.0;

void main()
{
	// Implement your shader here.
	// Pass through the position and texture coordinates for all invocations.
	// Then, set tessellation levels for invocation 0.
	// Recall this time that triangles have 3 outer and 1 inner tessellation levels.
}

