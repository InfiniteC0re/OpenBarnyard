#version 400
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec2 o_TexCoord;
in vec3 o_Normal;
in vec3 o_Position;
in vec3 o_ViewPos;
in vec4 o_FinalColor;

uniform sampler2D tex0;

void main()
{
	vec4 texColor = texture(tex0, o_TexCoord);
	
	// TODO: Make alpha reference an uniform value to control it from code
	if (texColor.a < 0.00392156862745098f) discard;
	
	gPosition = vec4(o_Position, 1.0f);
	gNormal = vec4(o_Normal, 1.0f);
	gColor = texColor;
	//gColor = texColor * clamp(o_FinalColor, 0.0f, 1.0f);
}
