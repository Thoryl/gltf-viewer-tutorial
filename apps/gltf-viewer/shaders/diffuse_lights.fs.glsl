#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uLightDirection;
uniform vec3 uLightIntensity;

out vec3 fColor;

void main(){
    vec3 brdf = vec3(1./3.14);
    vec3 viewSpaceNormal = normalize(vViewSpaceNormal);
    fColor = brdf * uLightIntensity * dot(viewSpaceNormal, uLightDirection);
}