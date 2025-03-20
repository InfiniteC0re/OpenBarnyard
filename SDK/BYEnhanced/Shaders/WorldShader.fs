#version 460
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gColor;
layout (location = 3) out vec4 gInfo;

in vec2 textureCoord;
in vec3 vertexColor;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D tex0;

void main() {
    vec4 texColor = texture(tex0, textureCoord);
    if (texColor.a <= 0.5) discard;

    gPosition = vec4(fragPos, 1.0f);
    gNormal = vec4(normal, 1.0f);
    gColor = texColor;
    gInfo.rgba = vec4(0.0f);
}