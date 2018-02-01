#pragma once

//#include <glad/glad.h>

#include "ShaderProgram.h"

#include <GLFW/glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define NUM_PARTICLE_ATTRIBUTES 7

#define MAX_PARTICLES_ON_SCENE 100000

#define PARTICLE_TYPE_GENERATOR 0
#define PARTICLE_TYPE_NORMAL 1

#define ATTR_POSITION 0
#define ATTR_VELOCITY 1
#define ATTR_COLOR 2
#define ATTR_LIFE 3
#define ATTR_SIZE 4
#define ATTR_WEIGHT 5
#define ATTR_TYPE 6

struct Particle {
	glm::vec3 position, velocity;
	glm::vec3 color;
	float life, size, weight;
	int type;
};

class TransformFeedbackParticleSystem
{
public:
	bool initParticleSystem();

	void setProperties(glm::vec3 a_vGenPosition, glm::vec3 a_vGenVelocityMin, glm::vec3 a_vGenVelocityMax, glm::vec3 a_vGenGravityVector, glm::vec3 a_vGenColor, float a_fGenLifeMin, float a_fGenLifeMax, float a_fGenSize, float fEvery, int a_iNumToGenerate);
	void setMatrices(glm::mat4 matProjection, glm::mat4 matView);
	void setCameraProperties(glm::vec3 vEye, glm::vec3 vView, glm::vec3 vUpVector);

	void updateParticles(float fTimePassed);
	void renderParticles();

private:

	unsigned int m_transformFeedbackBuffer;
	unsigned int m_query;

	unsigned int m_particleVBOs[2];
	unsigned int m_particlesVAOs[2];

	int m_readBufferIndex;

	int m_currentParticles;

	float fElapsedTime = 0.0f;
	float fNextGenerationTime;

	glm::vec3 vGenPosition;
	glm::vec3 vGenVelocityMin, vGenVelocityRange;
	glm::vec3 vGenGravityVector;
	glm::vec3 vGenColor;

	float fGenLifeMin, fGenLifeRange;
	float fGenSize;

	int iNumToGenerate;

	glm::mat4 m_matProjection;
	glm::mat4 m_matView;

	glm::vec3 m_vQuad1;
	glm::vec3 m_vQuad2;

	ShaderProgram m_updateShaderProgram;
	ShaderProgram m_renderShaderProgram;
};