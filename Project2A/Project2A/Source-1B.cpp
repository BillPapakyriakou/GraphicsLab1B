//********************************
//Αυτό το αρχείο θα το χρησιμοποιήσετε
// για να υλοποιήσετε την άσκηση 1B της OpenGL
//
//ΑΜ:   5186             Όνομα:  Βασιλείου Νικόλαος Μιχαήλ
//ΑΜ:   5324             Όνομα: Παπακυριακού Βασίλειος

//*********************************

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

/// <summary>
/// //////////////////////////////////////////////
/// </summary>
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;


glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

////////////////////////////////////////////// <summary>
/// Add camera function here
/// </summary>

void camera_function()
{

}

/////////////////////////////////////////////////

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}


	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}
///////////////////////////////////////////////////

// 2D Array for storing the coords of the cubes for maze generation

int labyrinth[10][10] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{0, 0, 1, 1, 1, 1, 0, 1, 0, 1},
	{1, 0, 1, 0, 0, 0, 0, 1, 0, 1},
	{1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 1, 1, 1, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

std::vector<float> vertices;   // for storing maze walls coords 
std::vector<float> verticesA;   // for storing playerA coords 

float C_x = -4.5f; // character x pos
float C_y = 2.5f;  // character y pos
float C_z = 0.5f;  // character z pos

GLuint vertexBufferPlayer;  // needed here instead of main because Player buffer
GLuint vertexBufferMaze;    // is constantly being updated on keyCallbackPlayer method


void generateMazeVertices() {
	int rows = 10;
	int cols = 10;

	// Each square is of size 1x1, and we want to center the maze at (0, 0).

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < cols; ++j) {

			if (labyrinth[i][j] == 1) {

				// Calculate the four corners of the base of the cube

				float topLeftX = j - cols / 2.0f;
				float topLeftY = (rows / 2.0f) - i;

				float topRightX = topLeftX + 1.0f;
				float topRightY = topLeftY;

				float bottomLeftX = topLeftX;
				float bottomLeftY = topLeftY - 1.0f;

				float bottomRightX = topRightX;
				float bottomRightY = bottomLeftY;

				// Calculate Z positions for top and bottom faces

				float zBottom = 0.0f;
				float zTop = 1.0f;

				// Calculate triangles (2 for each face)

				// Bottom face (z = 0) triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);


				// Top face (z = 0) triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);


				// Front face triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);

				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);


				// Back face triangles
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);

				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);


				// Left face triangles
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);

				vertices.push_back(topLeftX); vertices.push_back(topLeftY); vertices.push_back(zTop);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zBottom);
				vertices.push_back(bottomLeftX); vertices.push_back(bottomLeftY); vertices.push_back(zTop);


				// Right face triangles
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);

				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zBottom);
				vertices.push_back(topRightX); vertices.push_back(topRightY); vertices.push_back(zTop);
				vertices.push_back(bottomRightX); vertices.push_back(bottomRightY); vertices.push_back(zTop);

			}
		}
	}
}
void DrawPlayer() {
	
	verticesA.clear();

	float halfSize = 0.25f;  // Half of the cube's side length
	/*
	// Front face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-right front

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-right front

	// Back face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-right back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-right back

	// Left face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-left back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-left back

	// Right face
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-right back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-right back

	// Top face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(0.25f);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(-0.25f);  // Top-left back

	// Bottom face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(-0.25f);  // Bottom-left back
	*/

	// Front face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-right front

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-right front

	// Back face (two triangles)
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-right back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-left back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-right back

	// Left face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-left back

	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-left front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-left back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-left back

	// Right face
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-right back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-right back
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-right back

	// Top face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z + 0.25f);  // Top-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y + halfSize); verticesA.push_back(C_z - 0.25f);  // Top-left back

	// Bottom face
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-left front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-right front
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-left back

	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z + 0.25f);  // Bottom-right front
	verticesA.push_back(C_x + halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-right back
	verticesA.push_back(C_x - halfSize); verticesA.push_back(C_y - halfSize); verticesA.push_back(C_z - 0.25f);  // Bottom-left back

}

// returns true if move is valid
bool isValidMove(int newX, int newY) {


	int checkX = newX;
	int checkY = newY + 2; // because player starts at [0][2] 


	if (checkX < 0 || checkX >= 10 || checkY < 0 || checkY >= 10) {
		//std::cout << "Out of bounds: (" << checkX << ", " << checkY << ")" << std::endl;
		return false;
	}

	//std::cout << "(" << checkX << ", " << checkY << ")" << std::endl;

	return labyrinth[checkY][checkX] == 0;

}


void keyCallbackPlayer(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		int gridX = static_cast<int>(C_x + 4.5f); // Convert to grid position
		int gridY = static_cast<int>(2.5f - C_y); // Convert to grid position


		int newX = gridX;
		int newY = gridY;

		const int startX = 0;
		const int startY = 0;
		const int exitX = 9;
		const int exitY = 5;
		
		// check if player is at start or at exit positions

		// check if L is pressed at exit and teleport to start
		if (gridX == exitX && gridY == exitY && key == GLFW_KEY_L) {
			C_x = startX - 4.5f;
			C_y = 2.5f - startY;

			// Update gridX and gridY to the new position
			gridX = startX;
			gridY = startY;

			DrawPlayer();  // redraw player after change

			// bind buffers to reflect the players positions change
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
			glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);
			
			
		}
		// check if J is pressed at start and teleport to exit
		else if (gridX == startX && gridY == startY && key == GLFW_KEY_J) {
			C_x = exitX - 4.5f;
			C_y = 2.5f - exitY;

			// Update gridX and gridY to the new position
			gridX = exitX;
			gridY = exitY;

			DrawPlayer();  // redraw player after change

			// bind buffers to reflect the players positions change
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
			glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);


		}
		
		else {


			// Handle movement keys
			switch (key) {
			case GLFW_KEY_L: newX += 1; break; // right
			case GLFW_KEY_J: newX -= 1; break; // left
			case GLFW_KEY_K: newY += 1; break; // down
			case GLFW_KEY_I: newY -= 1; break; // up
			}


			// Check if the new move is valid before updating
			if (isValidMove(newX, newY)) {

				C_x = newX - 4.5f;
				C_y = 2.5f - newY;

				// Update gridX and gridY after C_x and C_y change to reflect new position 
				gridX = newX;
				gridY = newY;

				DrawPlayer();  // redraw player after change

				//std::cout << "valid move" << std::endl;

				glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
				glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_DYNAMIC_DRAW);
				
				
			}

		}

	//std::cout << "Current grid position: (" << gridX << ", " << gridY << ")" << std::endl;	
	
	}
	
}


// Starting camera position and zoom factor
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 20.0f);
float zoomFactor = 20.0f;
float rotationAngleX = 0.0f;
float rotationAngleY = 0.0f;


void keyCallbackCamera(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {

		// Move along the X-axis
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			//cameraPosition.x -= 0.1f;  // left movement
			rotationAngleX -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
			//cameraPosition.x += 0.1f;  // right movement
			rotationAngleX += 0.1f;
		}

		// Move along the Y-axis
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
			//cameraPosition.y -= 0.1f;  // down movement
			rotationAngleY -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			//cameraPosition.y += 0.1f;  // up movement
			rotationAngleY += 0.1f;
		}

		// Zoom in and out (move along the Z axis)
		if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
			zoomFactor -= 0.1f;  // zoom in
		}
		if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
			zoomFactor += 0.1f;  // zoom out
		}

		// Minimum zoom level limiter
		zoomFactor = std::max(zoomFactor, 1.0f);
		cameraPosition.z = zoomFactor;

	
	}
}

// This function is called by GLFW whenever a key is pressed.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	// Handle player keys
	keyCallbackPlayer(window, key, scancode, action, mods);

	// Handle camera keys
	keyCallbackCamera(window, key, scancode, action, mods);

}



int main(void)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// create 950 x 950 window
	window = glfwCreateWindow(950, 950, u8"’σκηση 1Β - 2024", NULL, NULL);


	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Set key callback
	glfwSetKeyCallback(window, keyCallback);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Black background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders

	GLuint programID = LoadShaders("P1BVertexShader.vertexshader", "P1BFragmentShader.fragmentshader");

	GLuint MatrixID = glGetUniformLocation(programID, "MVP");


	/*
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 100.0f);

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(5.0f, 5.0f, 5.0f), // Camera in World Space
		glm::vec3(0.0f, 0.0f, 0.0f), // and looks at the origin
		glm::vec3(0.0f, 0.0f, 1.0f));  // Head is up 


	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model;
	*/


	GLfloat len = 5.0f, wid = 2.5f, heig = 2.5f;

	static const GLfloat cube[] =
	{
		-2.0f,-2.0f,-2.0f,
		-2.0f,-2.0f, 2.0f,
		-2.0f, 2.0f, 2.0f,

		 2.0f, 2.0f,-2.0f,
		-2.0f,-2.0f,-2.0f,
		-2.0f, 2.0f,-2.0f,

		 2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f,-2.0f,
		 2.0f,-2.0f,-2.0f,

		 2.0f, 2.0f,-2.0f,
		 2.0f,-2.0f,-2.0f,
		-2.0f,-2.0f,-2.0f,

		-2.0f,-2.0f,-2.0f,
		-2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f,-2.0f,

		 2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f, 2.0f,
		-2.0f,-2.0f,-2.0f,

		-2.0f, 2.0f, 2.0f,
		-2.0f,-2.0f, 2.0f,
		 2.0f,-2.0f, 2.0f,

		 2.0f, 2.0f, 2.0f,
		 2.0f,-2.0f,-2.0f,
		 2.0f, 2.0f,-2.0f,

		 2.0f,-2.0f,-2.0f,
		 2.0f, 2.0f, 2.0f,
		 2.0f,-2.0f, 2.0f,

		 2.0f, 2.0f, 2.0f,
		 2.0f, 2.0f,-2.0f,
		-2.0f, 2.0f,-2.0f,

		 2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f,-2.0f,
		-2.0f, 2.0f, 2.0f,

		 2.0f, 2.0f, 2.0f,
		-2.0f, 2.0f, 2.0f,
		 2.0f,-2.0f, 2.0f
	};

	GLfloat a = 0.4f;
	static const GLfloat color[] = {
		0.583f,  0.771f,  0.014f,a,
		0.609f,  0.115f,  0.436f,a,
		0.327f,  0.483f,  0.844f,a,
		0.822f,  0.569f,  0.201f,a,
		0.435f,  0.602f,  0.223f,a,
		0.310f,  0.747f,  0.185f,a,
		0.597f,  0.770f,  0.761f,a,
		0.559f,  0.436f,  0.730f,a,
		0.359f,  0.583f,  0.152f,a,
		0.483f,  0.596f,  0.789f,a,
		0.559f,  0.861f,  0.639f,a,
		0.195f,  0.548f,  0.859f,a,
		0.014f,  0.184f,  0.576f,a,
		0.771f,  0.328f,  0.970f,a,
		0.406f,  0.615f,  0.116f,a,
		0.676f,  0.977f,  0.133f,a,
		0.971f,  0.572f,  0.833f,a,
		0.140f,  0.616f,  0.489f,a,
		0.997f,  0.513f,  0.064f,a,
		0.945f,  0.719f,  0.592f,a,
		0.543f,  0.021f,  0.978f,a,
		0.279f,  0.317f,  0.505f,a,
		0.167f,  0.620f,  0.077f,a,
		0.347f,  0.857f,  0.137f,a,
		0.055f,  0.953f,  0.042f,a,
		0.714f,  0.505f,  0.345f,a,
		0.783f,  0.290f,  0.734f,a,
		0.722f,  0.645f,  0.174f,a,
		0.302f,  0.455f,  0.848f,a,
		0.225f,  0.587f,  0.040f,a,
		0.517f,  0.713f,  0.338f,a,
		0.053f,  0.959f,  0.120f,a,
		0.393f,  0.621f,  0.362f,a,
		0.673f,  0.211f,  0.457f,a,
		0.820f,  0.883f,  0.371f,a,
		0.982f,  0.099f,  0.879f,a,
	};


	// draw maze 
	generateMazeVertices();


	// draw player
	DrawPlayer();

	
	std::vector<GLfloat> mazeColors(vertices.size() / 3 * 4, 0.0f);
	// GLfloat array size = total num of vertices (vertices_size / 3) times 4 (color chanels)
	

	// Define blue color for each maze vertex

	for (size_t i = 0; i < mazeColors.size(); i += 4) {
		mazeColors[i] = 0.0f;       // Red 
		mazeColors[i + 1] = 0.0f;   // Green 
		mazeColors[i + 2] = 1.0f;   // Blue
		mazeColors[i + 3] = 1.0f;   // Alpha 
	}

	std::vector<GLfloat> mazeColorsA(verticesA.size() / 3 * 4, 0.0f);
	// GLfloat array size = total num of verticesA (verticesA_size / 3) times 4 (color chanels)


	// Define red color for each maze vertex

	for (size_t i = 0; i < mazeColorsA.size(); i += 4) {
		mazeColorsA[i] = 1.0f;       // Red
		mazeColorsA[i + 1] = 1.0f;   // Green
		mazeColorsA[i + 2] = 0.0f;   // Blue
		mazeColorsA[i + 3] = 1.0f;   // Alpha
	}

	// Generate and bind buffer for the maze
	glGenBuffers(1, &vertexBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);


	// Generate and bind buffer for the player
	glGenBuffers(1, &vertexBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, verticesA.size() * sizeof(float), verticesA.data(), GL_STATIC_DRAW);


	// Generate and bind the color buffer for the maze
	GLuint colorBufferMaze;
	glGenBuffers(1, &colorBufferMaze);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferMaze);
	glBufferData(GL_ARRAY_BUFFER, mazeColors.size() * sizeof(GLfloat), mazeColors.data(), GL_STATIC_DRAW);


	// Generate and bind the color buffer for the player
	GLuint colorBufferPlayer;
	glGenBuffers(1, &colorBufferPlayer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferPlayer);
	glBufferData(GL_ARRAY_BUFFER, mazeColorsA.size() * sizeof(GLfloat), mazeColorsA.data(), GL_STATIC_DRAW);


	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);


	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);


	do {

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);
		
		/*
		// Camera matrix new

		glm::mat4 View = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 20.0f),
			glm::vec3(0.0f, 0.0f, 0.25f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);
		*/

		// Projection matrix
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 4.0f, 0.1f, 100.0f);


		// Update the view matrix based on the current position of the camera
		
		// View matrix
		glm::mat4 View = glm::lookAt(
			cameraPosition,               // camera position
			glm::vec3(0.0f, 0.0f, 0.25f), // looks towards the center of the labyrinth
			glm::vec3(0.0f, 1.0f, 0.0f)   // up vector
		);

		// Model matrix
		glm::mat4 Model = glm::mat4(1.0f);

		View = glm::rotate(View, rotationAngleX, glm::vec3(0.5f, 0.0f, 0.0f)); // rotate around X 
		View = glm::rotate(View, rotationAngleY, glm::vec3(0.0f, 0.5f, 0.0f)); // rotate around Y


		// Calculate the MVP matrix
		glm::mat4 MVP = Projection * View * Model;

		// Pass MVP matrix to shader
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

			

		// Draw maze walls
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferMaze);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// Enable the color attribute array (index 1 in shader)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferMaze);
		glVertexAttribPointer(
			1,            // Attribute location in shader
			4,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

			

		// Draw player
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferPlayer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
			
		// Enable the color attribute array (index 1 in shader)
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferPlayer);
		glVertexAttribPointer(
			1,            // Attribute location in shader
			4,            // Size of each color component (RGBA)
			GL_FLOAT,     // Type of each component
			GL_FALSE,     // Don't normalize
			0,            // Stride
			(void*)0      // Offset
		);
			
			
		glDrawArrays(GL_TRIANGLES, 0, verticesA.size() / 3);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);


		/*
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,
			4,                                // size
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		*/


		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the SPACE key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &vertexBufferMaze);
	glDeleteBuffers(1, &vertexBufferPlayer);
	glDeleteBuffers(1, &colorBufferMaze);
	glDeleteBuffers(1, &colorBufferPlayer);
	glDeleteProgram(programID);

	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;

}


