
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
uniform mat3 normToWorld;

uniform float time;

uniform float duration;
uniform float flameHeight;

uniform vec4 flameCubicCoeffts;

out VertexData {
	float texXOffset;
	float flameTime;
} VertexOut;

void main()
{
	// The x coordinate of vTex stores a random x offset for texture sampling
	// we'll pass this on, and use it later in the fragment shader.
	VertexOut.texXOffset = vTex.x;

	// This is the intial position at the base of the flame.
	vec3 initialPos = vPos;

	// The y coordinate of vTex stores a random time offset so the particles
	// don't all start at the same height.
	float timeOffset = vTex.y;

	// flameTime is the current time for this particle, normalised to [0,1].
	// flameTime 0 is the starting pos at the base of the flame
	// flameTime 1 is at the top.
	float flameTime = mod(time + timeOffset*duration, duration);
	flameTime /= duration;
	VertexOut.flameTime = flameTime;

	// Evaluate the given cubic coefficients at the current flameTime.
	float currRadius = flameCubicCoeffts[0] + 
		flameTime * flameCubicCoeffts[1] + 
		flameTime * flameTime * flameCubicCoeffts[2] + 
		flameTime * flameTime * flameTime * flameCubicCoeffts[3];

	// Find the final position - move up in y according to flameTime
	// and move outward according to currRadius.
	vec3 currPos = initialPos * currRadius + vec3(0.f, 1.f, 0.f) * flameHeight * flameTime;

	gl_Position = modelToWorld * vec4(currPos, 1.0f);
}

