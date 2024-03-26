#version 450 core
out vec4 FragColor;

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
};

uniform Material material;
uniform vec3 ambientColor;

in vec2 vUV;

vec3 getMaterialDiffuse() {
    vec3 materialDiffuse = vec3(1);

    if (material.hasDiffuseMap > 0) {
        materialDiffuse *= texture(material.diffuseMap, vUV).rgb;
    } else {
        materialDiffuse *= material.diffuseColor;
    }

    return materialDiffuse;
}
vec3 getMaterialAmbient() {
    vec3 materialAmbient = vec3(1);

    if (material.hasAmbientMap > 0) {
        materialAmbient *= texture(material.ambientMap, vUV).rgb;
    } else {
        materialAmbient *= material.ambientColor;
    }

    return materialAmbient;
}

void main() {
    vec3 result = getMaterialAmbient() * getMaterialDiffuse();

    FragColor = vec4(result, 1.0);

    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}