#version 460
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_UV;

out vec2 o_TexCoord;

void main()
{
	o_TexCoord = a_UV;
	gl_Position = vec4(a_Position, 0.0, 1.0);
}
