#version 330

precision mediump float;

uniform sampler2D u_texture;

in vec2 uv;
out vec4 out_color;

void main(void) {
    out_color = texture(u_texture, uv);
}