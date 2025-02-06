#version 400

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in vec2 a_UV;

out vec2 textureCoord;
out vec3 vertexColor;
out vec3 fragPos;
out vec3 normal;
out mat4 worldMatrix;

uniform mat4 u_Projection;
uniform mat4 u_ModelView;
uniform mat4 u_WorldMatrix;

void main()
{
	textureCoord = a_UV;
	vertexColor = a_Color;
	gl_Position = u_Projection * u_ModelView * vec4(a_Position, 1.0);

	worldMatrix = u_WorldMatrix;
	fragPos = (u_WorldMatrix * vec4(a_Position, 1.0)).xyz;
	normal = mat3(u_WorldMatrix) * a_Normal;
}