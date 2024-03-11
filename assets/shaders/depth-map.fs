
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
};
uniform Material material;

in vec2 vUV;

float getAlpha() {
    float alpha = material.alpha;
    if (material.hasAmbientMap > 0) {
        alpha *= texture(material.ambientMap, vUV).a;
    }

    if (material.hasDiffuseMap > 0) {
        alpha *= texture(material.diffuseMap, vUV).a;
    }
    
    return alpha;
}

void main()
{                
    if (getAlpha() < 0.1) {
        //gl_FragDepth = 1.0;
        discard;
    }
    //gl_FragDepth = 1.0;
}  