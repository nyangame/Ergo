#version 450

layout(set = 0, binding = 0) uniform sampler2D tex_sampler;

layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec4 frag_color;

layout(location = 0) out vec4 out_color;

void main() {
    vec4 tex_color = texture(tex_sampler, frag_uv);
    out_color = tex_color * frag_color;
}
