attribute vec3 a_position;
attribute vec3 a_color;
attribute vec2 a_texcoords;
varying vec4 v_color;
uniform mat4 m_rotation;
uniform mat4 m_scale;
uniform mat4 m_proj;
uniform mat4 m_translation;
uniform mat4 m_worldMatrix;
uniform mat4 m_viewMatrix;
void main(void) {
    gl_Position = m_proj * m_viewMatrix * m_worldMatrix * vec4(a_position, 1.0);
    v_color = vec4(0.149,0.141,0.912,1.0);
}

