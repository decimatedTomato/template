#version 330 core

precision mediump float;

// varying vec4 v_color;

in vec4 gl_FragCoord;
in vec2 v_texCoords;

out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;
uniform sampler2D u_texture;

void main() {
    vec4 color = texture2D(u_texture, v_texCoords); // * v_color;
    fragColor = color;
    // fragColor = vec4(color.rgb, 1.0);
    // if (gl_FragCoord.x < 20 || gl_FragCoord.y < 20 || gl_FragCoord.x > 620 || gl_FragCoord.y > 460) fragColor = vec4(1.0, 0.1, 0.5, 1.0);
}