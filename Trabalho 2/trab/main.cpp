/*
Universidade Federal da Fronteira Sul
Trabalho 2 de Computação Gráfica
Alunos: Doglas André Finco
		Patrick De Bastiani


Objective:
• Implement a interface that allow us to test the transformations over a triangular mesh:
	– Translation
	– Scale
	– Shear
	– Rotation around a point
	– Rotation around a arbitrary axis
	– Non-uniform scale
	– Animate the mesh using time as input
	– Etc..
• Render several models
	– Each with a different transformation
	– Implement a scene manager
• Translation, rotation, etc..
• class ModelManager
	– Store the models and meshes
• class Model
	– Mesh ID + Transformation
• class Mesh
	– Mesh
*/

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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;
 
// Include AntTweakBar
#include <AntTweakBar.h>
 
 
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards

vec3 gPosition1(2.0f, 0.0f, -1.0f);
vec3 gOrientation1(0.0f, 0.0f, 0.0f);
vec3 gScaleNon(1.0f, 1.0f, 1.0f);
float gScaleUni=1.0f;
float gAngulo = 0.0f;
vec3 gShear(1.57f, 1.57f, 1.57f);
vec3 gAroundPoint(0.0f, 0.0f, 0.0f);
vec3 arbitraryAxisOrientation[3]; // Eixo arbitrário
vec3 rotationP(0.0f, 0.0f, 0.0f); // Rotação ponto

vec3 gPosition2(-12.0f, -1.0f, -1.0f);
quat gOrientation2;
 
bool gLookAtOther = true;
bool gAnimation = true;
bool gRotationPoint= false;

 
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
	if( !glfwOpenWindow( 1024, 768, 0,0,0,0, 32,0, GLFW_WINDOW ) )
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
 
	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(1024, 768);

	TwBar * EulerGUI = TwNewBar("Interface");
	TwBar * QuaternionGUI = TwNewBar("Animacao");

	TwSetParam(QuaternionGUI, NULL, "position", TW_PARAM_CSTRING, 1, "808 16");
	TwSetParam(EulerGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");

	TwAddVarRW(EulerGUI, "Rotação no eixo X", TW_TYPE_FLOAT, &gOrientation1.x, "step=0.01");
	TwAddVarRW(EulerGUI, "Rotação no eixo Y", TW_TYPE_FLOAT, &gOrientation1.y, "step=0.01");
	TwAddVarRW(EulerGUI, "Rotação no eixo Z", TW_TYPE_FLOAT, &gOrientation1.z, "step=0.01");

	TwAddVarRW(EulerGUI, "Translação X", TW_TYPE_FLOAT, &gPosition1.x, "step=0.01");
	TwAddVarRW(EulerGUI, "Translação Y", TW_TYPE_FLOAT, &gPosition1.y, "step=0.01");
	TwAddVarRW(EulerGUI, "Translação Z", TW_TYPE_FLOAT, &gPosition1.z, "step=0.01");

	TwAddVarRW(EulerGUI, "Scala X", TW_TYPE_FLOAT, &gScaleNon.x, "step=0.01");
	TwAddVarRW(EulerGUI, "Scala Y", TW_TYPE_FLOAT, &gScaleNon.y, "step=0.01");
	TwAddVarRW(EulerGUI, "Scala Z", TW_TYPE_FLOAT, &gScaleNon.z, "step=0.01");
	TwAddVarRW(EulerGUI, "Scala Uniforme", TW_TYPE_FLOAT, &gScaleUni, "step=0.01");

	TwAddVarRW(EulerGUI, "Shear X", TW_TYPE_FLOAT, &gShear.x, "step=0.01");
	TwAddVarRW(EulerGUI, "Shear Y", TW_TYPE_FLOAT, &gShear.y, "step=0.01");
	TwAddVarRW(EulerGUI, "Shear Z", TW_TYPE_FLOAT, &gShear.z, "step=0.01");

	TwAddVarRW(EulerGUI, "Rotação num ponto", TW_TYPE_BOOL8 , &gRotationPoint, "help='Look at the other monkey ?'");
		TwAddVarRW(EulerGUI, "Sobre Ponto X", TW_TYPE_FLOAT, &gAroundPoint.x, "step=0.01");
		TwAddVarRW(EulerGUI, "Sobre Ponto Y", TW_TYPE_FLOAT, &gAroundPoint.y, "step=0.01");
		TwAddVarRW(EulerGUI, "Sobre Ponto Z", TW_TYPE_FLOAT, &gAroundPoint.z, "step=0.01");

	arbitraryAxisOrientation[0] = glm :: vec3(1.0f, 1.0f, 1.0f);
	TwAddVarRW(EulerGUI, "Vetor"  , TW_TYPE_DIR3F, &arbitraryAxisOrientation[0], "step=0.01");
	TwAddVarRW(EulerGUI, "Angulo"  , TW_TYPE_FLOAT, &gAngulo, "step=0.01");

	TwAddVarRW(QuaternionGUI, "Ativar Animação", TW_TYPE_BOOL8 , &gAnimation, "help='Look at the other monkey ?'");

	TwDefine(" Interface size='300 500' ");
	TwDefine(" Animacao size='250 100' ");

	
	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar
 
 
 
	glfwSetWindowTitle( "Tabalho 2" );
 
	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(1024/2, 768/2);
 
	// Dark blue background
	glClearColor(0.4f, 0.4f, 0.4f, 0.0f);
 
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
 
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
 
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );
 
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
 
	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
 
	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

 
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
 
	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
 
	// Load it into a VBO
 
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
 
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
 
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
 
	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);
 
	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
 
	// For speed computation
	double lastTime = glfwGetTime();
	double lastFrameTime = lastTime;
	float a=-1.5f, b=-2.0f, c=-3.0f;
	int k=0;
 
	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
		// Use our shader
		glUseProgram(programID);
 
		glm::mat4 ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
		glm::mat4 ViewMatrix = glm::lookAt(
			glm::vec3( 0, 0, 7 ), // Camera is here
			glm::vec3( 0, 0, 0 ), // and looks here
			glm::vec3( 0, 1, 0 )  // Head is up (set to 0,-1,0 to look upside-down)
		);
 
		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);
 
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);
 
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                   // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);
 
		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,    // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);
 
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
 
		glm::vec3 lightPos = glm::vec3(0,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		//Necessário para a Animação
		vec3 gPosition2(a, b, c);
		if(gAnimation){
			if(k==0){
				c-=0.005f;
				if(c<= -9) k=1;
			}else if(k==1){
				c+=0.005f;
				if(c>= -2) k=0;
			}
		}
		
		//Opção para escala Uniforme ou não

		{
			// Build the model matrix
			// Operações para rotação ao redor de um eixo arbitrário
			arbitraryAxisOrientation[2].x = arbitraryAxisOrientation[1].x - arbitraryAxisOrientation[0].x;
			arbitraryAxisOrientation[2].y = arbitraryAxisOrientation[1].y - arbitraryAxisOrientation[0].y;
			arbitraryAxisOrientation[2].z = arbitraryAxisOrientation[1].z - arbitraryAxisOrientation[0].z;
					
			arbitraryAxisOrientation[2] = glm::normalize(arbitraryAxisOrientation[2]); // Normalização do vetor
						
			glm::mat4 arbitraryAxisMatriz = mat4(1);
			if(glm::length(arbitraryAxisOrientation[2]) > 0.5 && glm::length(arbitraryAxisOrientation[2])< 1.5){
				float A = 1 - cos(gAngulo);
				arbitraryAxisMatriz = mat4( // Matriz para rotação em um eixo arbitrário
					vec4(cos(gAngulo)+(pow(arbitraryAxisOrientation[2].x,2)*A),(arbitraryAxisOrientation[2].y * arbitraryAxisOrientation[2].x * A +(arbitraryAxisOrientation[2].z * sin(gAngulo))), (arbitraryAxisOrientation[2].z * arbitraryAxisOrientation[2].x * A -(arbitraryAxisOrientation[2].y * sin(gAngulo))), (0)), // coluna
					vec4((arbitraryAxisOrientation[2].x * arbitraryAxisOrientation[2].y * A - (arbitraryAxisOrientation[2].z * sin(gAngulo))),(cos(gAngulo)+(pow(arbitraryAxisOrientation[2].y,2)*A)),(arbitraryAxisOrientation[2].z * arbitraryAxisOrientation[2].y * A + (arbitraryAxisOrientation[2].x * sin(gAngulo))),(0)),
					vec4(arbitraryAxisOrientation[2].x * arbitraryAxisOrientation[2].z * A + (arbitraryAxisOrientation[2].y * sin(gAngulo)),(arbitraryAxisOrientation[2].y * arbitraryAxisOrientation[2].z * A - (arbitraryAxisOrientation[2].x * sin(gAngulo))),(cos(gAngulo)+(pow(arbitraryAxisOrientation[2].z,2)*A)),(0)),
					vec4((0),(0),(0),(1))
				);
			}

			// Matrizes para rotação em um eixo arbitrário
			glm::mat4 Rx = mat4(
				vec4((1), (0), (0), (0)),
				vec4((0), (cos(gOrientation1.x)), (sin(gOrientation1.x)), (0)),
				vec4((0), (-sin(gOrientation1.x)), (cos(gOrientation1.x)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em X
			
			glm::mat4 Ry = mat4(
				vec4((cos(gOrientation1.y)), (0), (-sin(gOrientation1.y)), (0)),
				vec4((0), (1), (0), (0)),
				vec4((sin(gOrientation1.y)), (0), (cos(gOrientation1.y)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Y
			
			glm::mat4 Rz = mat4(
				vec4((cos(gOrientation1.z)), (sin(gOrientation1.z)), (0), (0)),
				vec4((-sin(gOrientation1.z)), (cos(gOrientation1.z)), (0), (0)),
				vec4((0), (0), (1), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Z

			glm::mat4 RotationMatrix = translate(mat4(), rotationP) * Rx * Ry *Rz * translate(mat4(), -rotationP);
			glm::mat4 TranslationMatrix = translate(mat4(), gPosition1); // A bit to the left
			glm::mat4 ScalingMatrix = scale(mat4(), gScaleNon*gScaleUni);
			glm::mat4 TranslationInversa = translate(mat4(), -gPosition1);
			glm::mat4 ShearMatrix = mat4(
											vec4(1.0f, 1/tan(-gShear.y), 1/tan(-gShear.z), 0),
											vec4(1/tan(-gShear.x), 1.0f, 1/tan(-gShear.z), 0),
											vec4(1/tan(-gShear.x), 1/tan(-gShear.y), 1.0f, 0),
											vec4(0, 0, 0, 1.0f)										
										);

			glm::mat4 AroundPointX = mat4(
											vec4(1, 0, 0, 0),
											vec4(0, cos(gAroundPoint.x), sin(gAroundPoint.x), 0),
											vec4(0, -sin(gAroundPoint.x), cos(gAroundPoint.x), 0),
											vec4(0, gAroundPoint.z - (gAroundPoint.z * cos(gAroundPoint.x)) + (gAroundPoint.y * sin(gAroundPoint.x)), gAroundPoint.y - (gAroundPoint.z * sin(gAroundPoint.x)) - (gAroundPoint.y * cos(gAroundPoint.x)), 1)
										);
			glm::mat4 AroundPointY = mat4(
											vec4(cos(gAroundPoint.y), 0, sin(gAroundPoint.y), 0),
											vec4(0, 1, 0, 0),
											vec4(-sin(gAroundPoint.y), 0, cos(gAroundPoint.y), 0),
											vec4(gAroundPoint.x - (gAroundPoint.x * cos(gAroundPoint.y)) + (gAroundPoint.z * sin(gAroundPoint.y)), 0, gAroundPoint.z - (gAroundPoint.x * sin(gAroundPoint.y)) - (gAroundPoint.z * cos(gAroundPoint.y)), 1)
										);
			glm::mat4 AroundPointZ = mat4(
											vec4(cos(gAroundPoint.z), sin(gAroundPoint.z), 0, 0),
											vec4(-sin(gAroundPoint.z), cos(gAroundPoint.z), 0, 0),
											vec4(0, 0, 1, 0),
											vec4(gAroundPoint.x - (gAroundPoint.x * cos(gAroundPoint.z)) + (gAroundPoint.y * sin(gAroundPoint.z)), gAroundPoint.y - (gAroundPoint.x * sin(gAroundPoint.z)) - (gAroundPoint.y * cos(gAroundPoint.z)), 0, 1)
										);

			glm::mat4 ModelMatrix;



			//Opção por escala ao redor de um ponto
			if(gRotationPoint){
				ModelMatrix = TranslationInversa * RotationMatrix * TranslationMatrix * ScalingMatrix * ModelMatrix * ShearMatrix * AroundPointX * AroundPointY * AroundPointZ * arbitraryAxisMatriz;
			}else{
				glm::mat4 TranslationInversa(1.0f);
				ModelMatrix = TranslationInversa * RotationMatrix * TranslationMatrix * ScalingMatrix * ModelMatrix * ShearMatrix * arbitraryAxisMatriz;
			}

			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
 
			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
 
 
 
			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,      // mode
				indices.size(),    // count
				GL_UNSIGNED_SHORT,   // type
				(void*)0           // element array buffer offset
			);
 
		}
		{ 
			glm::mat4 Rx = mat4(
				vec4((1), (0), (0), (0)),
				vec4((0), (cos(gOrientation2.x)), (sin(gOrientation2.x)), (0)),
				vec4((0), (-sin(gOrientation2.x)), (cos(gOrientation2.x)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em X
			
			glm::mat4 Ry = mat4(
				vec4((cos(gOrientation2.y)), (0), (-sin(gOrientation2.y)), (0)),
				vec4((0), (1), (0), (0)),
				vec4((sin(gOrientation2.y)), (0), (cos(gOrientation2.y)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Y
			
			glm::mat4 Rz = mat4(
				vec4((cos(gOrientation2.z)), (sin(gOrientation2.z)), (0), (0)),
				vec4((-sin(gOrientation2.z)), (cos(gOrientation2.z)), (0), (0)),
				vec4((0), (0), (1), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Z
			if(gAnimation){
				if(k == 0){
					gOrientation2.y -= 0.001f;
					gOrientation2.x -= 0.0005f;
					gOrientation2.z -= 0.0001f;
				}else if(k == 1){
					gOrientation2.y += 0.001f;
					gOrientation2.x += 0.0005f;
					gOrientation2.z += 0.0001f;
				}
			}
			glm::mat4 RotationMatrix1 = translate(mat4(), rotationP) * Rx * Ry *Rz * translate(mat4(), -rotationP);

			glm::mat4 RotationMatrix = mat4_cast(gOrientation2);
			glm::mat4 TranslationMatrix = translate(mat4(), gPosition2); // A bit to the right
			glm::mat4 ScalingMatrix = scale(mat4(), vec3(1.0f, 1.0f, 1.0f));
			glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix * RotationMatrix1;
 
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
 
			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
 
 
			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,      // mode
				indices.size(),    // count
				GL_UNSIGNED_SHORT,   // type
				(void*)0           // element array buffer offset
			);
		}
 
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
 
		// Draw GUI
		TwDraw();
 
		// Swap buffers
		glfwSwapBuffers();
 
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ) );
 
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
 
	// Close GUI and OpenGL window, and terminate GLFW
	TwTerminate();
	glfwTerminate();
 
	return 0;
}