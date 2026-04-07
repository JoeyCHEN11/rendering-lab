#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	// Default camera values
	static constexpr float YAW		   = 0.0f;
	static constexpr float PITCH	   = 0.0f;
	static constexpr float SPEED	   = 2.5f;
	static constexpr float SENSITIVITY = 0.1f;
	static constexpr float ZOOM		   = 45.0f;
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Fov; //Field of View Y in degree

	Camera(
		glm::vec3 position = { 0.0f, 0.0f, 0.0f },
		glm::vec3 up = { 0.0f, 1.0f, 0.0f },
		float yaw = YAW, float pitch = PITCH
	);
	// constructor with scalar values
	Camera(
		float posX, float posY, float posZ,
		float upX, float upY, float upZ,
		float yaw, float pitch
	);

	glm::mat4 GetViewMatrix() const;

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	void ProcessMouseScroll(float yoffset);

private:

	// update front, right and up vectors
	// called whenever Yaw/Pitch changes
	void updateCameraVectors();

	// Camera_Exercise 2
	glm::mat4 myLookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up) const;
};