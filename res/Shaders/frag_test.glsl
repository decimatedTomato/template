#version 330 core

precision mediump float;

// varying vec4 v_color;

in vec4 gl_FragCoord;
in vec2 v_texCoords;

out vec4 fragColor;

uniform vec2 u_resolution;
// uniform float u_time;
// uniform sampler2D u_texture;

void main() {
    vec2 norm_coord = gl_FragCoord.xy / u_resolution;
    vec3 color = vec3(norm_coord.x, 0.0, norm_coord.y);
    fragColor = vec4(color, 1.0);
}