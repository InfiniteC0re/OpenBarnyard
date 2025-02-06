#version 420
out vec4 FragColor;

in vec2 o_TexCoord;

layout(binding = 0) uniform sampler2D u_HDRBuffer;

void main()
{
	
	
	const float gamma = 0.5f;
	const float exposure = 0.8f;
    vec3 hdrColor = texture(u_HDRBuffer, o_TexCoord).rgb;
	
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));
  
    FragColor = vec4(mapped, 1.0);
	
    //FragColor.rgb = pow(hdrColor.rgb, vec3(1.0/gamma));
	//FragColor.a = 1.0f;
}