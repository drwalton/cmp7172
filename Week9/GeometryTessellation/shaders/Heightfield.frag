#version 410

in vec2 fTex;

uniform sampler2D colorTexture;

out vec4 color;

void main()
{
	color = texture(colorTexture, fTex);
}

