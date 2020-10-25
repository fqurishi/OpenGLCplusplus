#include <cmath>
#include <vector>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>
#include "Camera.h"
#include <glm\gtc\type_ptr.hpp>
#include <GL\glew.h>
using namespace std;

Camera::Camera(float x, float y, float z) {
	init();
	location = glm::vec4(x, y, z, 1);
	updateView();
}

void Camera::init() {
	u = glm::vec4(1, 0, 0, 0);
	v = glm::vec4(0, 1, 0, 0);
	n = glm::vec4(0, 0, 1, 0);
	m = glm::mat4(u, v, n, (glm::vec4(0, 0, 0, 1)));

}

void Camera::moveCameraLeft(float x) {
	c = u;
	glm::vec4 compute = glm::normalize(c) * x;
	location = location - compute;
	updateView();

}

void Camera::moveCameraRight(float x) {
	c = u;
	glm::vec4 compute = glm::normalize(c) * x;
	location = location + compute;
	updateView();

}

void Camera::moveCameraUp(float y) {
	c = v;
	glm::vec4 compute = glm::normalize(c) * y;
	location = location + compute;
	updateView();

}

void Camera::moveCameraDown(float y) {
	c = v;
	glm::vec4 compute = glm::normalize(c) * y;
	location = location - compute;
	updateView();

}

void Camera::moveCameraForward(float z) {
	c = n;
	glm::vec4 compute = glm::normalize(c) * z;
	location = location - compute;
	location.y = 0;
	updateView();
	

}

void Camera::moveCameraBackward(float z) {
	c = n;
	glm::vec4 compute = glm::normalize(c) * z;
	location = location + compute;
	location.y = 0;
	updateView();

}

void Camera::panLeft(float a) {
	pan = pan + a;
	updateView();
}

void Camera::panRight(float a) {
	pan = pan - a;
	updateView();
}

void Camera::pitchUp(float a) {
	pitch = pitch + a;
	updateView();
}

void Camera::pitchDown(float a) {
	pitch = pitch + a;
	updateView();
}

float Camera::getX() {
	return location.x;
}

float Camera::getY() {
	return location.y;
}

float Camera::getZ() {
	return location.z;
}

glm::mat4 Camera::getMatrix() {
	return m;
}

void Camera::updateView() {

	glm::vec4 newLocation = glm::vec4(location);
	float cosPitch = (float)cos(pitch);
	float sinPitch = (float)sin(pitch);
	float cosPan = (float)cos(pan);
	float sinPan = (float)sin(pan);

	u = glm::vec4(cosPan, 0, -sinPan, 0);
	v = glm::vec4(sinPan * sinPitch, cosPitch, cosPan * sinPitch, 0);
	n = glm::vec4(sinPan * cosPitch, -sinPitch, cosPitch * cosPan, 0);
	float matArray []= {
		u.x, v.x, n.x, 0,
			u.y, v.y, n.y, 0,
			u.z, v.z, n.z, 0,
			-(glm::dot(u, newLocation)), -(glm::dot(v, newLocation)), -(glm::dot(n, newLocation)), 1};

	m = glm::make_mat4(matArray);
}

void Camera::mouseMovement(double xoffset, double yoffset, GLboolean constrainPitch)
{
	xoffset *= 0.005;
	yoffset *= 0.005;

	pan -= xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 1.45f)
			pitch = 1.45f;
		if (pitch < -1.45f)
			pitch = -1.45f;
	}

	// update values
	updateView();
}