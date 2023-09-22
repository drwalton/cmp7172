
#version 410

in vec2 texCoord;
in vec3 worldNorm;

const vec3 sunDir = normalize(vec3(1.f, 0.f, 1.f));

uniform sampler2D tex;

out vec4 color;

void main()
{
	float lightIntensity = clamp(dot(sunDir, normalize(worldNorm)), 0, 1);
	color = vec4(lightIntensity * texture(tex, texCoord).xyz, 1.0);
}

