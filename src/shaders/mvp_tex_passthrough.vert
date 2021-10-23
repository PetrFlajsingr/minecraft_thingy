#version 460 core

in vec4 inCoord;

out vec4 texCoord;

uniform mat4 mvp;


void main() {
    gl_Position = mvp * vec4(inCoord.xyz, 1);
    texCoord = inCoord;
}