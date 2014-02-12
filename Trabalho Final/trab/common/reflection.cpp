#include "reflection.h"

Reflection::Reflection(double width, double height, glm::mat4 transformation){
	this->width = width;
	this->height = height;
	this->transformation = transformation;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	double w = width/2.0;
	double h = height/2.0;

	vertices.insert(vertices.end(), glm::vec3(-w,-h,0));
	vertices.insert(vertices.end(), glm::vec3(w,-h,0));
	vertices.insert(vertices.end(), glm::vec3(-w,h,0));
	vertices.insert(vertices.end(), glm::vec3(w,-h,0));
	vertices.insert(vertices.end(), glm::vec3(w,h,0));
	vertices.insert(vertices.end(), glm::vec3(-w,h,0));

	uvs.insert(uvs.end(), glm::vec2(0,0));
	uvs.insert(uvs.end(), glm::vec2(1,0));
	uvs.insert(uvs.end(), glm::vec2(0,1));
	uvs.insert(uvs.end(), glm::vec2(1,0));
	uvs.insert(uvs.end(), glm::vec2(1,1));
	uvs.insert(uvs.end(), glm::vec2(0,1));
	
	for(int i=0; i<6; i++) normals.insert(normals.end(), glm::vec3(0,0,1));

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

	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// The texture we're going to render to
	glGenTextures(1, &renderedTexture);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

	// Set the list of draw buffers.
	GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers); // "1" is the size of DrawBuffers

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Reflection::setCamera(glm::vec3 position, glm::vec3 direction, glm::vec3 up){
	double w = width/2.0, h = height/2.0;
	
	glm::vec4 p1 = transformation*glm::vec4(-w,-h,0,0);
	glm::vec4 p2 = transformation*glm::vec4(w,-h,0,0);
	glm::vec4 p3 = transformation*glm::vec4(-w,h,0,0);
	
	glm::vec4 v1, v2, n;

    v1.x = p2.x - p1.x;
    v1.y = p2.y - p1.y;
    v1.z = p2.z - p1.z;

    v2.x = p3.x - p1.x;
    v2.y = p3.y - p1.y;
    v2.z = p3.z - p1.z;

    n.x = (v1.y * v2.z) - (v1.z * v2.y);
    n.y = (v1.z * v2.x) - (v1.x * v2.z);
    n.z = (v1.x * v2.y) - (v1.y * v2.x);

	n /= sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
	
	double a = n.x, b= n.y, c = n.z;
	double d = p1.x*n.x+p1.y*n.y+p1.z*n.z;

	glm::mat4 reflectionMatrix( 
		1-2*a*a, -2*a*b, -2*a*c, 2*d*a,
		-2*a*b, 1-2*b*b, -2*b*c, 2*d*b,
		-2*a*c, -2*b*c, 1-2*c*c, 2*d*c,
		0,0,0,1
	);

	glm::vec4 vect = glm::vec4(position, 0) - (p2+p3)/2.0;
	
	double dist = sqrt(vect.x*vect.x + vect.y*vect.y + vect.z*vect.z);
	double fov = 1.0/dist*300+1.5;

	projectionMatrix = glm::perspective((float)fov, 4.0f / 3.0f, 0.1f, 100.0f);
	viewMatrix = (glm::mat4)glm::scale(-1,1,1);
	viewMatrix *= glm::lookAt(
		(glm::vec3)(glm::vec4(position, 0)*reflectionMatrix), 
		(glm::vec3)(glm::vec4(direction,0)*reflectionMatrix), 
		(glm::vec3)(glm::vec4(up,0)*reflectionMatrix)
	);

}

void Reflection::draw(){

	glBindTexture(GL_TEXTURE_2D, renderedTexture);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

}

void Reflection::init(){
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFrontFace(GL_CW);
}

void Reflection::finish(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glFrontFace(GL_CCW);
}