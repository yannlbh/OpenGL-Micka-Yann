attribute vec3 a_position;
attribute vec3 a_color;
attribute vec2 a_texcoords;
attribute vec3 a_N;

varying vec3 v_position;
varying vec4 v_color;
varying vec2 v_texcoords_d;
varying vec3 v_N;

uniform mat4 m_rotation;
uniform mat4 m_scale;
uniform mat4 m_proj;
uniform mat4 m_translation;
uniform mat4 m_worldMatrix;
uniform mat4 m_viewMatrix;

void main(void) {
    gl_Position = m_proj * m_viewMatrix * m_worldMatrix * vec4(a_position, 1.0);
    v_N = a_N;
    v_texcoords_d = a_texcoords;
    v_color = vec4(1.0);
    v_position = a_position;
}

