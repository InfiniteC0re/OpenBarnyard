#version 420
in vec2 o_TexCoord;
in vec3 o_Normal;
in vec3 o_Position;
in vec3 o_ViewPos;
in vec2 o_LightingTexCoord0;
in vec2 o_LightingTexCoord1;
in vec2 o_LightingTexCoord2;
in vec2 o_LightingTexCoord3;

out vec4 color;

uniform vec4 u_AmbientColour;
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
	color = texture(tex0, o_TexCoord);
	vec4 lighting1Color = texture(Lighting1, o_LightingTexCoord0);
	vec4 lighting2Color = texture(Lighting2, o_LightingTexCoord1);
	vec4 lighting3Color = texture(Lighting3, o_LightingTexCoord2);
	vec4 lighting4Color = texture(Lighting4, o_LightingTexCoord3);
	vec4 shade1 = mix(lighting3Color, lighting1Color, u_LightingLerp1);
	vec4 shade2 = mix(lighting4Color, lighting2Color, u_LightingLerp1);
	color.rgb = clamp(color.rgb - shade1.rgb, 0.0f, 1.0f);
	color.rgb = clamp(color.rgb + shade2.rgb, 0.0f, 1.0f);
	color.a *= u_AmbientColour.a;
}