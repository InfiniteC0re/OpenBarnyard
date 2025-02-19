#version 420
out vec4 FragColor;

in vec2 o_TexCoord;

layout(binding = 0) uniform sampler2D gPosition;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D gColor;
layout(binding = 3) uniform sampler2D gDepth;
layout(binding = 4) uniform sampler2D gShadow;

uniform vec4 u_AmbientColor;
uniform vec4 u_DiffuseColor;
uniform vec3 u_FogColor;
uniform vec3 u_DirectionalLightDir;
uniform mat4 u_LightViewMatrix;

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

vec3 uncharted2_tonemap_partial(vec3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 uncharted2_filmic(vec3 v)
{
    float exposure_bias = u_Exposure;
    vec3 curr = uncharted2_tonemap_partial(v * exposure_bias);

    vec3 W = vec3(11.2f);
    vec3 white_scale = vec3(1.0f) / uncharted2_tonemap_partial(W);
    return curr * white_scale;
}

vec3 directionalLighting(vec3 normal, vec3 texColor)
{
    vec3 lightDir = normalize(-u_DirectionalLightDir);
    
	// diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
	
    // combine results
    vec3 ambient = mix(u_DiffuseColor.rgb, u_AmbientColor.rgb, 0.6) * texColor;
    vec3 diffuse = 0.5 * u_AmbientColor.rgb * diff * texColor;
    return (ambient + diffuse);
}

vec3 pointLight(vec3 fragPos, vec3 normal, vec3 texColor)
{
	vec3 lightPosition = vec3(22.04f, -8.39f, -38.93f);
	vec3 lightDir = normalize(lightPosition - fragPos);

	// diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);

    // attenuation
    float distance    = length(lightPosition - fragPos);
    float attenuation = 1.0 / (0.1f + 0.22f * distance + 0.20f * (distance * distance));
	
    // combine results
    vec3 ambient = vec3(0.278, 0.690, 0.576) * texColor;
    vec3 diffuse = vec3(0.278, 0.690, 0.576) * diff * texColor;

    ambient *= attenuation;
    diffuse *= attenuation;
    return (ambient + diffuse);
}

float CalculateShadow(vec3 normal, vec4 fragPosLightSpace)
{
	vec3 lightDir = normalize(-u_DirectionalLightDir);

	vec3 shadowCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	shadowCoords = shadowCoords * 0.5 + 0.5; 
	
	float shadowMapDepth = texture(gShadow, shadowCoords.xy).r;
	float fragDepth = shadowCoords.z;
	
	float bias = max(u_ShadowBiasMax * (1.0 - dot(normalize(normal), lightDir)), u_ShadowBiasMin);  
	float shadow = fragDepth - bias > shadowMapDepth ? 1.0 : 0.0;  
	return shadow;
}

void main()
{
	vec3 fragPos = texture(gPosition, o_TexCoord).rgb;
	vec3 normal = texture(gNormal, o_TexCoord).rgb;
	vec4 albedo = texture(gColor, o_TexCoord);
	float depth = texture(gDepth, o_TexCoord).r;
	
	// Calculate directional lighting
	FragColor.rgb = directionalLighting(normal, albedo.rgb);
	
	// Apply shadow maps
	vec4 fragPosLightSpace = u_LightViewMatrix * vec4(fragPos, 1.0f);
	float flShadowStrength = CalculateShadow(normal, fragPosLightSpace);

	FragColor.rgb *= (1.0f - flShadowStrength * u_ShadowStrength);
	
	// Apply lighting from point lights
	FragColor.rgb += pointLight(fragPos, normal, FragColor.rgb);
	
	// Tonemap
	FragColor.rgb = uncharted2_filmic(FragColor.rgb);
	FragColor.rgb = pow(FragColor.rgb, vec3(u_Gamma));
	
	// float zNear = 1.0;    // TODO: Replace by the zNear of your perspective projection
    // float zFar  = 280.0; // TODO: Replace by the zFar  of your perspective projection
	//FragColor.rgb = vec3((2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear)));
	
	//FragColor.rgb = mix(FragColor.rgb, u_FogColor, (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear)));
	FragColor.a = 1.0f;
}