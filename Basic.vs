#version 150

attribute vec3 a_position;
attribute vec3 a_color;
attribute vec2 a_texcoords;
attribute vec3 a_N;

varying vec3 v_position;
varying vec4 v_color;
varying vec2 v_texcoords_d;
varying vec3 v_N;


uniform mat4 m_proj;
uniform mat4 m_worldMatrix;
uniform mat4 m_viewMatrix;

void main(void) {
    gl_Position = m_proj * m_viewMatrix * m_worldMatrix * vec4(a_position, 1.0);
    v_N = (transpose(inverse(m_worldMatrix)) * vec4(a_N, 0.0)).xyz;
    v_texcoords_d = a_texcoords;
    v_color = vec4(1.0);
    v_position = a_position;
}

