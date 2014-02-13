// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/reflection.h>
#include <common/text2D.hpp>

#include "wtypes.h";

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;

void setMVP(glm::mat4 ProjectionMatrix, glm::mat4 ViewMatrix, glm::mat4 ModelMatrix){
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
}

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;
GLuint Texture;
GLuint TextureID;
std::vector<unsigned short> indices;

void GetDesktopResolution(int& horizontal, int& vertical){
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

void draw(){

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glUniform1i(TextureID, 0);

	glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	int T_X = 0;
	int T_Y = 0;

	GetDesktopResolution(T_X, T_Y);
	if( !glfwOpenWindow( T_X, T_Y, 0,0,0,0, 32,0, GLFW_FULLSCREEN ) )
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	glfwSetWindowTitle( "Screensaver" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Dark blue background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
	glUseProgram(programID);

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ModelMatrixID = glGetUniformLocation(programID, "M");

	
	GLuint textureColor = glGetUniformLocation(programID, "textureColor");
	glUniform3f(textureColor, -1.0, -1.0, -1.0);

	// Load the texture
	Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("logo.obj", vertices, uvs, normals);

	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
	
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
	
	float aspec = 6;

	Reflection reflect1(aspec, aspec, (glm::mat4)glm::scale(1, 1, 1));//depth
	Reflection reflect2(aspec, aspec, (glm::mat4)glm::scale(1, 1, 1));//left
	Reflection reflect3(aspec, aspec, (glm::mat4)glm::scale(1, 1, 1));//right
	Reflection reflect4(aspec, aspec, (glm::mat4)glm::scale(1, 1, 1));//up
	Reflection reflect5(aspec, aspec, (glm::mat4)glm::scale(1, 1, 1));//down

	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	glm::vec3 lightPos = glm::vec3(0,3,3);
	glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

	initText2D("Holstein.tga");

	do{
		double currentTime = glfwGetTime();

		//reflection.transformation = glm::rotate(sin(currentTime)*0, 0.0, 1.0, 0.0);
		reflect1.transformation = rotate(mat4(), 0.0f, vec3(0, 1, 0)) * translate(mat4(), vec3(0, 0, -4));
		reflect2.transformation = rotate(mat4(), 90.0f, vec3(0, 1, 0)) * translate(mat4(), vec3(0, 0, -4));
		reflect3.transformation = rotate(mat4(), 270.0f, vec3(0, 1, 0)) * translate(mat4(), vec3(0, 0, -4));
		reflect4.transformation = rotate(mat4(), 90.0f, vec3(1, 0, 0)) * translate(mat4(), vec3(0, 0, -4));
		reflect5.transformation = rotate(mat4(), 270.0f, vec3(1, 0, 0)) * translate(mat4(), vec3(0, 0, -4));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		computeMatricesFromInputs();
		reflect1.setCamera(getPosition(), getDirection(), getUp());
		reflect2.setCamera(getPosition(), getDirection(), getUp());
		reflect3.setCamera(getPosition(), getDirection(), getUp());
		reflect4.setCamera(getPosition(), getDirection(), getUp());
		reflect5.setCamera(getPosition(), getDirection(), getUp());

		reflect1.init();
		setMVP(
			reflect1.projectionMatrix,
			reflect1.viewMatrix,
			translate(mat4(), vec3(sin(currentTime)*2, 0, 0)) * translate(mat4(), vec3(0, 0, 0)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0)) * scale(mat4(), vec3(0.1, 0.1, 0.1))
		);
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);
		reflect1.finish();

		reflect2.init();

		float asd1 = (sin(currentTime+3.1415)+1.0)/2.0;
		asd1*=2;
		setMVP(
			reflect2.projectionMatrix,
			reflect2.viewMatrix,
			translate(mat4(), vec3(0,0,0)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0)) * scale(mat4(), vec3(asd1*0.05+0.12, asd1*0.04+0.07, asd1*0.05+0.07))
		);
		
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);
		reflect2.finish();

		reflect3.init();

		float asd2 = (sin(currentTime)+1.0)/2.0;
		asd2*=2;
		setMVP(
			reflect3.projectionMatrix,
			reflect3.viewMatrix,
			translate(mat4(), vec3(0,0,0)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0))  * scale(mat4(), vec3(asd2*0.05+0.12, asd2*0.04+0.07, asd2*0.05+0.07))
		);
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);
		reflect3.finish();

		reflect4.init();
		setMVP(
			reflect4.projectionMatrix,
			reflect4.viewMatrix,
			glm::mat4(1.0) * translate(mat4(), vec3(0, 0, 0)) * translate(mat4(), vec3(sin(currentTime)*2, 0, -2)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0)) * scale(mat4(), vec3(0.15, 0.15, 0.15))
		);
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);
		reflect4.finish();

		reflect5.init();
		setMVP(
			reflect5.projectionMatrix,
			reflect5.viewMatrix,
			glm::mat4(1.0) * translate(mat4(), vec3(0, 0, 0)) * translate(mat4(), vec3(sin(currentTime)*2, 0, -2)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0)) * scale(mat4(), vec3(0.15, 0.15, 0.15))
		);
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);
		reflect5.finish();
		
		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			reflect1.transformation
		);
		reflect1.draw();

		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			reflect2.transformation
		);
		reflect2.draw();

		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			reflect3.transformation
		);
		reflect3.draw();

		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			reflect4.transformation
		);
		reflect4.draw();

		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			reflect5.transformation
		);
		reflect5.draw();
		
		setMVP(
			getProjectionMatrix(),
			getViewMatrix(),
			glm::mat4(1.0) * translate(mat4(), vec3(0, 0, 0)) * translate(mat4(), vec3(sin(currentTime)*2, 0, -2)) * (glm::mat4)glm::rotate(currentTime*100, 0.0, 1.0, 0.0) * rotate(mat4(), 90.0f, vec3(1, 0, 0)) * scale(mat4(), vec3(0.15, 0.15, 0.15))
		);
		glUniform3f(textureColor, 0.0, 1.0, 0.0);
		draw();
		glUniform3f(textureColor, -1.0, -1.0, -1.0);


		char text[256];

		char dev1[]= "Doglas A. Finco";
		sprintf(text, "%s", dev1);
		printText2D(text, 600, 20, 10);

		char dev2[]= "Patrick de Bastiani";
		sprintf(text, "%s", dev2);
		printText2D(text, 600, 10, 10);

		glfwSwapBuffers();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ) );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	cleanupText2D();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}