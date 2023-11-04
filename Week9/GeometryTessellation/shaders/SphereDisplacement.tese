#version 410

layout(triangles, equal_spacing, ccw) in;

uniform sampler2D heightTexture;
layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

uniform mat4 modelToWorld;
uniform mat3 normToWorld;
uniform float heightScale;
uniform float radius;

in vec2 eTex[];

out vec2 fTex;
out vec3 worldNorm;

void main() 
{
	// Implement your shader here.
	// Use barycentric interpolation to get the initial position, and the texture coordinate.
	// Sample from the height texture to get the height displacement.
	// Set the length of the position to be equal to the radius uniform (snap the vertex out to the sphere of radius "radius").
	// Finally, offset the position outward based on heightScale and the sampled height from the texture.
	// Don't forget to pass through the world normal (the model space normal is the normalised position, and use normToWorld to
	// take this to world space).
}

