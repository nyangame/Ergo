#version 450

layout(push_constant) uniform PushConstants {
    mat4 projection;
} pc;

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 frag_color;

void main() {
    gl_Position = pc.projection * vec4(in_pos, 0.0, 1.0);
    frag_color = in_color;
}
