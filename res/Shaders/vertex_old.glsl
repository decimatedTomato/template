#version 330 core

precision mediump float;

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec2 a_texCoord0;
// attribute vec4 a_color;

// uniform mat4 u_projTrans;

out vec2 v_texCoords;

void main() {
    // v_color = a_color;
    
    v_texCoords = a_texCoord0;

    // gl_Position = u_projTrans * a_position;
    gl_Position = a_position;
}