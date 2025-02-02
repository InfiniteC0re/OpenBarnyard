#version 400
in vec2 textureCoord;
in vec3 vertexColor;

out vec4 color;

uniform vec4 u_Color;
uniform vec4 u_ShadowColor;
uniform vec4 u_AmbientColor;
uniform sampler2D tex0;

void main()
{
	vec4 texColor = texture(tex0, textureCoord);
	if (texColor.a < 0.2) discard;
	vec3 shadow = vertexColor;
	color = texColor * mix(u_AmbientColor, u_ShadowColor, vec4(1 - vertexColor, 1.0f));
}