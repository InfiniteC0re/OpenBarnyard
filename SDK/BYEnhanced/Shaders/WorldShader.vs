#version 400

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in vec2 a_UV;

out vec2 textureCoord;
out vec3 vertexColor;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
	textureCoord = a_UV;
	vertexColor = a_Color;
	gl_Position = u_Projection * u_View * vec4(a_Position, 1.0);
}