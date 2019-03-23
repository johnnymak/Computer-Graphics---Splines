
/*****
*
*	Johnny Mak
*	Comp 371 - Computer Graphics
*	Assignment #2
*
*****/

#ifndef POINT_H
#define POINT_H

#include <string>
#include <GL/glew.h>

using namespace std;

class Point {

	private: 
		GLfloat coordX; 
		GLfloat coordY;
		GLfloat coordZ;

	public:
		Point();
		Point(GLfloat x, GLfloat y, GLfloat z);
		~Point();

		GLfloat getCoordX();
		GLfloat getCoordY();
		GLfloat getCoordZ();


};

#endif