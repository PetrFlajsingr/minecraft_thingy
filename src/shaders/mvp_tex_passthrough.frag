#version 460 core

in vec4 texCoord;

out vec4 fragColor;


void main() {
    fragColor = vec4(texCoord.w / 128.0 + 1.0, texCoord.w / 256.0, texCoord.w / 512.0, 1.0);
}