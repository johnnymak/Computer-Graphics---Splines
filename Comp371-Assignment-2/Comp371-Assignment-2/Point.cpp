
#include "Point.h"

using namespace std;

Point::Point() {
	this->coordX = 0;
	this->coordY = 0;
	this->coordZ = 0;
}

Point::Point(GLfloat x, GLfloat y, GLfloat z) {
	this->coordX = x;
	this->coordY = y;
	this->coordZ = z;
}

Point::~Point() {}

GLfloat Point::getCoordX() {
	return this->coordX;
}

GLfloat Point::getCoordY() {
	return this->coordY;
}

GLfloat Point::getCoordZ() {
	return this->coordZ;
}