#version 460 core

in vec4 texCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 fragColor;
// TODO:
uniform vec3 lightDir;

#define LIGHT_COLOR vec3(1, 1, 1)

#define AMBIENT_STRENGTH 0.1
#define DIFFUSE_STRENGTH 0.5


void main() {
    const vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR;

    const float diffuseV = max(dot(normal, lightDir), 0.0);
    const vec3 diffuse = diffuseV * LIGHT_COLOR;


    const vec3 objectColor = vec3(texCoord.w / 128.0 + 0.5, texCoord.w / 256.0, texCoord.w / 512.0) + fragPos * 0.00001;

    fragColor = vec4(objectColor * (ambient + diffuse), 1.0);
}