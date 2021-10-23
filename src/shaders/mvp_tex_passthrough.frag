#version 460 core

in vec4 fragCoord;
in vec3 normal;

out vec4 fragColor;
uniform vec3 lightDir;

#define LIGHT_COLOR vec3(1, 1, 1)

#define AMBIENT_STRENGTH 0.1
#define DIFFUSE_STRENGTH 0.5


void main() {
    const vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR;

    const float diffuseV = max(dot(normal, lightDir), 0.0);
    const vec3 diffuse = diffuseV * LIGHT_COLOR;



    const vec3 objectColor = vec3(fragCoord.w / 128.0 + 0.5, fragCoord.w / 256.0, fragCoord.w / 512.0);

    fragColor = vec4(objectColor * (ambient + diffuse), 1.0);
}