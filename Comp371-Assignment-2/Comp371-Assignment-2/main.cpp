
/*****
*
*	Johnny Mak
*	Comp 371 - Computer Graphics
*	Assignment #2
*
*****/


#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include "Shader.h"
#include "Point.h"

// GLEW 
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;


// ========== Function Prototypes ========== //

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow * window, int key, int action, int mods);
GLfloat getXCoordinateValues(double pointX);
GLfloat getYCoordinateValues(double pointY);
void addVertex(GLfloat xCoord, GLfloat yCoord, GLfloat zCoord);
void updateIndices(int vertexCount);
void printVertices();
void printIndices();
Point calculateSpline(double u, Point firstPoint, Point secondPoint, Point thirdPoint, Point forthPoint); 
double distance(Point first, Point second);
void subdivide(double u0, double u1, double maxLength, Point firstPoint, Point secondPoint, Point thirdPoint, Point forthPoint);
void convertVertices();
void convertVerticesTemp();


// ========== Constants and Values ========== //

// Vertex and Fragment Shader locations
const GLchar* VERTEX_SHADER_PATH   = "./vertex.shader";
const GLchar* FRAGMENT_SHADER_PATH = "./fragment.shader";

// Initial Screen dimensions (800x800) 
const GLuint WIDTH  = 800,
			 HEIGHT = 800;

// Vectors used to contain our points
vector<GLfloat> vertices;
vector<GLuint> indices;

vector<Point> points;
vector<Point> printPoints;

// Global Variables 
double cursorX,		// Windows Cursor Positions
	   cursorY;		

int width, height;	// Windows Size 

int centerX,		// Locate the center
	centerY;

bool isVertexEmpty = true;
bool isTranslational = false;
bool secondPass = false;
bool isDrawn = false;
bool isCompleted = false;

GLuint VBO, VAO, EBO;

// Basis Matrix (Used for Spline calculation) 
GLfloat basisValues[16] = {-0.5, 1, -0.5, 0, 1.5, -2.5, 0, 1, -1.5, 2, 0.5, 0, 0.5, -0.5, 0, 0};
glm::mat4x4 basisMatrix = glm::make_mat4x4(basisValues);

// Default Drawing mode
GLenum drawMode = GL_POINTS; 



// ========== Main Method ========== //

int main() {


	ofstream outfile;

	char input; 
	cout << "Press 'T' for Translation or 'R' for Rotational transformation" << endl;
	cin >> input; 

	// Input: Translational 
	if (toupper(input) == 'T') {
		isTranslational = true;
		outfile.open("txt/A2output.txt");
		outfile << "0" << endl;
		outfile.close();
	}

	// Input: Rotational 
	else if (toupper(input) == 'R') {
		isTranslational = false;
		outfile.open("txt/A2output.txt");
		outfile << "1" << endl;
		outfile << "50" << endl;
		outfile.close();
	}

	// Input: Error
	else {
		cout << "Input not recognized! ";
		system("PAUSE");
		return 0;
	}


	// Initiates GLFW and defines the settings
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);


	// Creates Window Object 
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Johnny Mak - Assignment #2", nullptr, nullptr);
	if (window == nullptr) {
		cout << "Failed to create GLFW Windows\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// Initiates GLEW (Extension Wrangler) 
	glewExperimental = GLU_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize GLEW\n";
		return -1;
	}

	// Creates the Viewport
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	centerX = width / 2;
	centerY = height / 2;

	cout << "Center Coordinates: (" << centerX << ", " << centerY << ") " << endl;

	// Enable Z-Buffer
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);


	// ========== Creating our shaders ========== //

	Shader ourShader(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);



	// ========== Creating our Vertex Buffer Obj, Vertex Array Obj ========== //

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);


	// Variables to be used in the main loop
	int minimum;
	int difference;

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Enable to see polygon lines

	

	// =============== Game Loop ================= //

	while (!glfwWindowShouldClose(window)) {

		// Check and call events
		glfwPollEvents();
		glfwSetKeyCallback(window, key_callback);
		glfwSetMouseButtonCallback(window, mouse_callback);

		// Resizes the window 
		minimum = min(width, height);
		difference = (width - height) / 2;
		glfwGetFramebufferSize(window, &width, &height);

		if (width > height) 
			glViewport(difference, 0, minimum, minimum);

		else 
			glViewport(0, difference, minimum, minimum);


		if (!isVertexEmpty) {
			glBindVertexArray(VAO); // Binds the VAO

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);	// Copy our vertices to the buffer

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);	// Set the vertex attribute pointers
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

			glBindVertexArray(0); // Unbinds the VAO
		}		

		// Rendering Commands
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.Use();

		// Projection, View, Model 
		glm::mat4 projection;
		glm::mat4 view;
		glm::mat4 model;

		model = glm::mat4(1.0);


		if (isCompleted && isDrawn && isTranslational) {
			projection = glm::mat4(1.0);
			view       = glm::mat4(1.0);
		}

		else if ( !isTranslational || !secondPass || isDrawn ) {
			view = glm::lookAt(glm::vec3(0.0, -3.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
			projection = glm::perspective(120.0f, (float)height / (float)width, 1.0f, 12.0f);
		}
		
		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		// Drawing the Triangle
		if (!isVertexEmpty) {
			glBindVertexArray(VAO);
			glDrawElements(drawMode, indices.size() * 2, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		// Swap the buffers
		glfwSwapBuffers(window);
	}

	// Frees up the buffers when done 
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}
// ========== End of Main Method ========== //


// Event handler on key press
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {

	if ((key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_P  && action == GLFW_PRESS)
		drawMode = GL_POINTS;

	if (key == GLFW_KEY_L  && action == GLFW_PRESS)
		drawMode = GL_LINES;

	if (key == GLFW_KEY_T  && action == GLFW_PRESS)
		drawMode = GL_TRIANGLES;

	if (key == GLFW_KEY_BACKSPACE  && action == GLFW_PRESS) {

		// Disable mouse click if spline is drawn
		if (!isDrawn) {
			cout << "Points Cleared! " << endl;
			vertices.clear();
			indices.clear();
			points.clear();
			printPoints.clear();
			isVertexEmpty = true;
		}

		else if (isCompleted) {
			cout << "Program ended, cannot erase! " << endl;
		}
		else 
			cout << "Spline already drawn, cannot erase points!" << endl;
	}
		

	if (key == GLFW_KEY_ENTER  && action == GLFW_PRESS) {
		
		// Disabled this key when the program completes
		if (isCompleted) {
			cout << "Finished clicking points!" << endl;
			vertices.clear();
			indices.clear();
			points.clear();
			printPoints.clear();
			isVertexEmpty = true;
		}

		// Disabled this key when it finished drawing
		else if (isDrawn) {
			cout << "IN HERE" << endl;
			vertices.clear();
			indices.clear();
			points.clear();
			printPoints.clear();
			secondPass = true;
			isDrawn = false;
			isVertexEmpty = true;
		}

		else if (points.size() < 4)
			cout << "Not enough points" << endl;

		else {

			// Calculate Catmull-Rom Spline
			for (int i = 0; i < points.size() - 3; i++) {
				printPoints.push_back(points.at(i + 1));
				subdivide(0, 1, 0.05, points.at(i), points.at(i + 1), points.at(i + 2), points.at(i + 3));
				printPoints.push_back(points.at(i + 2));
			}

			// Convert the vertices and update the indice
			convertVertices();
			updateIndices(printPoints.size());


			// Wrtiing the vertices into the file
			ofstream outfile;
			outfile.open("txt/A2output.txt", ios_base::app);
			outfile << vertices.size() / 3 << endl;

			for (int i = 0; i < vertices.size(); i++) {
				if (i % 3 == 0 && i != 0)
					outfile << endl;
				outfile << vertices.at(i) << " ";
			}

			outfile << endl;
			outfile.close();

			// Changing boolean values 
			isDrawn = true;

			// Set as Completed once we go through the second pass
			if (secondPass)
				isCompleted = true;

			// Disable everything after the first pass if not Translational
			if (!isTranslational) 
				isCompleted = true;

			// Enable Second pass if it is Translational
			else
				secondPass = true;
		}
	}
}


// Mouse Callback Function
void mouse_callback(GLFWwindow * window, int key, int action, int mods) {

	if (key == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		// Allow to click if it's not drawn
		if (!isDrawn) {
			glfwGetCursorPos(window, &cursorX, &cursorY);

			Point click;

			// Create a point on the XZ plane
			if (secondPass && isTranslational) {
				Point point(getXCoordinateValues(cursorX), getYCoordinateValues(cursorY), 0.0);
				click = point;
			}

			// Create a point on the XY plane
			else {
				Point point(getXCoordinateValues(cursorX), 0.0, getYCoordinateValues(cursorY));
				click = point;
			}

			points.push_back(click);

			updateIndices(points.size());
			convertVerticesTemp();
		}

		// Error if it's drawn
		else
			cout << "Finished clicking points!" << endl;
	}
}


// Function to Calculate Spline
Point calculateSpline(double u, Point firstPoint, Point secondPoint, Point thirdPoint, Point forthPoint) {

	// Control Matrix Values
	GLfloat coordinates[12] = {firstPoint.getCoordX() , secondPoint.getCoordX(), thirdPoint.getCoordX(), forthPoint.getCoordX(), 
		                       firstPoint.getCoordY() , secondPoint.getCoordY(), thirdPoint.getCoordY(), forthPoint.getCoordY(),
		                       firstPoint.getCoordZ() , secondPoint.getCoordZ(), thirdPoint.getCoordZ(), forthPoint.getCoordZ() };

	glm::mat3x4 points = glm::make_mat3x4(coordinates);

	// Parameter Matrix 
	glm::vec4 parameter(pow(u, 3), pow(u, 2) , u, 1);

	// Resulting Point
	glm::vec3 result = parameter * (basisMatrix * points);
	Point curvePoint(result.x, result.y, result.z);

	return curvePoint;
}


// Subdivion Recursive Function
void subdivide(double u0, double u1, double maxLength, Point firstPoint, Point secondPoint, Point thirdPoint, Point forthPoint) {

	double umid = (u0 + u1) / 2;

	Point first  = calculateSpline(u0, firstPoint, secondPoint, thirdPoint, forthPoint);
	Point second = calculateSpline(u1, firstPoint, secondPoint, thirdPoint, forthPoint);
	Point mid    = calculateSpline(umid, firstPoint, secondPoint, thirdPoint, forthPoint);

	if (distance(first, second) > maxLength) {
		subdivide(u0, umid, maxLength, firstPoint, secondPoint, thirdPoint,forthPoint);

		subdivide(umid, u1, maxLength, firstPoint, secondPoint, thirdPoint, forthPoint);
	}
	else {
		// Store Midpoint
		printPoints.push_back(mid);
	}
}

// Calculate distance between two points
double distance(Point first, Point second) {
	return sqrt((pow(first.getCoordX() - second.getCoordX(), 2)) + (pow(first.getCoordY() - second.getCoordY(), 2)) + (pow(first.getCoordZ() - second.getCoordZ(), 2)) );
}

// Convert window coordinate to space coordinate
GLfloat getXCoordinateValues(double pointX) {
	return (pointX - centerX) / (width / 2);
}

// Convert window coordinate to space coordinate
GLfloat getYCoordinateValues(double pointY) {

	return (centerY - pointY) / (height / 2);
}

// Store Vertex
void addVertex(GLfloat xCoord, GLfloat yCoord, GLfloat zCoord) {
	vertices.push_back(xCoord);
	vertices.push_back(yCoord);
	vertices.push_back(zCoord);
}

// Update Indice based on number of points
void updateIndices(int vertexCount) {

	indices.clear();

	for (int i = 1; i <= vertexCount; i++) {
		if (indices.size() <= 1)
			indices.push_back(i - 1);
		else {
			indices.push_back(i - 2);
			indices.push_back(i - 1);
		}
	}
}

// Print the vertices (Testing Purpose) 
void printVertices() {
	cout << "Printing Vertices: ";
	for (int i = 0; i < vertices.size(); i++) {
		cout << vertices.at(i) << ", ";
	}

	cout << endl;
}

// Print the indices (Testing Purposes) 
void printIndices() {
	cout << "Printing Indices: ";
	for (int i = 0; i < indices.size(); i++) {
		cout << indices.at(i) << ", ";
	}

	cout << endl;
}

// Convert Points to vertex coordinates
void convertVertices() {

	vertices.clear();

	for (int i = 0; i < printPoints.size(); i++) {
		vertices.push_back(printPoints.at(i).getCoordX());
		vertices.push_back(printPoints.at(i).getCoordY());
		vertices.push_back(printPoints.at(i).getCoordZ());
	}

	isVertexEmpty = false;
}

// Convert Points to vertex coordinates
void convertVerticesTemp() {

	vertices.clear();

	for (int i = 0; i < points.size(); i++) {
		vertices.push_back(points.at(i).getCoordX());
		vertices.push_back(points.at(i).getCoordY());
		vertices.push_back(points.at(i).getCoordZ());
	}

	isVertexEmpty = false;
}

