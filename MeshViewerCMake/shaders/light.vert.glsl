#version 410 core

in vec4 vertex_modelspace;
in vec3 normal_modelspace;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;

out vec4 myvertex;
out vec3 mynormal;

void main() {
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace ;
	myvertex = vertex_modelspace;
	mynormal = normal_modelspace;
}
