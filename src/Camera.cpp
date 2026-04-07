#include "Camera.h"

Camera::Camera(
	glm::vec3 position,
	glm::vec3 up,
	float yaw, float pitch
)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVITY),
	Fov(ZOOM)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

Camera::Camera(
	float posX, float posY, float posZ,
	float upX, float upY, float upZ,
	float yaw, float pitch
)
	: Front{ 0.0f, 0.0f, -1.0f },
	MovementSpeed(SPEED),
	MouseSensitivity(SENSITIVITY), Fov(ZOOM)
{
	Position = { posX, posY, posZ };
	WorldUp = { upX, upY, upZ };
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
	// Exercise 2
	return myLookAt(Position, Position + Front, Up);
	//return glm::lookAt(Position, Position + Front, Up);


}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		Position += velocity * Front;
	if (direction == BACKWARD)
		Position -= velocity * Front;
	if (direction == LEFT)
		// go left
		Position -= velocity * Right;
	if (direction == RIGHT)
		Position += velocity * Right;
	if (direction == UP)
		// go up (simulate the movement in Unity scene)
		Position += velocity * Up;
	if (direction == DOWN)
		Position -= velocity * Up;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	if (constrainPitch)
	{
		if (Pitch > 89.9f)
			Pitch = 89.9f;
		if (Pitch < -89.9f)
			Pitch = -89.9f;
	}
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
	Fov -= (float)yoffset;
	if (Fov < 1.0f)
		Fov = 1.0f;
	if (Fov > 45.0f)
		Fov = 45.0f;
}


void Camera::updateCameraVectors()
{
	// calculate Front
	// optimize: default look at -z
	// pitch: couter-clockwise; yall: clockwise
	glm::vec3 front{};
	front.x = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = -cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// update Right and Up
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));

}

glm::mat4 Camera::myLookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) const
{
	glm::vec3 zCam = glm::normalize(eye - center);
	glm::vec3 xCam = glm::normalize(glm::cross(up, zCam));
	glm::vec3 yCam = glm::cross(zCam, xCam);

	glm::mat4 rotateCam{ { xCam, 0 }, { yCam, 0 }, { zCam, 0 }, { 0.0f, 0.0f, 0.0f, 1.0f } };
	glm::mat4 transCam{ 1 };
	transCam[3] = { -eye, 1 };

	glm::mat4 view = glm::transpose(rotateCam) * transCam;
	return view;
}

