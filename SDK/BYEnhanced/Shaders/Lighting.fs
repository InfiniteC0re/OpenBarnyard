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
uniform mat4 u_Projection;
// uniform vec3 u_CamPos; // unused since everything is in world space now

uniform float u_Time;

uniform float u_ShadowBiasMin;
uniform float u_ShadowBiasMax;
uniform float u_ShadowStrength;

uniform float u_Exposure;
uniform float u_Gamma;

const vec3 SKYDOME_COLOR_MIDDLE = vec3(0.761, 0.902, 0.925);
const vec3 SKYDOME_COLOR_TOP = vec3(0.424, 0.549, 0.875);

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
    vec3 curr = uncharted2_tonemap_partial(v * u_Exposure);

    vec3 W = vec3(11.2);
    vec3 white_scale = vec3(1.0) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

vec3 directionalLighting(vec3 info, vec3 fragPos, vec3 normal, vec3 texColor, float shadowFactor) {
    vec3 viewDir = normalize(-fragPos);
    vec3 lightDir = -u_DirectionalLightDir;

    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = max(pow(max(dot(normal, halfwayDir), 0.0), info.r), 0.0f);

    // combine results
    vec3 sky = (min(max(0.0, dot(normal, vec3(0, -1, 0))), 1.0) * texColor) * SKYDOME_COLOR_MIDDLE * 0.4;
    vec3 ambient = mix(u_DiffuseColor.rgb, u_AmbientColor.rgb, 0.6) * texColor;
    vec3 diffuse = 0.5 * u_AmbientColor.rgb * diff * texColor;
    vec3 specular = u_SpecularColor * spec * info.g * shadowFactor;

    return (ambient + sky + diffuse + specular);
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
    vec3 lightDir = -u_DirectionalLightDir;
    float bias = max(u_ShadowBiasMax * (1.0 - dot(normal, lightDir)), u_ShadowBiasMin);

    vec3 shadowCoords = (fragPosLightSpace.xyz / fragPosLightSpace.w) * 0.5 + 0.5;
    float fragDepth = shadowCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(gShadow, 0);
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(gShadow, shadowCoords.xy + vec2(x, y) * texelSize).r;
            shadow += fragDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }

    shadow /= 25.0;

    return shadow;
}

#define PI 3.14159265359

float sdSphere(vec3 p, float radius) {
    // vec3 q = abs(p - vec3(0, -50, 0)) - vec3(80, 10, 80);
    // return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
    return dot(p, vec3(0, 1, 0)) + radius;
    // return length(p) - radius;
}

vec2 hash(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise(in vec2 p)
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

    vec2 i = floor(p + (p.x + p.y) * K1);
    vec2 a = p - i + (i.x + i.y) * K2;
    float m = step(a.y, a.x);
    vec2 o = vec2(m, 1.0 - m);
    vec2 b = a - o + K2;
    vec2 c = a - 1.0 + 2.0 * K2;
    vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    vec3 n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
    return dot(n, vec3(70.0));
}

const mat2 m2 = mat2(1.6, 1.2, -1.2, 1.6);

float fbm4(vec2 p) {
    float amp = 0.5;
    float h = 0.0;
    for (int i = 0; i < 4; i++) {
        float n = noise(p);
        h += amp * n;
        amp *= 0.5;
        p = m2 * p;
    }

    return 0.5 + 0.5 * h;
}

/**
This tab contains all the necessary noise functions required to model a cloud shape.
*/

// Hash by David_Hoskins
#define UI0 1597334673U
#define UI1 3812015801U
#define UI2 uvec2(UI0, UI1)
#define UI3 uvec3(UI0, UI1, 2798796415U)
#define UIF (1.0 / float(0xffffffffU))

vec3 hash33(vec3 p)
{
    uvec3 q = uvec3(ivec3(p)) * UI3;
    q = (q.x ^ q.y ^ q.z) * UI3;
    return -1. + 2. * vec3(q) * UIF;
}

float remap(float x, float a, float b, float c, float d)
{
    return (((x - a) / (b - a)) * (d - c)) + c;
}

// Gradient noise by iq (modified to be tileable)
float gradientNoise(vec3 x, float freq)
{
    // grid
    vec3 p = floor(x);
    vec3 w = fract(x);

    // quintic interpolant
    vec3 u = w * w * w * (w * (w * 6. - 15.) + 10.);

    // gradients
    vec3 ga = hash33(mod(p + vec3(0., 0., 0.), freq));
    vec3 gb = hash33(mod(p + vec3(1., 0., 0.), freq));
    vec3 gc = hash33(mod(p + vec3(0., 1., 0.), freq));
    vec3 gd = hash33(mod(p + vec3(1., 1., 0.), freq));
    vec3 ge = hash33(mod(p + vec3(0., 0., 1.), freq));
    vec3 gf = hash33(mod(p + vec3(1., 0., 1.), freq));
    vec3 gg = hash33(mod(p + vec3(0., 1., 1.), freq));
    vec3 gh = hash33(mod(p + vec3(1., 1., 1.), freq));

    // projections
    float va = dot(ga, w - vec3(0., 0., 0.));
    float vb = dot(gb, w - vec3(1., 0., 0.));
    float vc = dot(gc, w - vec3(0., 1., 0.));
    float vd = dot(gd, w - vec3(1., 1., 0.));
    float ve = dot(ge, w - vec3(0., 0., 1.));
    float vf = dot(gf, w - vec3(1., 0., 1.));
    float vg = dot(gg, w - vec3(0., 1., 1.));
    float vh = dot(gh, w - vec3(1., 1., 1.));

    // interpolation
    return va +
        u.x * (vb - va) +
        u.y * (vc - va) +
        u.z * (ve - va) +
        u.x * u.y * (va - vb - vc + vd) +
        u.y * u.z * (va - vc - ve + vg) +
        u.z * u.x * (va - vb - ve + vf) +
        u.x * u.y * u.z * (-va + vb + vc - vd + ve - vf - vg + vh);
}

// Fbm for Perlin noise based on iq's blog
float perlinfbm(vec3 p, float freq, int octaves)
{
    float G = exp2(-.85);
    float amp = 1.;
    float noise = 0.;
    for (int i = 0; i < octaves; ++i)
    {
        noise += amp * gradientNoise(p * freq + 0.1 * u_Time * (i * 0.5), freq);
        freq *= 2.;
        amp *= G;
    }

    return noise;
}

float scene(vec3 p) {
    float pfbm = mix(1., perlinfbm((p / 5000.0f) + (u_Time * 0.005 * normalize(vec3(1.0, 0.5, 0.0))), 4., 6), .5);
    pfbm = abs(pfbm * 2. - 1.);
    pfbm = remap(pfbm, 0., 1., 0.8f, 1.);

    return remap(pfbm, .85, 1., 0., 1.);
}

#define MAX_STEPS 48
const float MARCH_SIZE = 25.0;
const float SKYBOX_HEIGHT = 1200.0f;

vec4 raymarch(vec3 rayOrigin, vec3 rayDirection) {
    float depth = 0.0;
    vec3 p = rayOrigin;

    vec4 res = vec4(0.0);

    for (int i = 0; i < MAX_STEPS; i++) {

        // We only draw the density if it's greater than 0
        float dist = -sdSphere(p, 1000.0f);

        if (dist > 0) {
            float density = max(scene(p), 0.0f);

            vec4 color = vec4(mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.5, 0.5), density), density);
            color.rgb *= color.a;
            res += color * (1.0 - res.a);

            p += MARCH_SIZE * rayDirection;
            depth += MARCH_SIZE;
        } else {
            p += max(-dist, MARCH_SIZE) * rayDirection;
        }
    }

    return res;
}

void main() {
    vec3 fragPos = texture(gPosition, o_TexCoord).rgb;
    vec3 fragPosWorld = (u_View * vec4(fragPos, 1.0f)).xyz;
    vec3 normal = texture(gNormal, o_TexCoord).rgb;
    vec4 albedo = texture(gColor, o_TexCoord);
    vec3 info = texture(gInfo, o_TexCoord).rgb;
    vec3 depth = texture(gDepth, o_TexCoord).rgb;

    // Apply shadow maps
    vec4 fragPosLightSpace = u_LightViewMatrix * vec4(fragPosWorld, 1.0);
    float flShadowFactor = CalculateShadow(normal, fragPosLightSpace);

    // Calculate directional lighting
    FragColor.rgb = directionalLighting(info, fragPos, normal, albedo.rgb, 1.0 - flShadowFactor * 0.5f);

    FragColor.rgb *= (1.0 - flShadowFactor * u_ShadowStrength);

    // Apply lighting from point lights
    FragColor.rgb += pointLight(fragPosWorld, normal, FragColor.rgb);

    float zNear = 1.0; // TODO: Replace by the zNear of your perspective projection
    float zFar = 280.0; // TODO: Replace by the zFar  of your perspective projection

    if (depth.x == 1.0f) {
        // Volumetrics
        vec4 near = vec4(
                (o_TexCoord.x - 0.5) * 2.0f,
                (o_TexCoord.y - 0.5) * 2.0f,
                -1.0f,
                1.0f
            );

        vec4 far = vec4(
                (o_TexCoord.x - 0.5) * 2.0f,
                (o_TexCoord.y - 0.5) * 2.0f,
                1.0f,
                1.0f
            );

        // TODO: calculate it on CPU for better performance 
        mat4 ind = inverse(u_Projection * inverse(u_View));
        vec4 nearPosition = ind * near;
        vec4 farPosition = ind * far;
        nearPosition.xyz /= nearPosition.w;
        farPosition.xyz /= farPosition.w;

        // FragColor.rgb = mix(vec3(0.0f, 0.72549f, 0.949f), vec3(0.003f, 0.611f, 0.878f), near.y);
        FragColor.rgb = mix(SKYDOME_COLOR_MIDDLE, SKYDOME_COLOR_TOP, min(1.0f, abs(2.5 * dot(normalize(farPosition.xyz), vec3(0, 1, 0)))));

        vec3 rayDirection = normalize(farPosition.xyz - nearPosition.xyz);

        vec4 res = raymarch(nearPosition.xyz, rayDirection);
        FragColor.rgb = FragColor.rgb * (1.0 - res.a) + res.rgb;
    } else {
        FragColor.rgb = mix(FragColor.rgb, u_FogColor, max(((2.0 * zNear) / (zFar + zNear - depth.x * (zFar - zNear))) - 0.35, 0.0));

        // Tonemap
        FragColor.rgb *= u_Exposure;
        FragColor.rgb = pow(FragColor.rgb, vec3(0.823));
        FragColor.rgb = uncharted2_filmic(FragColor.rgb);
    }

    // Convert colors to sRGB space
    // FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / 2.2));

    FragColor.a = 1.0;

    //FragColor.rgb = vec3((2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear)));

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
