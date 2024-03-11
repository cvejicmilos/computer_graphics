#version 330 core

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D theTexture;

void main() {
    FragColor = vec4(texture(theTexture, vUV).x, 0.0, 0.0, 1.0);
} 