
#version 410

out vec4 colorOut;
uniform vec4 color;

in vec3 worldNorm;
in vec3 fragPosWorld;

uniform vec3 lightPosWorld;

void main()
{
	float lighting = clamp(dot(normalize(lightPosWorld - fragPosWorld), normalize(worldNorm)), 0, 1);
	colorOut = vec4(color.rgb * lighting, color.a);
}

