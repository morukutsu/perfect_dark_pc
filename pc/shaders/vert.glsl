#version 150

in vec4 i_position;
in vec4 i_color;
in vec2 i_uv;

out vec4 v_color;
out vec2 v_uv;

uniform mat4 u_projection_matrix;
uniform mat4 u_modelview_matrix;

void main() {
    v_color = i_color;
    v_uv = i_uv;
    gl_Position = u_projection_matrix * u_modelview_matrix * vec4(i_position);
}
