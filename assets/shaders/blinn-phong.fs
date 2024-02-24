#version 330 core
out vec4 FragColor;

in vec3 vNormal; 
in vec2 vUV;
in vec3 vFragPos;
in vec3 vViewPos;
in vec3 vVertexPos;

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

struct DirecitonalLight {
    float intensity;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

struct SpotLight {
    float intensity;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    vec3 position;
    float cutOff;
    float outerCutOff;
};

struct PointLight {
    float intensity;
    vec3 diffuse;
    vec3 specular;
    vec3 position;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform vec3 ambientColor;

// the ##CONSTANT's are replaced by a value when
// shader is parsed in the program.
uniform PointLight pointLights[##MAX_NUM_POINTLIGHTS];
uniform int numPointLights;
uniform DirecitonalLight dirLights[##MAX_NUM_DIRLIGHTS];
uniform int numDirLights;
uniform SpotLight spotLights[##MAX_NUM_SPOTLIGHTS];
uniform int numSpotLights;

vec3 getMaterialDiffuse() {
    vec3 materialDiffuse = vec3(1);

    if (material.hasDiffuseMap > 0) {
        materialDiffuse *= texture(material.diffuseMap, vUV).rgb;
    } else {
        materialDiffuse *= material.diffuseColor;
    }

    return materialDiffuse;
}
vec3 getMaterialSpecular() {
    vec3 materialSpecular = vec3(1);

    if (material.hasSpecularMap > 0) {
        materialSpecular *= texture(material.specularMap, vUV).rgb;
    } else {
        materialSpecular *= material.specularColor;
    }

    return materialSpecular;
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

float calcDiff(vec3 lightDir) {
    return max(dot(vNormal, lightDir), 0.0);
}
float calcSpec(vec3 lightDir) {

    /*
    vec3 viewDir = normalize(vViewPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, vNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.specularExponent);
    */

    vec3 viewDir = normalize(vViewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(vNormal, halfwayDir), 0.0), material.specularExponent);

    return spec;
}

vec3 getDirLightContribution(DirecitonalLight light) {
    vec3 lightDir = normalize(-light.direction);

    float diff = calcDiff(lightDir);
    vec3 diffuse = diff  * light.diffuse * getMaterialDiffuse();

    float spec = calcSpec(lightDir);
    
    vec3 specular = material.specularStrength * spec * light.specular * getMaterialSpecular();

    return (diffuse + specular) * light.intensity;
}
vec3 getPointLightContribution(PointLight light) {
    vec3 lightDir = normalize(light.position - vFragPos);  

    float diff = calcDiff(lightDir);
    vec3 diffuse = diff  * light.diffuse * getMaterialDiffuse();

    float spec = calcSpec(lightDir);
    vec3 specular = material.specularStrength * spec * light.specular * getMaterialSpecular();

    float distance    = length(light.position - vFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    return (diffuse + specular) * light.intensity;
}

vec3 getSpotLightContribution(SpotLight light) {
    vec3 lightDir = normalize(light.position - vFragPos);

    float theta = dot(lightDir, normalize(-light.direction));

    if (theta > light.outerCutOff) {

        float diff = calcDiff(lightDir);
        vec3 diffuse = diff  * light.diffuse * getMaterialDiffuse();

        float spec = calcSpec(lightDir);
        vec3 specular = material.specularStrength * spec * light.specular * getMaterialSpecular();

        float epsilon   = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); 
        diffuse *= intensity;
        specular *= intensity;

        return (diffuse + specular) * light.intensity;
    } else {
        return vec3(0);
    }
}

void main() {
    vec3 ambient = ambientColor * getMaterialAmbient();

    vec3 result = ambient;

    for (int i = 0; i < numPointLights; i++) {
        result += getPointLightContribution(pointLights[i]);
    }
    for (int i = 0; i < numDirLights; i++) {
        result += getDirLightContribution(dirLights[i]);
    }
    for (int i = 0; i < numSpotLights; i++) {
        result += getSpotLightContribution(spotLights[i]);
    }

    FragColor = vec4(result, 1.0);
}