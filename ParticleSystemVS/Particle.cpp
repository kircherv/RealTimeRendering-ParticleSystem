#include <gl/glew.h>
#include "Particle.h"

bool TransformFeedbackParticleSystem::initParticleSystem()
{
	glGenTransformFeedbacks(1, &m_transformFeedbackBuffer);
	glGenQueries(1, &m_query);

	glGenBuffers(2, m_particleVBOs);
	glGenVertexArrays(2, m_particlesVAOs);

	Particle initParticle;
	initParticle.type = PARTICLE_TYPE_GENERATOR;

	for (int i = 0; i < 2; i++)
	{
		glBindVertexArray(m_particlesVAOs[i]);
		glBindBuffer(GL_ARRAY_BUFFER, m_particleVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*MAX_PARTICLES_ON_SCENE, NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle);

		for(int j = 0; j < 7; j++)
			glEnableVertexAttribArray(j);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0); // Position
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12); // Velocity
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24); // Color
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36); // Lifetime
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)40); // Size
		glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)44); // Size
		glVertexAttribPointer(6, 1, GL_INT, GL_FALSE, sizeof(Particle), (const GLvoid*)48); // Type
	}

	m_currentParticles = 1;

	return true;
}