#version 400
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec2 textureCoord;
in vec3 vertexColor;
in vec3 fragPos;
in vec3 normal;
in mat4 worldMatrix;
uniform sampler2D tex0;

void main()
{
	vec4 texColor = texture(tex0, textureCoord);
	if (texColor.a <= 0.05) discard;
	
	gPosition = vec4(fragPos, 1.0f);
	gNormal = vec4(normal, 1.0f);
	gColor = texColor;
}