#version 120

attribute vec4 vertex_modelspace;
attribute vec3 normal_modelspace;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;

varying vec4 myvertex;
varying vec3 mynormal;

void main() {
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace ;
	myvertex = vertex_modelspace;
	mynormal = normal_modelspace;
}
