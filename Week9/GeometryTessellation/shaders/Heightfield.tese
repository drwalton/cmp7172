#version 410

layout(quads, equal_spacing, cw) in;

uniform sampler2D depthTexture;
layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};

uniform mat4 modelToWorld;
uniform float depthScaling;

in vec2 eTex[];

out vec2 fTex;

void main() 
{
	// Implement your tessellation evalution shader here.
	// First find the texture coordinate for this tessellated vertex (use linear
	// interpolation) and sample the depth texture.
	// Then find the position of the vertex - linearly interpolate to get the x and z coordinates, and then 
	// use your depth sample to set its y coordinate. Scale the y coordinate by the depthScaling uniform.
}

