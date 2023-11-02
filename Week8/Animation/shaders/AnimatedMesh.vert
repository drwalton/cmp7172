
#version 410

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNorm;
layout(location = 2) in vec2 vTex;
layout(location = 6) in ivec4 vBoneIndices;
layout(location = 7) in vec4 vBoneWeights;

layout(std140) uniform cameraBlock
{
	mat4 worldToClip;
	vec4 cameraPos;
	vec4 cameraDir;
};
uniform mat4 modelToWorld;
uniform mat3 normToWorld;

const int MAX_BONES = 40;
const int MAX_BONES_PER_VERTEX = 4;

uniform mat4 boneMatrices[MAX_BONES];

out vec2 texCoord;
out vec3 worldNorm;

void main()
{
	vec4 animatedPosition = vec4(0.0);
	vec3 animatedNormal = vec3(0.0);

	// Your code here
	// replace the lines below with code that finds the blended position and normal
	// using linear vertex skinning.
	// For now you can assume the matrices don't perform scaling (so just taking the top 3x3 block to transform the normal is fine).
	// Remember that a bone index of -1 means that less than 4 bones affect a vertex (you can skip these indices).
	animatedPosition = vec4(vPos, 1.0);
	animatedNormal = vNorm;

	animatedPosition.w = 1.0;

	worldNorm = normalize(normToWorld * animatedNormal);
	texCoord = vTex;
	gl_Position = worldToClip *  modelToWorld * animatedPosition;
}

