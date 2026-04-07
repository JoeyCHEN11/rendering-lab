#include "Shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexCode;
	std::string fragCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	// ensure ifstream objects can throw exceptions
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);

		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		
		vShaderFile.close();
		fShaderFile.close();

		vertexCode = vShaderStream.str();
		fragCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
	}
	const char* vertexCodeCStr = vertexCode.c_str();
	const char* fragCodeCStr = fragCode.c_str();

	// create vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexCodeCStr, nullptr);
	glCompileShader(vertexShader);
	checkShaderCompilation(vertexShader);

	unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragCodeCStr, nullptr);
	glCompileShader(fragShader);
	checkShaderCompilation(fragShader);
	
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragShader);
	glLinkProgram(ID);
	checkShaderLink();

	// delete already linked shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform1i(uniformLocation, (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform1i(uniformLocation, value);
}

void Shader::setFloat(const std::string& name, float value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform1f(uniformLocation, value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform2fv(uniformLocation, 1, glm::value_ptr(value));
}

void Shader::setVec2(const std::string& name, float x, float y) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform2f(uniformLocation, x, y);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform3fv(uniformLocation, 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform3f(uniformLocation, x, y, z);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform4fv(uniformLocation, 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
	int uniformLocation = getUniformLocation(name);
	glUniform4f(uniformLocation, x, y, z, w);
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
	int uniformLocation = getUniformLocation(name);
	glUniformMatrix2fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
	int uniformLocation = getUniformLocation(name);
	glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
	int uniformLocation = getUniformLocation(name);
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(mat));
}

bool Shader::checkShaderCompilation(const unsigned int shaderId)
{
	//test compilation
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderId, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		return false;
	}
	else
	{
		std::cout << "Shader " << shaderId << " compile success" << std::endl;
		return true;
	}
}

bool Shader::checkShaderLink()
{
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return false;
	}
	else
	{
		std::cout << "Shader program " << ID << " link success" << std::endl;
		return true;
	}
}

int Shader::getUniformLocation(const std::string& name) const
{
	int uniformLocation = glGetUniformLocation(ID, name.c_str());
	// debug
	if (uniformLocation == -1)
	{
		//std::cout << "\rSet Uniform Failed, name \"" << name << "\" not found" << std::string(50, ' ') << std::flush;
		std::cout << "WARNING: Set Uniform Failed, name \"" << name << "\" not found" << std::endl;
	}
	return uniformLocation;
}
