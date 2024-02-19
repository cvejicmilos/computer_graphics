#version 330 core
out vec4 FragColor;

in vec3 vNormal; 
in vec3 vSunDir;
in vec2 vUV;
in vec3 vDiffuseColor;

uniform sampler2D diffuseMap;
uniform int hasDiffuseMap;

void main() {

    vec4 diffuse = vec4(vDiffuseColor, 1.0);

    if (hasDiffuseMap > 0) {
        diffuse *= texture(diffuseMap, vec2(vUV.x, 1.0 - vUV.y));
    }

    float minBrightness = 0.2;
    float brightness = max(dot(normalize(vNormal), normalize(vSunDir)), 0.0);
    brightness = max(brightness, minBrightness);
    FragColor = diffuse * brightness;
}
