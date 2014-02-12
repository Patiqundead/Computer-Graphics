/*
Universidade Federal da Fronteira Sul
Trabalho 1 de Computação Gráfica
Alunos: Doglas André Finco
		Patrick De Bastiani


Objective:
• Implement a simple mesh simplification
	- Vertex removal
	- Priority queue
*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

int main( void )
{	
	srand (time(NULL));
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

	glfwSetWindowTitle( "Trabalho 1" );

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );
	glfwSetMousePos(1024/2, 768/2);

	// Dark blue background
	glClearColor(0.4f, 0.4f, 0.4f, 0.4f);

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
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Load the texture
	GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals; // Won't be used at the moment.
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	// Load it into a VBO
	GLuint vertexbuffer;
	GLuint uvbuffer;

	std::vector< std::vector<glm::vec3> > novo;
	
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
		
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	
	do{
		//system("PAUSE");
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //mostra os triangulos	
		
		std::vector<glm::vec3> aux;
		glm::vec3 verticesNew;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
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

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers();

		if(glfwGetKey( GLFW_KEY_PAGEDOWN ) == GLFW_PRESS){
			if(vertices.size() > 0){
				int mais = vertices.size()-1; //O vértice a ser removido é sempre último do vetor de vértices
				int escolhido = mais-1;//vértice que será ligado aos outros vértices para a reantrigulação
				
				//salva as coordenadas do vértice escolhido
				float x = vertices[mais].x;
				float y = vertices[mais].y;
				float z = vertices[mais].z;

				novo.push_back(vertices);//vetor de vértices é copiado para o novo

				for(int i = 0; i < vertices.size(); i++){//retriangulação
					if(vertices[i].x == x && vertices[i].y == y && vertices[i].z == z){
						vertices[i].x = vertices[escolhido].x; 
						vertices[i].y = vertices[escolhido].y;
						vertices[i].z = vertices[escolhido].z;
					}
				}

				for(int i = 0; i < vertices.size(); i+=3){//Teste para remoção de retas e pontos, pois não precisam ser desenhadas novamente e não interferem
					if(vertices[i].x != vertices[i+1].x || vertices[i].y != vertices[i+1].y || vertices[i].z != vertices[i+1].z){
						if(vertices[i].x != vertices[i+2].x || vertices[i].y != vertices[i+2].y || vertices[i].z != vertices[i+2].z){
							if(vertices[i+1].x != vertices[i+2].x || vertices[i+1].y != vertices[i+2].y || vertices[i+1].z != vertices[i+2].z){
								verticesNew.x=vertices[i].x;
								verticesNew.y=vertices[i].y;
								verticesNew.z=vertices[i].z;
								aux.push_back(verticesNew);
								verticesNew.x=vertices[i+1].x;
								verticesNew.y=vertices[i+1].y;
								verticesNew.z=vertices[i+1].z;		
								aux.push_back(verticesNew);
								verticesNew.x=vertices[i+2].x;
								verticesNew.y=vertices[i+2].y;
								verticesNew.z=vertices[i+2].z;		
								aux.push_back(verticesNew);
							}
						}
					}
				}
				if(vertices.size() > 0){//Enquanto houverem vértices irá remover
					vertices = aux;
				}

				if(vertices.size() > 0){
					glGenBuffers(1, &vertexbuffer);
					glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
					glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
				}
		
				glGenBuffers(1, &uvbuffer);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
				glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
			}
		}
		if(glfwGetKey( GLFW_KEY_PAGEUP) == GLFW_PRESS){ // reconstruo
			if(novo.size() > 0){ 
				vertices = novo[novo.size()-1];
				
				if(vertices.size() > 0){
					glGenBuffers(1, &vertexbuffer);
					glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
					glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
				}
		
				glGenBuffers(1, &uvbuffer);
				glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
				glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
				novo.pop_back();//retira uma posição do novo
			}
		}
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS && glfwGetWindowParam( GLFW_OPENED ));

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

