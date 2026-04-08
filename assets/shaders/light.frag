#version 330 core
// Exercise 1
struct Light
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_diffuse4;
	sampler2D texture_specular1;
	sampler2D texture_specular2;

	float shininess;
};

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform Light light;

void main()
{
//	vec3 diffColor = texture(material.texture_diffuse1, TexCoords).rgb;
//	vec3 specColor = texture(material.texture_specular1, TexCoords).rgb;
	FragColor = vec4(light.ambient + light.diffuse, 1.0);

}