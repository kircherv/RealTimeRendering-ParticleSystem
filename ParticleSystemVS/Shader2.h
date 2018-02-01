#pragma once

//#include <glad/glad.h>
#include <gl/glew.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader2
{
public:
	GLuint ID;
	bool isCompiled = false;
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader2(const char* path, int shaderType)
	{
		// 1. retrieve the vertex/fragment source code from filePath
		std::string code;
		std::ifstream file;

		// ensure ifstream objects can throw exceptions:
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			std::stringstream stream;
			file.open(path); // open files
			stream << file.rdbuf();
			file.close();
			auto code = stream.str();

			// vertex shader
			const char* vShaderCode = code.c_str();
			ID = glCreateShader(shaderType);
			glShaderSource(ID, 1, &vShaderCode, NULL);
			glCompileShader(ID);
			checkCompileErrors(ID, "PROGRAM");
			checkCompileErrors2();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
	}

	void deleteShader()
	{
		glDeleteShader(ID);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
			
		}
	}

	void checkCompileErrors2()
	{
		GLint success;
		glGetShaderiv(ID, GL_COMPILE_STATUS, &success);
		if (success != GL_TRUE)
		{
			GLchar infoLog[1024];
			glGetShaderInfoLog(ID, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR: " << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
		else isCompiled = true;
	}
};
