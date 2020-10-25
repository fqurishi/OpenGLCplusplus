#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL2\soil2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <random>
#include <glm\gtc\type_ptr.hpp> // glm::value_ptr
#include <glm\gtc\matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Utils.h"
#include "ImportedModel.h"
#include "Camera.h"
using namespace std;

void passOne(void);
void passTwo(void);
void passSkyBox(void);

float toRadians(float degrees) { return (degrees * 2.0f * 3.14159f) / 360.0f; }

#define numVAOs 1
#define numVBOs 10

Utils util = Utils();
//screen
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
//camera
Camera *camera = new Camera(0.0f, 0.0f, 35.0f);
Camera *gunCamera = new Camera(-0.04f, 0.0f, -34.8f);
double lastX = SCR_WIDTH / 2.0f;
double lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


GLuint renderingProgramShadows, renderingProgram, renderingProgramCubeMap, renderingProgramEnvironment,
renderingProgramBump;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];





//objects and models
ImportedModel ground("objects/grid.obj");
ImportedModel building("objects/building1.obj");
ImportedModel gun("objects/gun1.obj");
int numGroundVertices;
int numBuildingVertices;
int numGunVertices;
//locations
glm::vec3 lightLoc(0.0f, 60.0f, -50.0f);
float terLocX, terLocY, terLocZ;
float buildingLocX, buildingLocY, buildingLocZ;
float gunLocX, gunLocY, gunLocZ;
float tessInner = 30.0f;
float tessOuter = 20.0f;

// white light
float globalAmbient[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// gold material
float* gMatAmb = Utils::goldAmbient();
float* gMatDif = Utils::goldDiffuse();
float* gMatSpe = Utils::goldSpecular();
float gMatShi = Utils::goldShininess();

//silver materal
// bronze material
float* sMatAmb = Utils::silverAmbient();
float* sMatDif = Utils::silverDiffuse();
float* sMatSpe = Utils::silverSpecular();
float sMatShi = Utils::silverShininess();

// bronze material
float* bMatAmb = Utils::bronzeAmbient();
float* bMatDif = Utils::bronzeDiffuse();
float* bMatSpe = Utils::bronzeSpecular();
float bMatShi = Utils::bronzeShininess();

// zero material
float* zMatAmb = Utils::zeroAmbient();
float* zMatDif = Utils::zeroDiffuse();
float* zMatSpe = Utils::zeroSpecular();
float zMatShi = Utils::zeroShininess();

float thisAmb[4], thisDif[4], thisSpe[4], matAmb[4], matDif[4], matSpe[4];
float thisShi, matShi;

// shadow stuff
int scSizeX, scSizeY;
GLuint shadowTex, shadowBuffer;
glm::mat4 lightVmatrix;
glm::mat4 lightPmatrix;
glm::mat4 shadowMVP1;
glm::mat4 shadowMVP2;
glm::mat4 b;

//textures
GLuint skyboxTexture;
GLuint blackTexture;
GLuint whiteTexture;
GLuint grassyTexture;
GLuint grassyTextureNM;
GLuint grassyTextureHM;

//variable for functions
float rotAmt = 0.0f;
float playerSpeed = 0.25f;

// variable allocation for display
GLuint vLoc, mvLoc, projLoc, nLoc, sLoc;
int width, height;
float aspect;
glm::mat4 pMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos, transformed;
float lightPos[3];
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;
glm::vec3 origin(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

//noise stuff
GLuint noiseTexture;
const int noiseHeight = 300;
const int noiseWidth = 300;
const int noiseDepth = 300;
double noise[noiseHeight][noiseWidth][noiseDepth];


// 3D Noise Texture section

double smoothNoise(double x1, double y1, double z1) {
	//get fractional part of x, y, and z
	double fractX = x1 - (int)x1;
	double fractY = y1 - (int)y1;
	double fractZ = z1 - (int)z1;

	//neighbor values
	int x2 = ((int)x1 + noiseWidth + 1) % noiseWidth;
	int y2 = ((int)y1 + noiseHeight + 1) % noiseHeight;
	int z2 = ((int)z1 + noiseDepth + 1) % noiseDepth;

	//smooth the noise by interpolating
	double value = 0.0;
	value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * noise[(int)x1][(int)y1][(int)z1];
	value += (1 - fractX) * fractY * (1 - fractZ) * noise[(int)x1][(int)y2][(int)z1];
	value += fractX * (1 - fractY) * (1 - fractZ) * noise[(int)x2][(int)y1][(int)z1];
	value += fractX * fractY * (1 - fractZ) * noise[(int)x2][(int)y2][(int)z1];

	value += (1 - fractX) * (1 - fractY) * fractZ * noise[(int)x1][(int)y1][(int)z2];
	value += (1 - fractX) * fractY * fractZ * noise[(int)x1][(int)y2][(int)z2];
	value += fractX * (1 - fractY) * fractZ * noise[(int)x2][(int)y1][(int)z2];
	value += fractX * fractY * fractZ * noise[(int)x2][(int)y2][(int)z2];

	return value;
}

double turbulence(double x, double y, double z, double size) {
	double value = 0.0, initialSize = size;
	while (size >= 0.9) {
		value = value + smoothNoise(x / size, y / size, z / size) * size;
		size = size / 2.0;
	}
	value = 128.0 * value / initialSize;
	return value;
}

double logistic(double x) {
	double k = 3.0;
	return (1.0 / (1.0 + pow(2.718, -k * x)));
}

void fillDataArray(GLubyte data[]) {
	double veinFrequency = 1.75;
	double turbPower = 3.0;  //4
	double turbSize = 32.0;
	for (int i = 0; i < noiseHeight; i++) {
		for (int j = 0; j < noiseWidth; j++) {
			for (int k = 0; k < noiseDepth; k++) {
				double xyzValue = (float)i / noiseWidth + (float)j / noiseHeight + (float)k / noiseDepth
					+ turbPower * turbulence(i, j, k, turbSize) / 256.0;

				double sineValue = logistic(abs(sin(xyzValue * 3.14159 * veinFrequency)));
				sineValue = max(-1.0, min(sineValue * 1.25 - 0.20, 1.0));

				float redPortion = 255.0f * (float)sineValue;
				float greenPortion = 255.0f * (float)min(sineValue * 1.5 - 0.25, 1.0);
				float bluePortion = 255.0f * (float)sineValue;

				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 0] = (GLubyte)redPortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 1] = (GLubyte)greenPortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 2] = (GLubyte)bluePortion;
				data[i * (noiseWidth * noiseHeight * 4) + j * (noiseHeight * 4) + k * 4 + 3] = (GLubyte)255;
			}
		}
	}
}

GLuint buildNoiseTexture() {
	GLuint textureID;
	GLubyte* data = new GLubyte[noiseHeight * noiseWidth * noiseDepth * 4];

	fillDataArray(data);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_3D, textureID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, noiseWidth, noiseHeight, noiseDepth);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, noiseWidth, noiseHeight, noiseDepth, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, data);

	return textureID;
}

void generateNoise() {
	for (int x = 0; x < noiseHeight; x++) {
		for (int y = 0; y < noiseWidth; y++) {
			for (int z = 0; z < noiseDepth; z++) {
				noise[x][y][z] = (double)rand() / (RAND_MAX + 1.0);
			}
		}
	}
}

void installLights(int renderingProgram, glm::mat4 vMatrix) {
	transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
	lightPos[0] = transformed.x;
	lightPos[1] = transformed.y;
	lightPos[2] = transformed.z;

	matAmb[0] = thisAmb[0]; matAmb[1] = thisAmb[1]; matAmb[2] = thisAmb[2]; matAmb[3] = thisAmb[3];
	matDif[0] = thisDif[0]; matDif[1] = thisDif[1]; matDif[2] = thisDif[2]; matDif[3] = thisDif[3];
	matSpe[0] = thisSpe[0]; matSpe[1] = thisSpe[1]; matSpe[2] = thisSpe[2]; matSpe[3] = thisSpe[3];
	matShi = thisShi;

	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
	diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
	specLoc = glGetUniformLocation(renderingProgram, "light.specular");
	posLoc = glGetUniformLocation(renderingProgram, "light.position");
	mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
	mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
	mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
	mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

	//  set the uniform light and material values in the shader
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
	glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
	glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
	glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
	glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
	glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
	glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
	glProgramUniform1f(renderingProgram, mshiLoc, matShi);
}

void setupVertices(void) {

	
	std::vector<glm::vec3> vert;
	std::vector<glm::vec2> tex;
	std::vector<glm::vec3> norm;
	std::vector<glm::vec3> tang;

	
	//grid
	numGroundVertices = ground.getNumVertices();
	vert = ground.getVertices();
	tex = ground.getTextureCoords();
	norm = ground.getNormals();

	std::vector<float> groundPValues;
	std::vector<float> groundTValues;
	std::vector<float> groundNValues;

	for (int i = 0; i < numGroundVertices; i++) {
		groundPValues.push_back(vert[i].x);
		groundPValues.push_back(vert[i].y);
		groundPValues.push_back(vert[i].z);
		groundTValues.push_back(tex[i].x);
		groundTValues.push_back(tex[i].y);
		groundNValues.push_back(norm[i].x);
		groundNValues.push_back(norm[i].y);
		groundNValues.push_back(norm[i].z);
	}

	//gun
	numGunVertices = gun.getNumVertices();
	vert = gun.getVertices();
	tex = gun.getTextureCoords();
	norm = gun.getNormals();

	std::vector<float> gunPValues;
	std::vector<float> gunTValues;
	std::vector<float> gunNValues;

	for (int i = 0; i < numGunVertices; i++) {
		gunPValues.push_back(vert[i].x);
		gunPValues.push_back(vert[i].y);
		gunPValues.push_back(vert[i].z);
		gunTValues.push_back(tex[i].x);
		gunTValues.push_back(tex[i].y);
		gunNValues.push_back(norm[i].x);
		gunNValues.push_back(norm[i].y);
		gunNValues.push_back(norm[i].z);
	}

	//building
	numBuildingVertices = building.getNumVertices();
	vert = building.getVertices();
	tex = building.getTextureCoords();
	norm = building.getNormals();

	std::vector<float> buildingPValues;
	std::vector<float> buildingTValues;
	std::vector<float> buildingNValues;

	for (int i = 0; i < numBuildingVertices; i++) {
		buildingPValues.push_back(vert[i].x);
		buildingPValues.push_back(vert[i].y);
		buildingPValues.push_back(vert[i].z);
		buildingTValues.push_back(tex[i].x);
		buildingTValues.push_back(tex[i].y);
		buildingNValues.push_back(norm[i].x);
		buildingNValues.push_back(norm[i].y);
		buildingNValues.push_back(norm[i].z);
	}

	//cube
	float cubeVertexPositions[108] =
	{ -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f,
		1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
	};


	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	//cube
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexPositions), cubeVertexPositions, GL_STATIC_DRAW);

	//grid stuff 
	//  ground vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, groundPValues.size() * 4, &groundPValues[0], GL_STATIC_DRAW);

	//  ground texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, groundTValues.size() * 4, &groundTValues[0], GL_STATIC_DRAW);

	// ground normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, groundNValues.size() * 4, &groundNValues[0], GL_STATIC_DRAW);

	//gun stuff 
	//  gun vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, gunPValues.size() * 4, &gunPValues[0], GL_STATIC_DRAW);

	//  gun texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, gunTValues.size() * 4, &gunTValues[0], GL_STATIC_DRAW);

	// gun normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, gunNValues.size() * 4, &gunNValues[0], GL_STATIC_DRAW);

	//building stuff 
	//  building vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, buildingPValues.size() * 4, &buildingPValues[0], GL_STATIC_DRAW);

	//  building texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, buildingTValues.size() * 4, &buildingTValues[0], GL_STATIC_DRAW);

	// building normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, buildingNValues.size() * 4, &buildingNValues[0], GL_STATIC_DRAW);

}


void setupShadowBuffers(GLFWwindow* window) {
	glfwGetFramebufferSize(window, &width, &height);
	scSizeX = width;
	scSizeY = height;

	glGenFramebuffers(1, &shadowBuffer);

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
		scSizeX, scSizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	// may reduce shadow border artifacts
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->moveCameraForward(playerSpeed);
		gunCamera->moveCameraBackward(playerSpeed);
		cout << "Camera: " << camera->getX() << " " << camera->getY() << " " << camera->getZ() << endl;
		cout << "Mouse Position:" << lastX << " " << lastY << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->moveCameraBackward(playerSpeed);
		gunCamera->moveCameraForward(playerSpeed);
		cout << "Camera: " << camera->getX() << " " << camera->getY() << " " << camera->getZ() << endl;
		cout << "Mouse Position:" << lastX << " " << lastY << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->moveCameraLeft(playerSpeed);
		gunCamera->moveCameraRight(playerSpeed);
		cout << "Camera: " << camera->getX() << " " << camera->getY() << " " << camera->getZ() << endl;
		cout << "Mouse Position:" << lastX << " " << lastY << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->moveCameraRight(playerSpeed);
		gunCamera->moveCameraLeft(playerSpeed);
		cout << "Camera: " << camera->getX() << " " << camera->getY() << " " << camera->getZ() << endl;
		cout << "Mouse Position:" << lastX << " " << lastY << endl;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		playerSpeed = 0.5f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
		playerSpeed = 0.25f;
	}

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	if (ypos > 195.0f)
		ypos = 195.0f;
	if (ypos < -195.0f)
		ypos = -195.0f;

	camera->mouseMovement(xoffset, yoffset);
	//gunCamera->mouseMovement(-xoffset, -yoffset);
	cout << "Camera: " << camera->getX() << " " << camera->getY() << " " << camera->getZ() << endl;
	cout << "Mouse Position:" << xpos << " " << ypos << endl;
}


void init(GLFWwindow* window) {
	renderingProgramShadows = Utils::createShaderProgram("shaders/vertShadowShader.glsl", "shaders/fragShadowShader.glsl");
	renderingProgram = Utils::createShaderProgram("shaders/vertShader.glsl", "shaders/fragShader.glsl");
	renderingProgramCubeMap = Utils::createShaderProgram("shaders/vertCShader.glsl", "shaders/fragCShader.glsl");
	renderingProgramEnvironment = Utils::createShaderProgram("shaders/vertEShader.glsl", "shaders/fragEShader.glsl");
	renderingProgramBump = Utils::createShaderProgram("shaders/vertBumpShader.glsl", "shaders/fragBumpShader.glsl");

	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);

	setupVertices();

	skyboxTexture = Utils::loadCubeMap("textures/cubeMap"); // expects a folder name
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	setupShadowBuffers(window);

	b = glm::mat4(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f);

	generateNoise();

	grassyTexture = Utils::loadTexture("textures/grassyTexture.png");
	grassyTextureNM = Utils::loadTexture("textures/grassyTextureNM.png");
	grassyTextureHM = Utils::loadTexture("textures/grassyTextureHM.png");
	blackTexture = Utils::loadTexture("textures/blackTexture.png");
	whiteTexture = Utils::loadTexture("textures/whiteTexture.png");


}

void display(GLFWwindow* window, double currentTime) {

	processInput(window);

	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	

	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);

	glDrawBuffer(GL_NONE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);	// for reducing
	glPolygonOffset(3.0f, 5.0f);		//  shadow artifacts

	passOne();

	glDisable(GL_POLYGON_OFFSET_FILL);	// artifact reduction, continued

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	passSkyBox();

	glDrawBuffer(GL_FRONT);

	passTwo();


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void passOne(void) {
	glUseProgram(renderingProgramShadows);

	currentLightPos = glm::vec3(lightLoc);

	lightVmatrix = glm::lookAt(currentLightPos, origin, up);
	lightPmatrix = glm::perspective(toRadians(60.0f), aspect, 0.1f, 1000.0f);

	// draw the ground 
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(terLocX, terLocY - 35, terLocZ));
	mMat = glm::scale(mMat, glm::vec3(300.0f, 300.0f, 300.0f));

	shadowMVP1 = glm::mat4(1.0f);
	shadowMVP1 = shadowMVP1 * lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// vertices buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numGroundVertices);

	// draw the gun
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(camera->getX() + 0.04f, camera->getY() - 0.03f, camera->getZ() - 0.2f));
	mMat = glm::scale(mMat, glm::vec3(0.007f, 0.007f, 0.007f));
	mMat = glm::rotate(mMat, -1.2f, glm::vec3(0.0f, 1.0f, 0.0f));


	shadowMVP1 = glm::mat4(1.0f);
	shadowMVP1 = shadowMVP1 * lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// vertices buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numGunVertices);

	// draw the building
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(buildingLocX, buildingLocY - 15.0f, buildingLocZ + 20.0f));
	mMat = glm::scale(mMat, glm::vec3(60.0f, 60.0f, 60.0f));

	shadowMVP1 = glm::mat4(1.0f);
	shadowMVP1 = shadowMVP1 * lightPmatrix * lightVmatrix * mMat;
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP1));

	// vertices buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numBuildingVertices);

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void passSkyBox(void) {
	glClear(GL_DEPTH_BUFFER_BIT);

	vMat = glm::mat4(1.0f);
	vMat = vMat * camera->getMatrix();

	// draw cube map

	glUseProgram(renderingProgramCubeMap);

	vLoc = glGetUniformLocation(renderingProgramCubeMap, "v_matrix");
	projLoc = glGetUniformLocation(renderingProgramCubeMap, "proj_matrix");

	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);	// cube is CW, but we are viewing the inside
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_DEPTH_TEST);

}

void passTwo(void) {
	// draw scene 

	//draw ground
	glUseProgram(renderingProgramBump);

	mvLoc = glGetUniformLocation(renderingProgramBump, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgramBump, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgramBump, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgramBump, "shadowMVP");

	thisAmb[0] = zMatAmb[0]; thisAmb[1] = zMatAmb[1]; thisAmb[2] = zMatAmb[2];  // zero
	thisDif[0] = zMatDif[0]; thisDif[1] = zMatDif[1]; thisDif[2] = zMatDif[2];
	thisSpe[0] = zMatSpe[0]; thisSpe[1] = zMatSpe[1]; thisSpe[2] = zMatSpe[2];
	thisShi = zMatShi;

	vMat = glm::mat4(1.0f);
	vMat = vMat * camera->getMatrix();


	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(terLocX, terLocY - 35, terLocZ));
	mMat = glm::scale(mMat, glm::vec3(600.0f, 600.0f, 600.0f));

	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgramBump, vMat);

	mvMat = glm::mat4(1.0f);
	mvMat = mvMat * vMat * mMat;

	invTrMat = glm::transpose(glm::inverse(mvMat));


	shadowMVP2 = glm::mat4(1.0f);
	shadowMVP2 = shadowMVP2 * b * lightPmatrix * lightVmatrix * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	// vertices buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	// texture coordinate buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(2);

	// normals buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	// texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grassyTexture);

	// height map
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, grassyTextureHM);

	// normal map
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, grassyTextureNM);

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glDrawArrays(GL_TRIANGLES, 0, numGroundVertices);

	//draw gun
	glUseProgram(renderingProgram);


	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram, "shadowMVP");

	thisAmb[0] = zMatAmb[0]; thisAmb[1] = zMatAmb[1]; thisAmb[2] = zMatAmb[2];  // zero
	thisDif[0] = zMatDif[0]; thisDif[1] = zMatDif[1]; thisDif[2] = zMatDif[2];
	thisSpe[0] = zMatSpe[0]; thisSpe[1] = zMatSpe[1]; thisSpe[2] = zMatSpe[2];
	thisShi = zMatShi;

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(camera->getX() + 0.04f, camera->getY() - 0.03f, camera->getZ() - 0.2f));
	mMat = glm::scale(mMat, glm::vec3(0.007f, 0.007f, 0.007f));
	mMat = glm::rotate(mMat, -1.2f, glm::vec3(0.0f, 1.0f, 0.0f));

	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram, vMat);

	mvMat = glm::mat4(1.0f);
	mvMat = mvMat * vMat * mMat;

	invTrMat = glm::transpose(glm::inverse(mvMat));


	shadowMVP2 = glm::mat4(1.0f);
	shadowMVP2 = shadowMVP2 * b * lightPmatrix * lightVmatrix * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blackTexture);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, gun.getNumVertices());

	//draw building
	glUseProgram(renderingProgram);


	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
	sLoc = glGetUniformLocation(renderingProgram, "shadowMVP");

	thisAmb[0] = zMatAmb[0]; thisAmb[1] = zMatAmb[1]; thisAmb[2] = zMatAmb[2];  // zero
	thisDif[0] = zMatDif[0]; thisDif[1] = zMatDif[1]; thisDif[2] = zMatDif[2];
	thisSpe[0] = zMatSpe[0]; thisSpe[1] = zMatSpe[1]; thisSpe[2] = zMatSpe[2];
	thisShi = zMatShi;


	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(buildingLocX, buildingLocY - 15.0f, buildingLocZ + 20.0f));
	mMat = glm::scale(mMat, glm::vec3(60.0f, 60.0f, 60.0f));

	currentLightPos = glm::vec3(lightLoc);
	installLights(renderingProgram, vMat);

	mvMat = glm::mat4(1.0f);
	mvMat = mvMat * vMat * mMat;

	invTrMat = glm::transpose(glm::inverse(mvMat));


	shadowMVP2 = glm::mat4(1.0f);
	shadowMVP2 = shadowMVP2 * b * lightPmatrix * lightVmatrix * mMat;

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniformMatrix4fv(sLoc, 1, GL_FALSE, glm::value_ptr(shadowMVP2));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, whiteTexture);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, building.getNumVertices());



}



void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
	aspect = (float)newWidth / (float)newHeight;
	glViewport(0, 0, newWidth, newHeight);
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(SCR_HEIGHT, SCR_WIDTH, "Outbreak 64 By Isloomer", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	glfwSetWindowSizeCallback(window, window_size_callback);



	init(window);



	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();

	}


	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}