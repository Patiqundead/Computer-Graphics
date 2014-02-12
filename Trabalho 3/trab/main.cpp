/*
Universidade Federal da Fronteira Sul
Trabalho 3 de Computação Gráfica
Alunos: Doglas André Finco
		Patrick De Bastiani


Objective:
• Implement a interface that allow us to test the camera transformations:
	– Translation
	– Look at
	– Rotation around a point
	– Rotation around a arbitrary axis
	– Animate the camera using time as input
• Translation, rotation, etc..
• Define a path for the camera to follow
• Introduce some noise to the camera movement
	– Etc..
• Allow the user to choose the number of cameras
	– Each with a different transformation
	– Different position
	– Different angle of view
	– Different Near and Far clipping
	– Etc.
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
#include <glm/gtc/noise.hpp>
using namespace glm;
 
// Include AntTweakBar
#include <AntTweakBar.h>
 
 
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/quaternion_utils.hpp> // See quaternion_utils.cpp for RotationBetweenVectors, LookAt and RotateTowards
// perspective matrix
float fovy = 45.0f;
float aspectA = 4.0f;
float aspectB = 3.0f;
float near = 0.1f;
float far = 100.0f;

//  view matrix
vec3 gPosition1Camera(0.0f, 0.0f, 5.0f);//Posição da câmera
vec3 gLook1Camera(0.0f, 0.0f, -5.0f);//Direção que a câmera está olhando
vec3 gUpVector1Camera(0.0f, 1.0f, 0.0f);//Up Vector(Pra cima)
vec3 arbitraryAxisOrientationCam[3]; // Eixo arbitrário
float gAnguloCam = 0.0f;
vec3 gAroundPointCam(0.0f, 0.0f, 0.0f);
vec3 gOrientation1Cam(0.0f, 0.0f, 0.0f);
bool gAnimationCam = false;

// model matrix
vec3 gPosition1(0.0f, 0.0f, -10.0f);
vec3 gOrientation1(0.0f, 0.0f, 0.0f);
vec3 gScaleNon(1.0f, 1.0f, 1.0f);
float gScaleUni=1.0f;
float gAngulo = 0.0f;
vec3 gShear(1.57f, 1.57f, 1.57f);
vec3 gAroundPoint(0.0f, 0.0f, 0.0f);
vec3 arbitraryAxisOrientation[3]; // Eixo arbitrário
vec3 rotationP(0.0f, 0.0f, 0.0f); // Rotação ponto

 
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

	TwBar * EulerGUI = TwNewBar("Objeto");
	TwBar * CameraGUI = TwNewBar("Camera");
	TwBar * WorldGUI = TwNewBar("Mundo");

	//------INTERFACE OBJETO-----------------------

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

	//----INTERFACE CÂMERA------------------------------
	TwSetParam(CameraGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");

	TwAddVarRW(EulerGUI, "Rotação no eixo X", TW_TYPE_FLOAT, &gOrientation1Cam.x, "step=0.01");
	TwAddVarRW(EulerGUI, "Rotação no eixo Y", TW_TYPE_FLOAT, &gOrientation1Cam.y, "step=0.01");
	TwAddVarRW(EulerGUI, "Rotação no eixo Z", TW_TYPE_FLOAT, &gOrientation1Cam.z, "step=0.01");

	TwAddVarRW(CameraGUI, "Translação X", TW_TYPE_FLOAT, &gPosition1Camera.x, "step=0.01");
	TwAddVarRW(CameraGUI, "Translação Y", TW_TYPE_FLOAT, &gPosition1Camera.y, "step=0.01");
	TwAddVarRW(CameraGUI, "Translação Z", TW_TYPE_FLOAT, &gPosition1Camera.z, "step=0.01");

	TwAddVarRW(CameraGUI, "LookAt X", TW_TYPE_FLOAT, &gLook1Camera.x, "step=0.01");
	TwAddVarRW(CameraGUI, "LookAt Y", TW_TYPE_FLOAT, &gLook1Camera.y, "step=0.01");
	TwAddVarRW(CameraGUI, "LookAt Z", TW_TYPE_FLOAT, &gLook1Camera.z, "step=0.01");

	TwAddVarRW(CameraGUI, "UpVector X", TW_TYPE_FLOAT, &gUpVector1Camera.x, "step=0.01");
	TwAddVarRW(CameraGUI, "UpVector Y", TW_TYPE_FLOAT, &gUpVector1Camera.y, "step=0.01");
	TwAddVarRW(CameraGUI, "UpVector Z", TW_TYPE_FLOAT, &gUpVector1Camera.z, "step=0.01");

	TwAddVarRW(CameraGUI, "Sobre Ponto X", TW_TYPE_FLOAT, &gAroundPointCam.x, "step=0.01");
	TwAddVarRW(CameraGUI, "Sobre Ponto Y", TW_TYPE_FLOAT, &gAroundPointCam.y, "step=0.01");
	TwAddVarRW(CameraGUI, "Sobre Ponto Z", TW_TYPE_FLOAT, &gAroundPointCam.z, "step=0.01");

	TwAddVarRW(CameraGUI, "Animação ", TW_TYPE_BOOL8, &gAnimationCam, "step=0.01");

	arbitraryAxisOrientationCam[0] = glm :: vec3(1.0f, 1.0f, 1.0f);
	TwAddVarRW(CameraGUI, "Vetor"  , TW_TYPE_DIR3F, &arbitraryAxisOrientationCam[0], "step=0.01");
	TwAddVarRW(CameraGUI, "Angulo"  , TW_TYPE_FLOAT, &gAnguloCam, "step=0.01");

	//-------INTERFACE PERSPECTIVA-------------------------------------------
	TwSetParam(WorldGUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");

	TwAddVarRW(WorldGUI, "Angle - Fovy", TW_TYPE_FLOAT, &fovy, "step=0.01");
	TwAddVarRW(WorldGUI, "Aspect - A", TW_TYPE_FLOAT, &aspectA, "step=0.01");
	TwAddVarRW(WorldGUI, "Aspect - B", TW_TYPE_FLOAT, &aspectB, "step=0.01");
	TwAddVarRW(WorldGUI, "Near", TW_TYPE_FLOAT, &near, "step=0.01");
	TwAddVarRW(WorldGUI, "Far", TW_TYPE_FLOAT, &far, "step=0.01");

	TwDefine(" Objeto size='250 450' position='10 10' color='255 0 0' text=dark");
	TwDefine(" Camera size='250 400' position='750 350' color='0 255 0' text=dark");
	TwDefine(" Mundo size='250 300' position='750 10' color='0 0 255' text=dark");

	
	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar
 
	glfwSetWindowTitle( "Tabalho 3" );
 
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
	bool res = loadOBJ("f15.obj", vertices, uvs, normals);
 
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
	double asd=1;
	double angulo = 360;
	double anguloRes = 0;
	double frame, r=0;
	float a=-1.5f, b=-2.0f, c=-3.0f;
	int k=0, nbFrames=0;
 
	do{
		double currentTime = glfwGetTime();
		float deltaTime = (float)(currentTime - lastFrameTime); 
		lastFrameTime = currentTime;
		

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
		// Use our shader
		glUseProgram(programID);
	
		glm::mat4 ProjectionMatrix = glm::perspective(fovy, aspectA / aspectB, near, far);

		// Operações para rotação ao redor de um eixo arbitrário
		arbitraryAxisOrientationCam[2].x = arbitraryAxisOrientationCam[1].x - arbitraryAxisOrientationCam[0].x;
		arbitraryAxisOrientationCam[2].y = arbitraryAxisOrientationCam[1].y - arbitraryAxisOrientationCam[0].y;
		arbitraryAxisOrientationCam[2].z = arbitraryAxisOrientationCam[1].z - arbitraryAxisOrientationCam[0].z;
					
		arbitraryAxisOrientationCam[2] = glm::normalize(arbitraryAxisOrientationCam[2]); // Normalização do vetor
						
		glm::mat4 arbitraryAxisMatrizC = mat4(1);
		if(glm::length(arbitraryAxisOrientationCam[2]) > 0.5 && glm::length(arbitraryAxisOrientationCam[2])< 1.5){
			float A = 1 - cos(gAnguloCam);
			arbitraryAxisMatrizC = mat4( // Matriz para rotação em um eixo arbitrário
				vec4(cos(gAnguloCam)+(pow(arbitraryAxisOrientationCam[2].x,2)*A),(arbitraryAxisOrientationCam[2].y * arbitraryAxisOrientationCam[2].x * A +(arbitraryAxisOrientationCam[2].z * sin(gAnguloCam))), (arbitraryAxisOrientationCam[2].z * arbitraryAxisOrientationCam[2].x * A -(arbitraryAxisOrientationCam[2].y * sin(gAnguloCam))), (0)), // coluna
				vec4((arbitraryAxisOrientationCam[2].x * arbitraryAxisOrientationCam[2].y * A - (arbitraryAxisOrientationCam[2].z * sin(gAnguloCam))),(cos(gAnguloCam)+(pow(arbitraryAxisOrientationCam[2].y,2)*A)),(arbitraryAxisOrientationCam[2].z * arbitraryAxisOrientationCam[2].y * A + (arbitraryAxisOrientationCam[2].x * sin(gAnguloCam))),(0)),
				vec4(arbitraryAxisOrientationCam[2].x * arbitraryAxisOrientationCam[2].z * A + (arbitraryAxisOrientationCam[2].y * sin(gAnguloCam)),(arbitraryAxisOrientationCam[2].y * arbitraryAxisOrientationCam[2].z * A - (arbitraryAxisOrientationCam[2].x * sin(gAnguloCam))),(cos(gAnguloCam)+(pow(arbitraryAxisOrientationCam[2].z,2)*A)),(0)),
				vec4((0),(0),(0),(1))
			);
		}

		// Matrizes para rotação em um eixo arbitrário
		glm::mat4 RxC = mat4(
			vec4((1), (0), (0), (0)),
			vec4((0), (cos(gOrientation1Cam.x)), (sin(gOrientation1Cam.x)), (0)),
			vec4((0), (-sin(gOrientation1Cam.x)), (cos(gOrientation1Cam.x)), (0)),
			vec4((0), (0), (0), (1))
		); // Rotação em X
			
		glm::mat4 RyC = mat4(
			vec4((cos(gOrientation1Cam.y)), (0), (-sin(gOrientation1Cam.y)), (0)),
			vec4((0), (1), (0), (0)),
			vec4((sin(gOrientation1Cam.y)), (0), (cos(gOrientation1Cam.y)), (0)),
			vec4((0), (0), (0), (1))
		); // Rotação em Y
			
		glm::mat4 RzC = mat4(
			vec4((cos(gOrientation1Cam.z)), (sin(gOrientation1Cam.z)), (0), (0)),
			vec4((-sin(gOrientation1Cam.z)), (cos(gOrientation1Cam.z)), (0), (0)),
			vec4((0), (0), (1), (0)),
			vec4((0), (0), (0), (1))
		); // Rotação em Z

		glm::mat4 AroundPointXCam = mat4(
										vec4(1, 0, 0, 0),
										vec4(0, cos(gAroundPointCam.x), sin(gAroundPointCam.x), 0),
										vec4(0, -sin(gAroundPointCam.x), cos(gAroundPointCam.x), 0),
										vec4(0, gAroundPointCam.z - (gAroundPointCam.z * cos(gAroundPointCam.x)) + (gAroundPointCam.y * sin(gAroundPointCam.x)), gAroundPointCam.y - (gAroundPointCam.z * sin(gAroundPointCam.x)) - (gAroundPointCam.y * cos(gAroundPointCam.x)), 1)
									);
		glm::mat4 AroundPointYCam = mat4(
										vec4(cos(gAroundPointCam.y), 0, sin(gAroundPointCam.y), 0),
										vec4(0, 1, 0, 0),
										vec4(-sin(gAroundPointCam.y), 0, cos(gAroundPointCam.y), 0),
										vec4(gAroundPointCam.x - (gAroundPointCam.x * cos(gAroundPointCam.y)) + (gAroundPointCam.z * sin(gAroundPointCam.y)), 0, gAroundPointCam.z - (gAroundPointCam.x * sin(gAroundPointCam.y)) - (gAroundPointCam.z * cos(gAroundPointCam.y)), 1)
									);
		glm::mat4 AroundPointZCam = mat4(
										vec4(cos(gAroundPointCam.z), sin(gAroundPointCam.z), 0, 0),
										vec4(-sin(gAroundPointCam.z), cos(gAroundPointCam.z), 0, 0),
										vec4(0, 0, 1, 0),
										vec4(gAroundPointCam.x - (gAroundPointCam.x * cos(gAroundPointCam.z)) + (gAroundPointCam.y * sin(gAroundPointCam.z)), gAroundPointCam.y - (gAroundPointCam.x * sin(gAroundPointCam.z)) - (gAroundPointCam.y * cos(gAroundPointCam.z)), 0, 1)
									);

		glm::mat4 ViewMatrix = glm::lookAt(
			glm::vec3( gPosition1Camera.x,  gPosition1Camera.y,  gPosition1Camera.z), // Camera is here
			glm::vec3( gLook1Camera.x, gLook1Camera.y, gLook1Camera.z), // and looks here
			glm::vec3( gUpVector1Camera.x, gUpVector1Camera.y, gUpVector1Camera.z)  // Head is up (set to 0,-1,0 to look upside-down)
		);

		if( glfwGetKey( GLFW_KEY_UP)) gAnimationCam = true;
		else if( glfwGetKey( GLFW_KEY_DOWN)) gAnimationCam = false;

		
		if(gAnimationCam){
			nbFrames++;
			if(k==0) {
				gPosition1.z -= 10;
				if(gPosition1Camera.z < 50)	gPosition1Camera.z = 50;
				//gPosition1Camera.z = 10;
				k=1;
			}
			//printf("Asd-> %.2lf\n", asd);
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			printf("%f DELTA ms/frame\n", deltaTime);
			
			frame = 1000.0/double(nbFrames);
			anguloRes = nbFrames / angulo;
			//r = frame * anguloRes;

			glm::mat4 RxCA = mat4(
				vec4((1), (0), (0), (0)),
				vec4((0), (cos(anguloRes)), (sin(anguloRes)), (0)),
				vec4((0), (-sin(anguloRes)), (cos(anguloRes)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em X
		
			glm::mat4 RyCA = mat4(
				vec4((cos(anguloRes)), (0), (-sin(anguloRes)), (0)),
				vec4((0), (1), (0), (0)),
				vec4((sin(anguloRes)), (0), (cos(anguloRes)), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Y

			glm::mat4 RzCA = mat4(
				vec4((cos(anguloRes)), (sin(anguloRes)), (0), (0)),
				vec4((-sin(anguloRes)), (cos(anguloRes)), (0), (0)),
				vec4((0), (0), (1), (0)),
				vec4((0), (0), (0), (1))
			); // Rotação em Z

			asd += 0.01;
			ViewMatrix = ViewMatrix * RzCA * RyCA * RxCA;


			if ( currentTime - lastTime >= 60*5){ // If last prinf() was more than 1sec ago
				// printf and reset
				//printf("%f ms/frame\n", 1000.0/double(nbFrames));

				nbFrames = 0;
				lastTime += 1.0;

			}
		}

		ViewMatrix = ViewMatrix * arbitraryAxisMatrizC * RxC * RyC * RzC * AroundPointXCam * AroundPointYCam * AroundPointZCam;

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
				ModelMatrix = TranslationInversa * RotationMatrix * TranslationMatrix * ScalingMatrix * ModelMatrix* ShearMatrix * arbitraryAxisMatriz;
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