#version 460 core

in vec4 inCoord;
//TODO:
in vec3 inNormal;

out vec4 texCoord;
out vec3 fragPos;
out vec3 normal;

uniform mat4 mvp;
uniform mat4 view;


void main() {
    gl_Position = mvp * vec4(inCoord.xyz, 1);
    fragPos = (view * vec4(inCoord.xyz, 1)).xyz;
    texCoord = inCoord;
    normal = inNormal;
}