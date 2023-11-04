
#version 410

smooth in vec2 texCoord;

uniform sampler2D tex;
uniform float time;

out vec4 color;

void main()
{
	color = texture(tex, texCoord);
	// Vary the lighting a little bit to fake firelight affecting fireplace.
	color.rgb *= 0.9 + 0.1 * sin(time*10) * cos(time);
	color.a = 1.0;
}

