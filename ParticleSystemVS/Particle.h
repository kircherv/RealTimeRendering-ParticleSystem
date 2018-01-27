#pragma once

//#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define MAX_PARTICLES_ON_SCENE 100000

#define PARTICLE_TYPE_GENERATOR 0
#define PARTICLE_TYPE_NORMAL 1

struct Particle {
	glm::vec3 position, velocity;
	glm::vec3 color;
	float life, size, weight;
	int type;
};

class TransformFeedbackParticleSystem
{
	bool initParticleSystem();

private:

	unsigned int m_transformFeedbackBuffer;
	unsigned int m_query;

	unsigned int m_particleVBOs[2];
	unsigned int m_particlesVAOs[2];

	int m_currentParticles;
};