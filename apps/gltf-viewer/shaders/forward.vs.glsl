#version 330

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec4 aTangent;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec2 vTexCoords;
out vec3 vViewSpaceTangent;
out vec3 vViewSpaceBitangent;
flat out int vHasTangent;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

uniform int uHasNormalMap;// 0 =  No normal map ; 1 = Normal map but no tangent

void main()
{
    vViewSpacePosition = vec3(uModelViewMatrix * vec4(aPosition, 1.f));
    vViewSpaceNormal = normalize(vec3(uNormalMatrix * vec4(aNormal, 0.f)));
    if (uHasNormalMap == 1) {
        vViewSpaceTangent = normalize(vec3(uModelMatrix * aTangent));
        vViewSpaceBitangent = cross(vViewSpaceNormal, vViewSpaceTangent) * aTangent.w;
    } else {
        vViewSpaceTangent = vec3(0., 0., 0.);
        vViewSpaceBitangent = vec3(0., 0., 0.);
    }
    if(aTangent.x == 0. && aTangent.y == 0. && aTangent.z == 0.) {
        vHasTangent = 0;
    } else {
        vHasTangent = 1;
    }

    vTexCoords = aTexCoords;
    gl_Position =  uModelViewProjMatrix * vec4(aPosition, 1);
}