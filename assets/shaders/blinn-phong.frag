#version 450 core
out vec4 FragColor;

in vec3 vNormal; 
in vec2 vUV;
in vec3 vFragPos;
in vec3 vViewPos;
in vec3 vVertexPos;
in mat4 vModel;
in vec3 vTangent;
in vec3 vBitangent;

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
struct DepthMapInfo {
    bool shouldCast;
    sampler2D shadowMap;
    mat4 proj;
    mat4 view;
};

struct DirecitonalLight {
    float intensity;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    DepthMapInfo depthMapInfo;
};

struct SpotLight {
    float intensity;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    vec3 position;
    float cutOff;
    float outerCutOff;
    DepthMapInfo depthMapInfo;
};

struct PointLight {
    vec3 diffuse;
    float intensity;
    vec3 specular;
    float innerRadius;
    vec3 position;
    float outerRadius;
    int shouldCast;
    samplerCube shadowMap;
};

uniform Material material;
uniform vec3 ambientColor;
uniform samplerCube skybox;
uniform vec3 skyboxAmbient;
uniform float shadowFarPlane;
uniform int testInt;

// the ##CONSTANT's are replaced by a value when
// shader is parsed in the program.
uniform PointLight pointLights[##MAX_NUM_POINTLIGHTS];
uniform int numPointLights;
uniform DirecitonalLight dirLights[##MAX_NUM_DIRLIGHTS];
uniform int numDirLights;
uniform SpotLight spotLights[##MAX_NUM_SPOTLIGHTS];
uniform int numSpotLights;

vec4 getPosInLightSpace(mat4 lightProj, mat4 lightView) {
    return lightProj * lightView * vec4(vFragPos, 1.0);
}

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
vec3 getNormal() {
    
    if (material.hasNormalMap > 0) {
        vec3 normSample = texture(material.normalMap, vUV).rgb;
        vec3 norm = normSample * 2.0 - 1.0; // Convert from [0, 1] to [-1, 1]

        // Construct the TBN matrix
        vec3 T = normalize(vTangent);
        vec3 B = normalize(vBitangent);
        vec3 N = normalize(vNormal);
        mat3 TBN = mat3(T, B, N);

        // Transform the normal from tangent space to world space
        return normalize(mat3(transpose(inverse(vModel))) * (TBN * norm));
    } else {
        return normalize(vNormal);
    }
}

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

vec3 getSkyboxRaw() {
    vec3 viewDir = normalize(vViewPos - vFragPos);
    vec3 normal = normalize(vNormal);
    viewDir.y *= -1;
    vec3 reflectDir = reflect(viewDir, normal);
    reflectDir.z *= -1;
    vec3 R = reflect(reflectDir, normal);
    return texture(skybox, R).rgb * skyboxAmbient;
}

vec3 getSkyboxImpact() {
    vec3 viewDir = normalize(vViewPos - vFragPos);
    vec3 normal = getNormal();
    viewDir.y *= -1;
    vec3 reflectDir = reflect(viewDir, normal);
    reflectDir.z *= -1;
    vec3 R = reflect(reflectDir, normal);
    return texture(skybox, R).rgb * skyboxAmbient;
}

float calcDiff(vec3 lightDir) {
    return max(dot(getNormal(), lightDir), 0.0);
}
float calcSpec(vec3 lightDir) {

    vec3 viewDir = normalize(vViewPos - vFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(getNormal(), halfwayDir), 0.0), material.specularExponent);

    return spec;
}

float compute3DShadow(int index) {
    if (pointLights[index].shouldCast == 0) return 0.0;
    vec3 fragToLight = vFragPos - pointLights[index].position;
    fragToLight.x *= -1.0;
    float currentDepth = length(fragToLight);
    float shadow  = 0.0;
    float bias    = 0.2; 
    float samples = 4.0;
    float offset  = 0.1;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            for(float z = -offset; z < offset; z += offset / (samples * 0.5))
            {
                float closestDepth = texture(pointLights[index].shadowMap, fragToLight + vec3(x, y, z)).r; ;
                closestDepth *= shadowFarPlane;   // undo mapping [0;1]
                if(currentDepth - bias > closestDepth)
                    shadow += 1.0;
            }
        }
    }
    shadow /= (samples * samples * samples);
    
    return clamp(shadow, 0.0, 1.0);
}

float computeDirShadow(DepthMapInfo depthMapInfo) {
    if (!depthMapInfo.shouldCast) return 0.0;
    vec4 fragPosLightSpace = getPosInLightSpace(depthMapInfo.proj, depthMapInfo.view);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.x > 1.0 || projCoords.x < 0.0) return 0.0;
    if (projCoords.y > 1.0 || projCoords.y < 0.0) return 0.0;
    if (projCoords.z > 1.0 || projCoords.z < 0.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = 0.0015;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(depthMapInfo.shadowMap, 0);
    const int halfkernelWidth = 2;
    for(int x = -halfkernelWidth; x <= halfkernelWidth; ++x)
    {
        for(int y = -halfkernelWidth; y <= halfkernelWidth; ++y)
        {
            float pcfDepth = texture(depthMapInfo.shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= ((halfkernelWidth*2+1)*(halfkernelWidth*2+1));
    
    return clamp(shadow, 0.0, 1.0);
}

vec3 applySkybox(vec3 contrib) {
    if (material.reflectiveness <= 0.0) return contrib;
    vec3 skyboxContrib = getSkyboxImpact();

    vec3 spec = getMaterialSpecular();

    return mix(contrib, (skyboxContrib * spec), material.reflectiveness);
}

vec3 getDirLightContribution(DirecitonalLight light) {
    vec3 lightDir = normalize(-light.direction);

    float diff = calcDiff(lightDir);
    vec3 diffuse = diff  * light.diffuse * getMaterialDiffuse();

    float spec = calcSpec(lightDir);
    
    vec3 specular = material.specularStrength * spec * light.specular * getMaterialSpecular();

    float shadow = computeDirShadow(light.depthMapInfo);
    vec3 contrib = (1.0 - shadow) * applySkybox(diffuse + specular)  * light.intensity;

    return contrib;
}

float computeAttenuation(float distance, float innerRadius, float outerRadius) {
    float attenuation = 1.0;

    if (distance > innerRadius) {
        float theta = clamp((distance - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);
        attenuation = 1.0 - theta * theta * (3.0 - 2.0 * theta);
    }

    return clamp(attenuation, 0.0, 1.0); // Ensure attenuation is within valid range
}

vec3 getPointLightContribution(int index) {
    vec3 lightDir = normalize(pointLights[index].position - vFragPos);  

    float diff = calcDiff(lightDir);
    vec3 diffuse = diff * pointLights[index].diffuse * getMaterialDiffuse();

    float spec = calcSpec(lightDir);
    vec3 specular = material.specularStrength * spec * pointLights[index].specular * getMaterialSpecular();

    float distance = length(pointLights[index].position - vFragPos);
    float attenuation = computeAttenuation(distance, pointLights[index].innerRadius, pointLights[index].outerRadius);

    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = compute3DShadow(index);
    
    vec3 contrib = (1.0 - shadow) * applySkybox(diffuse + specular) * pointLights[index].intensity;
    return contrib;
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

        float shadow = computeDirShadow(light.depthMapInfo);
        vec3 contrib = (1.0 - shadow) * applySkybox(diffuse + specular) * light.intensity;

        return contrib;
    } else {
        return vec3(0.0, 0.0, 0.0);
    }
}

void main() {
    if (getAlpha() < 0.1) discard;
    

    vec3 lighting = vec3(0.0);
    

    for (int i = 0; i < numPointLights; i++) {
        lighting += getPointLightContribution(i);
    }
    for (int i = 0; i < numDirLights; i++) {
        lighting += getDirLightContribution(dirLights[i]);
    }
    for (int i = 0; i < numSpotLights; i++) {
        lighting += getSpotLightContribution(spotLights[i]);
    }

    vec3 ambient =  ambientColor * getMaterialAmbient();
    vec3 result = ambient + lighting;
    FragColor = vec4(result, getAlpha());

    float gamma = 1.1;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
    
}