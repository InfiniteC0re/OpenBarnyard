#version 400

out vec4 color;

in vec4 o_Colour;
in vec2 o_TexCoord;

uniform sampler2D tex0;

void main()
{
	vec4 texColor = texture(tex0, o_TexCoord);
	color = texture(tex0, o_TexCoord) * o_Colour;
}
