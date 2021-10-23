#version 460 core

in vec4 inCoord;
in vec3 inNormal;

out vec4 fragCoord;
out vec3 normal;

uniform mat4 mvp;


void main() {
    gl_Position = mvp * vec4(inCoord.xyz, 1);
    fragCoord = inCoord;
    normal = inNormal;
}