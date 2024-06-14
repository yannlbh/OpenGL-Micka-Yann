attribute vec2 a_position;
attribute vec3 a_color;
varying vec4 v_color;
uniform mat4 m_projectionMatrix;
uniform mat4 m_worldMatrix;
uniform mat4 m_viewMatrix;
void main(void) {
    gl_Position = m_projectionMatrix * m_worldMatrix * vec4(a_position, 0.0, 1.0);
    v_color = vec4(a_color, 1.0);
}