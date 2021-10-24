#version 460 core

in vec4 inCoord;
in vec3 inNormal;

out vec4 fragCoord;
out vec3 normal;

uniform mat4 mvp;
uniform vec3 chunkPosition;


void main() {
    vec4 coord = vec4(inCoord.xyz + chunkPosition, inCoord.w);
    gl_Position = mvp * vec4(coord.xyz, 1);
    fragCoord = coord;
    normal = inNormal;
}