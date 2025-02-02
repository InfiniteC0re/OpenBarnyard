#version 400
in vec2 o_TexCoord;
in vec3 o_Normal;
in vec3 o_Position;
in vec3 o_ViewPos;
in vec4 o_FinalColor;

out vec4 color;

uniform sampler2D tex0;

void main()
{
	vec4 texColor = texture(tex0, o_TexCoord);
	
	// TODO: Make alpha reference an uniform value to control it from code
	if (texColor.a < 0.2f) discard;
	
	color = texColor * clamp(o_FinalColor, 0.0f, 1.0f);
}
