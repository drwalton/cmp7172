#version 410

layout (vertices=4) out;

in vec2 cTex[];

out vec2 eTex[];

uniform float tessLevel;

void main()
{
	// Implement your tessellation control shader here.
	// First pass through the position and texture coordinates for
	// the vertex belonging to this gl_InvocationID.

	// Next, for gl_InvocationID 0 only, set all tessellation levels to tessLevel.
}

