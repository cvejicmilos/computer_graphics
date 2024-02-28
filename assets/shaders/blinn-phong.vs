#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


out vec3 vNormal;
out vec2 vUV;
out vec3 vFragPos;
out vec3 vViewPos;
out vec3 vVertexPos;
out mat4 vModel;
out vec3 vTangent;
out vec3 vBitangent;

void main() {
    vNormal = mat3(transpose(inverse(model))) * aNormal; 
    //vNormal = aNormal;
    //vNormal = vec3(0);
    vUV = aUV;
    vViewPos = vec3(inverse(view) * vec4(0.0, 0.0, 0.0, 1.0));
    vVertexPos = aPos;
    vModel = model;
    vTangent = aTangent;
    vBitangent = aBitangent;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vFragPos = vec3(model * vec4(aPos, 1.0));


}