#version 400

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_UV;

out vec4 o_Colour;
out vec2 o_TexCoord;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	o_TexCoord = a_UV;
	o_Colour = vec4(a_Colour.b, a_Colour.g, a_Colour.r, a_Colour.a);
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}
