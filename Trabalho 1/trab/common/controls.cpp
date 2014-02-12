// Include GLFW
#include <GL/glfw.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::vec3 direction;
glm::vec3 up;
float FoV;
glm::vec3 position = glm::vec3( 0, 0, 10 ); 

glm::vec3 getDirection(){
	return direction;
}
glm::vec3 getUp(){
	return up;
}
glm::vec3 getPosition(){
	return position;
}

glm::mat4 getViewMatrix(){
	return glm::lookAt(
			position,           // Camera is here
			direction, // and looks here : at the same position, plus "direction"
			up                  // Head is up (set to 0,-1,0 to look upside-down)
	 );
}
glm::mat4 getProjectionMatrix(){
	return glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
}

// Initial Field of View
float initialFoV = 45.0f;


void computeMatricesFromInputs(){
	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = vec3(0, 0, 0);
	
	// Up vector
	up = vec3(0, 1, 0);

	FoV = initialFoV; 
}