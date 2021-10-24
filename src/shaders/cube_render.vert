#version 460 core

in vec3 inCoord;
uniform vec3 hack;

void main() {
    gl_Position = vec4(hack, 1.0);
}
