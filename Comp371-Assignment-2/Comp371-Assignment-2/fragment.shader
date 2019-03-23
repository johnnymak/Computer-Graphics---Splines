
/*****
*
*	Johnny Mak
*	Comp 371 - Computer Graphics
*	Assignment #2
*
*****/

#version 330 core

in vec3 colorDepth;

out vec4 color;

void main() {
	color = vec4(colorDepth , 1.0f);
}

