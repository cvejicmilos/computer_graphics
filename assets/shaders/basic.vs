#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 sunDir;
uniform vec3 diffuseColor;

out vec3 vNormal;
out vec3 vSunDir;
out vec2 vUV;
out vec3 vDiffuseColor;

void main() {
    vDiffuseColor = diffuseColor;
    vec4 normalWorldSpace = transpose(inverse(model)) * vec4(aNormal, 0.0);
    vNormal = normalize(normalWorldSpace.xyz);
    vSunDir = normalize(sunDir);
    vUV = aUV;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
