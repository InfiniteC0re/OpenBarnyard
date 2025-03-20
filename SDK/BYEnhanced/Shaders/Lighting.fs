#version 460
out vec4 FragColor;

in vec2 o_TexCoord;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gColor;
layout(binding = 3) uniform sampler2D gInfo;
layout(binding = 4) uniform sampler2D gDepth;
layout(binding = 5) uniform sampler2D gShadow;

uniform vec4 u_AmbientColor;
uniform vec4 u_DiffuseColor;
uniform vec3 u_SpecularColor;
uniform vec3 u_FogColor;
uniform vec3 u_DirectionalLightDir;
uniform mat4 u_LightViewMatrix;
uniform mat4 u_View;
uniform vec3 u_CamPos;

uniform float u_Time;

uniform float u_ShadowBiasMin;
uniform float u_ShadowBiasMax;
uniform float u_ShadowStrength;

uniform float u_Exposure;
uniform float u_Gamma;

vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 filmic(vec3 x) {
    vec3 X = max(vec3(0.0), x - 0.004);
    vec3 result = (X * (6.2 * X + 0.5)) / (X * (6.2 * X + 1.7) + 0.06);
    return pow(result, vec3(2.2));
}

vec3 uncharted2_tonemap_partial(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2_filmic(vec3 v) {
    float exposure_bias = u_Exposure;
    vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    vec3 W = vec3(11.2);
    vec3 white_scale = vec3(1.0) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

vec3 directionalLighting(vec3 info, vec3 fragPos, vec3 normal, vec3 texColor, float shadowFactor) {
    vec3 viewDir = normalize(u_CamPos - fragPos);
    vec3 lightDir = normalize(-u_DirectionalLightDir);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = max(pow(max(dot(normal, halfwayDir), 0.0), info.r), 0.0f);

    // combine results
    vec3 ambient = mix(u_DiffuseColor.rgb, u_AmbientColor.rgb, 0.6) * texColor;
    vec3 diffuse = 0.5 * u_AmbientColor.rgb * diff * texColor;
    vec3 specular = u_SpecularColor * spec * info.g;
    return (ambient + diffuse + specular);
}

vec3 pointLight(vec3 fragPos, vec3 normal, vec3 texColor) {
    vec3 lightPosition = vec3(22.04, -8.39, -38.93);
    vec3 lightDir = normalize(lightPosition - fragPos);

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // attenuation
    float distance = length(lightPosition - fragPos);
    float attenuation = 1.0 / (0.1 + 0.22 * distance + 0.20 * (distance * distance));

    // combine results
    vec3 ambient = vec3(0.278, 0.690, 0.576) * texColor;
    vec3 diffuse = vec3(0.278, 0.690, 0.576) * diff * texColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    return (ambient + diffuse);
}

vec3 selfLighting(vec3 texColor, float intensity) {
    return (texColor * intensity);
}

float CalculateShadow(vec3 normal, vec4 fragPosLightSpace) {
    vec3 lightDir = normalize(-u_DirectionalLightDir);
    float bias = max(u_ShadowBiasMax * (1.0 - dot(normal, lightDir)), u_ShadowBiasMin);

    vec3 shadowCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float fragDepth = shadowCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(gShadow, 0);
    for(int x = -2; x <= 2; ++x) {
        for(int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(gShadow, shadowCoords.xy + vec2(x, y) * texelSize).r;
            shadow += fragDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 25.0;

    return shadow;
}

void main() {
    vec3 fragPos = (u_View * vec4(texture(gPosition, o_TexCoord).rgb, 1.0)).xyz;
    vec3 normal  = texture(gNormal, o_TexCoord).rgb;
    vec4 albedo  = texture(gColor, o_TexCoord);
    vec3 info    = texture(gInfo, o_TexCoord).rgb;

    // Apply shadow maps
    vec4 fragPosLightSpace = u_LightViewMatrix * vec4(fragPos, 1.0);
    float flShadowFactor = CalculateShadow(normal, fragPosLightSpace);

    // Calculate directional lighting
    FragColor.rgb = directionalLighting(info, fragPos, normal, albedo.rgb, 1.0 - flShadowFactor * 0.5f);

    FragColor.rgb *= (1.0 - flShadowFactor * u_ShadowStrength);

    // Apply lighting from point lights
    FragColor.rgb += pointLight(fragPos, normal, FragColor.rgb);

    // Tonemap
    FragColor.rgb = uncharted2_filmic(FragColor.rgb);
    FragColor.rgb = pow(FragColor.rgb, vec3(u_Gamma));

    FragColor.a = 1.0;

    // float zNear = 1.0;    // TODO: Replace by the zNear of your perspective projection
    // float zFar  = 280.0; // TODO: Replace by the zFar  of your perspective projection
    //FragColor.rgb = vec3((2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear)));

    // FragColor.rgb = mix(FragColor.rgb, u_FogColor, (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear)));
    // FragColor.a = 1.0;

    // float Pixels = 4096.0 + sin(u_Time / 50) * 4096;
    // float dx = 15.0 * (1.0 / Pixels);
    // float dy = 10.0 * (1.0 / Pixels);
    // vec2 Coord = vec2(dx * floor(o_TexCoord.x / dx), dy * floor(o_TexCoord.y / dy));
    // FragColor = texture(gColor, Coord);
    // FragColor.r *= 2.0f * max(sin(fragPos.x + u_Time), 0.5f);
    // FragColor.g *= 2.0f * max(sin(fragPos.y + u_Time + 48), 0.5f);
    // FragColor.b *= 2.0f * max(sin(fragPos.z + u_Time + 152), 0.5f);

    // FragColor.rgb = uncharted2_filmic(FragColor.rgb);
    // FragColor.rgb = pow(FragColor.rgb, vec3(u_Gamma));
}