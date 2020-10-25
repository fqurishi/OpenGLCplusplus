#include <cmath>
#include <vector>
#include <glm\glm.hpp>
#include <GL\glew.h>

class Camera
{
private:
	glm::vec4 u, v, n, c, location;
	glm::mat4 m;
	void init();

public:
	Camera(float x, float y, float z);
	double pitch = 0;
	double pan = 0;
	void moveCameraLeft(float x);
	void moveCameraRight(float x);
	void moveCameraUp(float y);
	void moveCameraDown(float y);
	void moveCameraForward(float z);
	void moveCameraBackward(float z);
	void panRight(float a);
	void panLeft(float a);
	void pitchUp(float a);
	void pitchDown(float a);
	float getX();
	float getY();
	float getZ();
	glm::mat4 getMatrix();
	void updateView();
	void mouseMovement(double xoffset, double yoffset, GLboolean constrainPitch = true);


};