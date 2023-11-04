#version 410

in vec2 fTex;
in vec3 worldNorm;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform vec3 lightDir;

out vec4 color;

void main()
{
	float lighting = clamp(0.3 + dot(normalize(lightDir), normalize(worldNorm)), 0.0, 1.0);

	color = vec4(lighting * texture(colorTexture, fTex).xyz, 1.0);
}

