#version 460 core

in vec4 fragCoord;
in vec3 normal;

out vec4 fragColor;
uniform vec3 lightDir;

uniform sampler2D atlas;

#define LIGHT_COLOR vec3(1, 1, 1)

#define AMBIENT_STRENGTH 0.1
#define DIFFUSE_STRENGTH 0.5


void main() {
    const vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR;

    const float diffuseV = max(dot(normal, lightDir), 0.0);
    const vec3 diffuse = diffuseV * LIGHT_COLOR;

    // no time
    vec3 objectColor;
    if (normal.x != 0) {
        objectColor = texture2D(atlas, vec2((fract(fragCoord.z) + (fragCoord.w - 1.0)) / 4.0, fragCoord.y)).xyz;
    } else if (normal.y != 0) {
        objectColor = texture2D(atlas, vec2((fract(fragCoord.x) + (fragCoord.w - 1.0)) / 4.0, fragCoord.z)).xyz;
    } else {
        objectColor = texture2D(atlas, vec2((fract(fragCoord.x) + (fragCoord.w - 1.0)) / 4.0, fragCoord.y)).xyz;
    }


    fragColor = vec4(objectColor * (ambient + diffuse), 1.0);
}