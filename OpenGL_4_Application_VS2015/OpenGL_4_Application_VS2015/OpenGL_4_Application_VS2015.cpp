//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
glm::vec3 lightColorNight;
glm::vec3 lightColorDay;
GLuint lightColorLoc;

//Starting point camera position
gps::Camera myCamera(glm::vec3(1.5f, 3.0f, 135.5f), glm::vec3(0.0f, 3.0f, 3.0f));
GLfloat cameraSpeed = 0.35f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 views = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle;

GLfloat miscaMasina = 0.0f;
GLfloat adaptMiscaMasina = 0.0f;
GLfloat miscaAvionul = 0.0f;

gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

gps::Model3D house2;
gps::Model3D grass;
gps::Model3D asfalt;
gps::Model3D casa;
gps::Model3D masinuta;
gps::Model3D gardut;
gps::Model3D barn;
gps::Model3D woodenBarn;
gps::Model3D copac;
gps::Model3D biserica;
gps::Model3D houseStone;
gps::Model3D plane;
gps::Model3D stalp;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	
	lightShader.useShaderProgram();
	
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

bool merge = false;
void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f; 
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}	

	//misca masinuta in fata
	if (pressedKeys[GLFW_KEY_B]) {
		miscaMasina += 0.1f;
		adaptMiscaMasina += 0.070f;	
	}

	//misca masinuta in spate
	if (pressedKeys[GLFW_KEY_N]) {
		miscaMasina -= 0.1f;
		adaptMiscaMasina -= 0.070f;
	}

	//misca avionul in fata
	if (pressedKeys[GLFW_KEY_P]) {
		merge = true;	
	}
	if (merge)
		miscaAvionul += 0.7f;

	if (pressedKeys[GLFW_KEY_Y]) {
		glm::vec3 posi = myCamera.getCameraPosition();
		glm::vec3 dire = myCamera.getCameraDirection();
		std::cout <<"camera position =  "<< posi.x <<"   "<< posi.y <<"   "<< posi.z << "\n";
		std::cout <<"camera direction =  "<< dire.x << "   " << dire.y << "   " << dire.z << "\n";
	}

	//schimbam lumina din zi in noapte
	if (pressedKeys[GLFW_KEY_R]) {
		myCustomShader.useShaderProgram();
		lightColorNight = glm::vec3(0.01f, 0.01f, 0.01f); //NIGHT LIGHT modified by me to make it sunset									
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorNight));
	}

	//schimbam lumina din noapte in zi
	if (pressedKeys[GLFW_KEY_T]) {
		myCustomShader.useShaderProgram();
		lightColorDay = glm::vec3(1.0f, 1.0f, 1.0f); //DAY LIGHT modified by me to make it sunset									
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColorDay));
	}

	//wireframe
	if (pressedKeys[GLFW_KEY_F]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	//points
	if (pressedKeys[GLFW_KEY_G]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	//back to initials (normal objects form)
	if (pressedKeys[GLFW_KEY_H]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouse_callback);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 60.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(20.0f * lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));  //0.0 1.0 0.0

	return lightProjection * lightView;
}

void initModels()
{
	//myModel = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	house2 = gps::Model3D("objects/House Complex/House Complex.obj", "objects/House Complex/");
	grass = gps::Model3D("objects/Grass/10450_Rectangular_Grass_Patch_v1_iterations-2.obj", "objects/Grass/");
	asfalt = gps::Model3D("objects/rua para blender/untitled.obj", "objects/rua para blender/");
	casa = gps::Model3D("objects/housefinally/Models and Textures/house.obj", "objects/housefinally/Models and Textures/");
	masinuta = gps::Model3D("objects/BMW850/BMW850.obj", "objects/BMW850/");
	gardut = gps::Model3D("objects/Wooden_Post/Wooden_Post.obj", "objects/Wooden_Post/");
	barn = gps::Model3D("objects/RuralStallObj/RuralStall.obj", "objects/RuralStallObj/");
	woodenBarn = gps::Model3D("objects/Wooden/WoodenCabinObj.obj", "objects/Wooden/");
	copac = gps::Model3D("objects/Tree 02/Tree.obj", "objects/Tree 02/");
	biserica = gps::Model3D("objects/Biserica/chapel_obj.obj", "objects/Biserica/");
	houseStone = gps::Model3D("objects/HouseStone/house_obj.obj", "objects/HouseStone/");
	plane = gps::Model3D("objects/Great_Lakes_Biplane/Hi Poly/GreatLakesBiplaneHP.obj", "objects/Great_Lakes_Biplane/Hi Poly/");
	stalp = gps::Model3D("objects/StLamp/STLamp.obj", "objects/StLamp/");
	
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	
	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f); //0.0f, 1.0f, 0.0f
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //DAY LIGHT original
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//SKYBOX
	std::vector<const GLchar*> faces;
	faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

}

void setReflector()
{
	//lumina spotlight difusse
	myCustomShader.useShaderProgram();
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.position"), 15.96f, 9.95f, 22.64f);
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.direction"), 0.00f, -0.26f, 0.1f);
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.ambient"), 0.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.diffuse"), 1.0f, 1.0f, 0.3f);  //yellow
	glUniform3f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.specular"), 0.1f, 0.1f, 0.1f);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.cutOff"), glm::cos(glm::radians(7.5f)));
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "felinar.outerCutOff"), glm::cos(glm::radians(13.0f)));
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();	

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwMakeContextCurrent(glWindow);
	glfwSetFramebufferSizeCallback(glWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(glWindow, mouse_callback);
	glfwSetScrollCallback(glWindow, scroll_callback);

	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	//FIRST---------------------------------------------------------------------------------------------
	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
		
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	/**
	//create model matrix for nanosuit
	model = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 1.0f, -10.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1, 0, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	myModel.Draw(depthMapShader);
	**/


	//create model matrix for grass
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::scale(model, glm::vec3(1.3f, 1.3f, 0.0082f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	grass.Draw(depthMapShader);


	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(50.0f, 1.0f, 50.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"), 
						1, 
						GL_FALSE, 
						glm::value_ptr(model));

	ground.Draw(depthMapShader);


	//create model matrix for house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-38.0f, 0.00f, -13.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for road	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -1.0f, 15.0f));
	model = glm::scale(model, glm::vec3(12.0f, 1.0f, 3.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	asfalt.Draw(depthMapShader);


	//create model matrix for CASA	Axa X inversata cu Z din cauza rotatiei.
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-13.0f, 0.0f, -29.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	casa.Draw(depthMapShader);


	//create model matrix for Masinuta
	model = glm::rotate(glm::mat4(1.0f), glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-60.0f + miscaMasina, 0.15f, -12.0f + adaptMiscaMasina));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	masinuta.Draw(depthMapShader);


	//create model matrix for Gardut
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-160.0f, -180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	gardut.Draw(depthMapShader);


	//create model matrix for Gardut2 Y intors cu Z ii ala din stanga
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-160.0f, 180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	gardut.Draw(depthMapShader);


	//create model matrix for road latitudinal stanga adancime
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-98.5f, -0.9f, 0.3f));
	model = glm::scale(model, glm::vec3(6.3f, 1.0f, 3.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	asfalt.Draw(depthMapShader);


	//create model matrix for road latitudinal drapta plan apropiat
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(101.8f, -0.9f, 0.3f));
	model = glm::scale(model, glm::vec3(5.5f, 1.0f, 3.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	asfalt.Draw(depthMapShader);


	//create model matrix for Barn
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-160.0f, -1.0f, -70.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for Wooden Barn
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-95.0f, 0.0f, -30.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	woodenBarn.Draw(depthMapShader);


	//create model matrix for Copac
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-80.0f, 0.0f, -50.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);

	//create model matrix for Biserica
	model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 48.0f));
	model = glm::scale(model, glm::vec3(0.03f, 0.025f, 0.025f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	biserica.Draw(depthMapShader);


	//create model matrix for house plan 1 the second house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-38.0f, 0.00f, -55.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Copac 2 Plan 1
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, -95.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for House Stone
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-25.0f, 0.0f, 35.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	houseStone.Draw(depthMapShader);


	//create model matrix for House Stone Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(60.0f, 0.0f, 105.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	houseStone.Draw(depthMapShader);


	//create model matrix for house plan 4 the second house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(48.0f, 0.00f, 165.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Plane
	model = glm::translate(glm::mat4(1.0f), glm::vec3(28.0f, 30.0f, -135.0f + miscaAvionul));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	plane.Draw(depthMapShader);


	//create model matrix for CASA Plan 1	Axa X inversata cu Z din cauza rotatiei.
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-13.0f, 0.0f, 155.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	casa.Draw(depthMapShader);


	//create model matrix for Copac 3 Plan 1
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-130.0f, 0.0f, -18.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for house plan 1 the third house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-70.0f, 0.00f, -145.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for house plan 1 the forth house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-140.0f, 0.00f, -145.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Wooden Barn Second, Plane 1
	model = glm::translate(glm::mat4(1.0f), glm::vec3(160.0f, 0.0f, -95.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	woodenBarn.Draw(depthMapShader);


	//create model matrix for Copac 4 Plan 1
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-105.0f, 0.0f, -110.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for CASA 2 Plan 1	Axa X inversata cu Z din cauza rotatiei.
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-75.0f, 0.0f, 100.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	casa.Draw(depthMapShader);


	//create model matrix for house plan 4 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-110.0f, 0.00f, 60.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for house 2 plan 4 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-110.0f, 0.00f, 130.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Barn Plane 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-170.0f, -1.0f, 90.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for CASA 2 Plan 3	Axa X inversata cu Z din cauza rotatiei.
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(100.0f, 0.0f, 40.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	casa.Draw(depthMapShader);


	//create model matrix for Copac 1 Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-60.0f, 0.0f, 150.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for house plan 4 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 0.0f, 50.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Wooden Barn Second, Plane 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 155.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	woodenBarn.Draw(depthMapShader);


	//create model matrix for Copac Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-160.0f, 0.0f, 165.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for Copac 4 Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-170.0f, 0.0f, 40.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for house plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 130.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Barn Plane 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 170.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for Barn 2 Plane 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 140.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for Copac Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(140.0f, 0.0f, 90.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for Copac 2 Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(90.0f, 0.0f, 70.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for house plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, -20.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for house 2 plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 0.0f, -140.0f));
	model = glm::scale(model, glm::vec3(0.085f, 0.085f, 0.085f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	house2.Draw(depthMapShader);


	//create model matrix for Barn Plane 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(85.0f, -1.0f, -30.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for CASA Plan 2	Axa X inversata cu Z din cauza rotatiei.
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-90.0f, 0.0f, -90.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	casa.Draw(depthMapShader);


	//create model matrix for House Stone Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, -140.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	houseStone.Draw(depthMapShader);


	//create model matrix for Barn Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -1.0f, -75.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	barn.Draw(depthMapShader);


	//create model matrix for Copac Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(55.0f, 0.0f, -60.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for Copac 2 Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 0.0f, -170.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for Copac 3 Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(125.0f, 0.0f, -80.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	copac.Draw(depthMapShader);


	//create model matrix for Gardut plane 3 
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(47.0f, -180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	gardut.Draw(depthMapShader);


	//create model matrix for Gardut2 Y intors cu Z ii ala din stanga plane 4
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(47.0f, 180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	gardut.Draw(depthMapShader);

	//create model matrix for Stalp
	model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, 25.0f));
	model = glm::scale(model, glm::vec3(2.3f, 2.3f, 2.3f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	stalp.Draw(depthMapShader);



	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//mySkyBox.Draw(skyboxShader, view, projection);










	//SECOND----------------------------------------------------------------------------------
	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));	

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	
	/**
	//create model matrix for nanosuit
	model = glm::translate(glm::mat4(1.0f), glm::vec3(6.0f, 1.0f, 10.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myModel.Draw(myCustomShader);
	**/

			
	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(50.0f, 1.0f, 50.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	ground.Draw(myCustomShader);


	//create model matrix for house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-38.0f, 0.00f, -13.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for grass
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-6.0f, 0.0f, 0.0f));
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));
	model = glm::scale(model, glm::vec3(1.3f, 1.3f, 0.0082f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	grass.Draw(myCustomShader);


	//create model matrix for road
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, -1.0f, 15.0f));
	model = glm::scale(model, glm::vec3(12.0f, 1.0f, 3.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	asfalt.Draw(myCustomShader);

	//create model matrix for CASA
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-13.0f, 0.0f, -29.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	casa.Draw(myCustomShader);


	//create model matrix for Masinuta
	model = glm::rotate(glm::mat4(1.0f), glm::radians(35.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-60.0f + miscaMasina, 0.15f, -12.0f + adaptMiscaMasina));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	masinuta.Draw(myCustomShader);


	//create model matrix for Gardut
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-160.0f, -180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	gardut.Draw(myCustomShader);


	//create model matrix for Gardut2 ii ala din stanga
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(-160.0f, 180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	gardut.Draw(myCustomShader);


	//create model matrix for road latitudinal stanga adancime
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-98.5f, -0.9f, 0.3f));
	model = glm::scale(model, glm::vec3(6.3f, 1.0f, 3.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	asfalt.Draw(myCustomShader);


	//create model matrix for road latitudinal dreapta plan apropiat
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(101.8f, -0.9f, 0.3f));
	model = glm::scale(model, glm::vec3(5.5f, 1.0f, 3.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	asfalt.Draw(myCustomShader);


	//create model matrix for Barn
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-160.0f, -1.0f, -70.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	barn.Draw(myCustomShader);


	//create model matrix for Wooden Barn
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-95.0f, 0.0f, -30.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for Copac
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-80.0f, 0.0f, -50.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for Biserica
	model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, 48.0f));
	model = glm::scale(model, glm::vec3(0.03f, 0.025f, 0.025f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	biserica.Draw(myCustomShader);


	//create model matrix for house plan 1 the second house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-38.0f, 0.00f, -55.0f)); 
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Copac 2 Plan 1 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, -95.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for House Stone Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-25.0f, 0.0f, 35.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	houseStone.Draw(myCustomShader);


	//create model matrix for House Stone Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(60.0f, 0.0f, 105.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	houseStone.Draw(myCustomShader);


	//create model matrix for house plan 4 the second house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(48.0f, 0.00f, 165.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Plane
	model = glm::translate(glm::mat4(1.0f), glm::vec3(28.0f, 30.0f, -135.0f + miscaAvionul));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	plane.Draw(myCustomShader);

	//create model matrix for CASA Plan 1
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-13.0f, 0.0f, 155.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	casa.Draw(myCustomShader);


	//create model matrix for Copac 3 Plan 1 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-130.0f, 0.0f, -18.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for house plan 4 the third house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-70.0f, 0.00f, -145.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for house plan 4 the forth house
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-140.0f, 0.00f, -145.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Wooden Barn
	model = glm::translate(glm::mat4(1.0f), glm::vec3(160.0f, 0.0f, -95.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for Copac 4 Plan 1 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-105.0f, 0.0f, -110.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);

	//create model matrix for CASA 2 Plan 1
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-75.0f, 0.0f, 100.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	casa.Draw(myCustomShader);


	//create model matrix for house plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-110.0f, 0.00f, 60.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for house 2 plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-110.0f, 0.00f, 130.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Barn Plane 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-170.0f, -1.0f, 90.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	barn.Draw(myCustomShader);


	//create model matrix for CASA Plan 3
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(100.0f, 0.0f, 40.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	casa.Draw(myCustomShader);
	

	//create model matrix for Copac 1 Plan 4 
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-60.0f, 0.0f, 150.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for house 2 plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 0.0f, 50.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Wooden Barn Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 155.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for Copac Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-160.0f, 0.0f, 165.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for Copac Plan 3
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-170.0f, 0.0f, 40.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for house plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 130.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Wooden Barn Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 170.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for Wooden Barn 2 Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 140.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for Copac Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(140.0f, 0.0f, 90.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for Copac Plan 4
	model = glm::translate(glm::mat4(1.0f), glm::vec3(90.0f, 0.0f, 70.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for house plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, -20.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for house 2 plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(130.0f, 0.0f, -140.0f));
	model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	house2.Draw(myCustomShader);


	//create model matrix for Wooden Barn Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(85.0f, 0.0f, -30.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	woodenBarn.Draw(myCustomShader);


	//create model matrix for CASA Plan 2 X inversat cu Z
	model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
	model = glm::translate(model, 1.0f * glm::vec3(-90.0f, 0.0f, -90.0f));
	model = glm::scale(model, glm::vec3(0.9f, 0.9f, 0.9));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	casa.Draw(myCustomShader);


	//create model matrix for House Stone Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 0.0f, -140.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	houseStone.Draw(myCustomShader);


	//create model matrix for Barn Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, -1.0f, -75.0f));
	model = glm::scale(model, glm::vec3(0.015f, 0.015f, 0.015f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	barn.Draw(myCustomShader);


	//create model matrix for Copac Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(55.0f, 0.0f, -60.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for Copac 2 Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(35.0f, 0.0f, -170.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	copac.Draw(myCustomShader);
	//create model matrix for Copac 2 Plan 2
	model = glm::translate(glm::mat4(1.0f), glm::vec3(125.0f, 0.0f, -80.0f));
	model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	copac.Draw(myCustomShader);


	//create model matrix for Gardut plan 3
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(47.0f, -180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	gardut.Draw(myCustomShader);


	//create model matrix for Gardut2 ii ala din stanga plan 4
	model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, 1.0f * glm::vec3(47.0f, 180.0f, -1.3f));
	model = glm::scale(model, glm::vec3(0.67f, 0.02f, 0.02));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	gardut.Draw(myCustomShader);

	//create model matrix for Stalp
	model = glm::translate(glm::mat4(1.0f), glm::vec3(15.0f, 0.0f, 25.0f));
	model = glm::scale(model, glm::vec3(2.3f, 2.3f, 2.3f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	stalp.Draw(myCustomShader);

	

	//draw a white cube around the light

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, 1.0f * lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);

	mySkyBox.Draw(skyboxShader, view, projection);

}

int main(int argc, const char * argv[]) {

	
	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();	
	setReflector();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
