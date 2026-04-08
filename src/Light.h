#pragma once
#include <glm/glm.hpp>

enum LightType
{
	Directional,
	Point,
	Spot
};

struct LightConfig
{
	bool isActive = false;
	LightType type = LightType::Directional;

	glm::vec3 position  {  0.0f  };
	glm::vec3 direction { -0.2f, -1.0f, -0.3f };

	glm::vec3 ambient	{  0.2f  };
	glm::vec3 diffuse   {  0.5f  };
	glm::vec3 specular  {  1.0f  };

	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	float cutOff = glm::cos(glm::radians(12.5f)); // cosine of cutoff angle
	float outerCutOff = glm::cos(glm::radians(15.0f));
};