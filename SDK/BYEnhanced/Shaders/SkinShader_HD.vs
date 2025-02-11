#version 420

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Weights;
layout(location = 3) in vec4 a_Bones;
layout(location = 4) in vec2 a_UV;

out vec2 o_TexCoord;
out vec3 o_Normal;
out vec3 o_Position;
out vec3 o_ViewPos;
out vec2 o_LightingTexCoord0;
out vec2 o_LightingTexCoord1;
out vec2 o_LightingTexCoord2;
out vec2 o_LightingTexCoord3;

uniform vec3 u_LightDirection;
uniform vec4 u_UpAxis;
uniform vec4 u_LightColourParams;
uniform mat4 u_Projection;
uniform mat4 u_ModelView;
uniform mat4 u_Model;
uniform mat4 u_ViewWorld;
uniform mat4 u_BoneTransforms[28];
uniform int u_NumBones;

void main()
{
	// Apply animation transform for this vertex
	vec3 vertex = vec3(0.0f);
	vec3 normal = vec3(0.0f);
	for (int i = 0; i < 4; i++)
	{
		int boneIndex = int(a_Bones[i] * 256.0 / 3);
		mat4 transform = transpose(u_BoneTransforms[boneIndex]);
		vertex += vec3((vec4(a_Position, 1.0f) * transform) * a_Weights[i]);
		normal += (a_Normal * mat3(transform)) * a_Weights[i];
	}
	
	// Calculate position of the vertex
	gl_Position = (u_Projection * u_ModelView) * vec4(vertex, 1.0f);
	
	o_Position = (u_Model * vec4(vertex, 1.0f)).xyz;
	o_Normal = mat3(u_Model) * normalize(normal);
	o_ViewPos = mat3(u_ViewWorld) * vec3(0.0, 0.0, 0.0);
	o_TexCoord = a_UV;
	
	// Lighting
	vec2 lightingUV;
	lightingUV.x = dot(o_Normal, -u_LightDirection);
	lightingUV.y = dot(o_Normal, vec3(u_UpAxis));
	o_LightingTexCoord0 = lightingUV;
	o_LightingTexCoord1 = lightingUV;
	o_LightingTexCoord2 = lightingUV;
	o_LightingTexCoord3 = lightingUV;
}