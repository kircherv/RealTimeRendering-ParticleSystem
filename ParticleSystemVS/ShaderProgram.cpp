#include "ShaderProgram.h"
#include <iostream>

void ShaderProgram::createProgram()
{
	ID = glCreateProgram();
}

void ShaderProgram::addShader(GLuint shaderID)
{
	glAttachShader(ID, shaderID);
}

bool ShaderProgram::linkProgram()
{
	glLinkProgram(ID);

	int iLinkStatus;
	glGetProgramiv(ID, GL_LINK_STATUS, &iLinkStatus);
	isLinked = iLinkStatus == GL_TRUE;

	if (!isLinked)
	{
		GLchar infoLog[1024];
		glGetProgramInfoLog(ID, 1024, NULL, infoLog);
		std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << "" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
	}

	return isLinked;
}

void ShaderProgram::use()
{
	glUseProgram(ID);
}
