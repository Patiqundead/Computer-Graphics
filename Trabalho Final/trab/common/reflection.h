#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>
#include <vector>
#include <common/vboindexer.hpp>
#include <stdio.h>


class Reflection {
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint normalbuffer;
	GLuint elementbuffer;
	GLuint frameBuffer;
	GLuint depthRenderBuffer;
	GLuint renderedTexture;
	std::vector<unsigned short> indices;
public:
	double width, height;
	glm::mat4 transformation;
	Reflection(double width, double height, glm::mat4 transformation);
	void draw();
	void init();
	void finish();
	void setCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up);
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
};