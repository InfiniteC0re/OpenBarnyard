#version 460

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
uniform mat4 u_WorldView;
uniform mat4 u_ModelView;

layout(std430, binding = 3) buffer multidrawbuffer
{
	int indices[8000];
    mat4 modelViews[8000];
};

void main()
{
	textureCoord = a_UV;
	vertexColor = a_Color;
	gl_Position = u_Projection * modelViews[indices[gl_BaseInstance + gl_InstanceID]] * vec4(a_Position, 1.0);

	worldMatrix = inverse(u_WorldView) * modelViews[indices[gl_BaseInstance]];
	fragPos = (worldMatrix * vec4(a_Position, 1.0)).xyz;
	normal = mat3(worldMatrix) * a_Normal;
}