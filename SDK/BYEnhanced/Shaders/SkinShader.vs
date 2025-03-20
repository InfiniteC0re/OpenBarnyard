#version 460

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Weights;
layout(location = 3) in vec4 a_Bones;
layout(location = 4) in vec2 a_UV;

out vec2 o_TexCoord;
out vec3 o_Normal;
out vec3 o_Position;
// out vec3 o_ViewPos;
// out vec4 o_FinalColor;

uniform vec4 u_AmbientColor;
uniform vec4 u_LightColour;
uniform vec3 u_LightDirection;
uniform mat4 u_Projection;
uniform mat4 u_ViewWorld;
uniform mat4 u_BoneTransforms[28];
uniform int u_NumBones;

layout(std430, binding = 3) buffer multidrawbuffer {
    mat4 modelViews[];
};

void main() {
    // Apply animation transform for this vertex
    vec3 vertex = vec3(0.0f);
    vec3 normal = vec3(0.0f);
    for (int i = 0; i < 4; i++) {
        int boneIndex = int(a_Bones[i] * 256.0 / 3);
        mat4 transform = transpose(u_BoneTransforms[boneIndex]);
        vertex += vec3((vec4(a_Position, 1.0f) * transform) * a_Weights[i]);
        normal += (a_Normal * mat3(transform)) * a_Weights[i];
    }

    mat4 modelView = modelViews[gl_BaseInstance + gl_InstanceID];
    mat4 modelMatrix = u_ViewWorld * modelView;
    vec4 vertexViewSpace = modelView * vec4(vertex, 1.0);

    // Calculate screen space position of the vertex
    gl_Position = u_Projection * vertexViewSpace;

    o_Position = vertexViewSpace.xyz;
    o_Normal = mat3(transpose(inverse(modelMatrix))) * normalize(normal);
    o_TexCoord = a_UV;

    // Lighting
    // float diffuse = min(max(dot(o_Normal, -u_LightDirection), 0.0f), 1.0f);

    // vec4 finalColor = diffuse * vec4(1.0f);
    // finalColor += (1.0f - diffuse) * u_AmbientColor;

    // o_FinalColor.rgb = finalColor.rgb;
    // o_FinalColor.a = 1.0f;
}
