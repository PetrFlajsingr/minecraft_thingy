#version 460 core
layout(points) in;
layout(triangle_strip, max_vertices=24) out;

uniform mat4 mvp;
uniform float sideLen;


void main (void)
{
    gl_Position = mvp * (gl_in[0].gl_Position);
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, 0, 0));
    EmitVertex();
    EndPrimitive();

    gl_Position = mvp * (gl_in[0].gl_Position);
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, 0, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, sideLen, 0));
    EmitVertex();
    EndPrimitive();

    gl_Position = mvp * (gl_in[0].gl_Position);
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, 0, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, sideLen, 0));
    EmitVertex();
    EndPrimitive();

    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, 0, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, sideLen, 0));
    EmitVertex();
    EndPrimitive();

    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, 0, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, sideLen, 0));
    EmitVertex();
    EndPrimitive();

    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(0, sideLen, sideLen, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, 0, 0));
    EmitVertex();
    gl_Position = mvp * (gl_in[0].gl_Position + vec4(sideLen, sideLen, sideLen, 0));
    EmitVertex();
    EndPrimitive();
}