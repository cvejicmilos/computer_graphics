#version 330 core

struct Material {
    int hasDiffuseMap;
    sampler2D diffuseMap;
    vec3 diffuseColor;

    sampler2D specularMap;
    int hasSpecularMap;
    vec3 specularColor;
    float specularExponent;
    float specularStrength;

    int hasAmbientMap;
    sampler2D ambientMap;
    vec3 ambientColor;

    int hasNormalMap;
    sampler2D normalMap;

    float alpha;
    float reflectiveness;

    bool shouldCastShadow;
};
uniform Material material;

in vec2 gUV;
in vec4 gFragPos;

float getAlpha() {
    float alpha = material.alpha;
    if (material.hasAmbientMap > 0) {
        alpha *= texture(material.ambientMap, gUV).a;
    }

    if (material.hasDiffuseMap > 0) {
        alpha *= texture(material.diffuseMap, gUV).a;
    }
    
    return alpha;
}

uniform vec3 lightPos;
uniform float farPlane;

void main()
{                
    if (getAlpha() < 0.1) {
        discard;
    } else {
        float lightDistance = length(gFragPos.xyz - lightPos);
        lightDistance = lightDistance / farPlane;
        gl_FragDepth = lightDistance;
    }

}  