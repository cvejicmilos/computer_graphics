#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec3 vNormal;
out vec2 vUV;
out vec3 vFragPos;
out vec3 vViewPos;
out vec3 vVertexPos;

void main() {
    vNormal = normalize(mat3(transpose(inverse(model))) * aNormal); 
    vUV = aUV;
    vViewPos = vec3(inverse(view) * vec4(0.0, 0.0, 0.0, 1.0));
    vVertexPos = aPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vFragPos = vec3(model * vec4(aPos, 1.0));


}