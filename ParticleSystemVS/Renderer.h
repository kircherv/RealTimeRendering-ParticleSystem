#pragma once

#include <GL/glew.h>
#include <iostream>

//Macro for insert breakpoint for loc with gl error
#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCALL(x) GLClearError();\
	x;\
	ASSERT(glLogCall(#x, __FILE__, __LINE__))


static void GLClearError()
{

	while (glGetError() != GL_NO_ERROR);
}

static bool glLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "Breakpoint Error![OpenGL Error]( " << error << " )" << function << " " << file << ": (line of code with error): " << line << std::endl;
		return false;
	}
	return true;
}