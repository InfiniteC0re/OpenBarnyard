#version 420
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;

in vec2 o_TexCoord;
in vec3 o_Normal;
in vec3 o_Position;
in vec3 o_ViewPos;
in vec2 o_LightingTexCoord0;
in vec2 o_LightingTexCoord1;
in vec2 o_LightingTexCoord2;
in vec2 o_LightingTexCoord3;

uniform vec4 u_AmbientColor;
uniform vec4 u_LightColour;
uniform vec3 u_LightDirection;
uniform vec4 u_LightingLerp1;
uniform vec4 u_LightingLerp2;

layout(binding = 0) uniform sampler2D tex0;
layout(binding = 1) uniform sampler2D Lighting1;
layout(binding = 2) uniform sampler2D Lighting2;
layout(binding = 3) uniform sampler2D Lighting3;
layout(binding = 4) uniform sampler2D Lighting4;

void main()
{
	gPosition = vec4(o_Position, 1.0f);
	gNormal = vec4(o_Normal, 1.0f);
	
	vec4 lighting1Color = texture(Lighting1, o_LightingTexCoord0);
	vec4 lighting2Color = texture(Lighting2, o_LightingTexCoord1);
	vec4 lighting3Color = texture(Lighting3, o_LightingTexCoord2);
	vec4 lighting4Color = texture(Lighting4, o_LightingTexCoord3);
	vec4 shade1 = mix(lighting3Color, lighting1Color, u_LightingLerp1);
	vec4 shade2 = mix(lighting4Color, lighting2Color, u_LightingLerp1);
	
	gColor = texture(tex0, o_TexCoord);
	//gColor.rgb = clamp(gColor.rgb - shade1.rgb, 0.0f, 1.0f);
	//gColor.rgb = clamp(gColor.rgb + shade2.rgb, 0.0f, 1.0f);
	gColor.a *= u_AmbientColor.a;
}