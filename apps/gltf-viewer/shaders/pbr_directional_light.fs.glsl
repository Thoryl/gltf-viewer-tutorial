#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uLightDirection;
uniform vec3 uLightIntensity;

uniform sampler2D uBaseColorTexture;
uniform vec4 uBaseColorFactor;

uniform float uMetallicFactor;
uniform float uRoughnessFactor;
uniform sampler2D uMetallicRoughnessTexture;

uniform sampler2D uEmissiveTexture;
uniform vec3 uEmissiveFactor;

uniform sampler2D uOcclusionTexture;
uniform float uOcclusionStrength;

out vec3 fColor;

// Constants
const float GAMMA = 2.2;
const float INV_GAMMA = 1. / GAMMA;
const float M_PI = 3.141592653589793;
const float M_1_PI = 1.0 / M_PI;
const vec3 dielectricSpecular = vec3(0.04, 0.04, 0.04);
const vec3 black = vec3(0, 0, 0);

// We need some simple tone mapping functions
// Basic gamma = 2.2 implementation
// Stolen here: https://github.com/KhronosGroup/glTF-Sample-Viewer/blob/master/src/shaders/tonemapping.glsl

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 LINEARtoSRGB(vec3 color)
{
  return pow(color, vec3(INV_GAMMA));
}

// sRGB to linear approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec4 SRGBtoLINEAR(vec4 srgbIn)
{
  return vec4(pow(srgbIn.xyz, vec3(GAMMA)), srgbIn.w);
}

void main()
{
  vec3 N = normalize(vViewSpaceNormal);
  vec3 L = uLightDirection;
  vec3 V = normalize(-vViewSpacePosition);
  vec3 H = normalize(L + V);

  vec4 baseColorFromTexture = SRGBtoLINEAR(texture(uBaseColorTexture, vTexCoords));
  vec4 baseColor = baseColorFromTexture * uBaseColorFactor;
  vec4 metallicRoughnessFromTexture = texture(uMetallicRoughnessTexture, vTexCoords);
  float metallic = uMetallicFactor * metallicRoughnessFromTexture.b;
  float roughness = uRoughnessFactor * metallicRoughnessFromTexture.g;
  vec4 emissiveFromTexture = SRGBtoLINEAR(texture(uEmissiveTexture, vTexCoords));
  vec4 emissive = emissiveFromTexture * vec4(uEmissiveFactor, 1);
  vec4 occlusionFromTexture = texture(uOcclusionTexture, vTexCoords);

  vec3 c_diffuse = mix(baseColor.rgb * (1 - dielectricSpecular.r), black, metallic);
  vec3 F_O = mix(dielectricSpecular, baseColor.rgb, metallic);
  float alpha = roughness * roughness;

  float NdotL = clamp(dot(N, L), 0, 1);
  float NdotV = clamp(dot(N, V), 0, 1);
  float NdotH = clamp(dot(N, H), 0, 1);
  float VdotH = clamp(dot(V, H), 0, 1);

  float baseShlickFactor = (1 - VdotH);
  float shlickFactor = baseShlickFactor * baseShlickFactor;
  shlickFactor *= shlickFactor;
  shlickFactor *= baseShlickFactor;
  vec3 F = F_O + (1 - F_O) * shlickFactor;

  float alphaPowTwo = alpha * alpha;
  float denominatorVis = NdotL * sqrt(pow(NdotV, 2) * (1 - alphaPowTwo) + alphaPowTwo) + NdotV * sqrt((NdotL * NdotL) * (1 - alphaPowTwo) + alphaPowTwo);
  float Vis = 0;
  if(denominatorVis > 0) {
    Vis = 0.5 / denominatorVis;
  }

  float denominatorD = M_PI * (((NdotH * NdotH) * (alphaPowTwo - 1) + 1) * ((NdotH * NdotH) * (alphaPowTwo - 1) + 1));
  float D = 0;
  if(denominatorD > 0) {
    D = alphaPowTwo / denominatorD;
  }

  vec3 diffuse = c_diffuse / M_PI;

  vec3 f_diffuse = (1 - F) * diffuse;
  vec3 f_specular = F * Vis * D;

  fColor = (f_diffuse + f_specular) * uLightIntensity * NdotL + emissive.xyz;
  fColor = mix(fColor, fColor * occlusionFromTexture.r, uOcclusionStrength);
  fColor = LINEARtoSRGB(fColor);
}
