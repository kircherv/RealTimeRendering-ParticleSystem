#include <gl/glew.h>
#include "Shader2.h"
#include "Particle.h"

#include "Renderer.h"

bool TransformFeedbackParticleSystem::initParticleSystem()
{
	glGenTransformFeedbacks(1, &m_transformFeedbackBuffer);
	glGenQueries(1, &m_query);

	glGenBuffers(2, m_particleVBOs);
	glGenVertexArrays(2, m_particlesVAOs);

	Particle initParticle;
	initParticle.type = PARTICLE_TYPE_GENERATOR;
	initParticle.position = glm::vec3(1, 2, 3);

	for (int i = 0; i < 2; i++)
	{
		GLCALL(glBindVertexArray(m_particlesVAOs[i]));
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, m_particleVBOs[i]));
		GLCALL(glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*MAX_PARTICLES_ON_SCENE, NULL, GL_DYNAMIC_DRAW));
		GLCALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), &initParticle));

		for (int j = 0; j < NUM_PARTICLE_ATTRIBUTES; j++) {
			GLCALL(glEnableVertexAttribArray(j));
		}

		GLCALL(glVertexAttribPointer(ATTR_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0)); // Position
		GLCALL(glVertexAttribPointer(ATTR_VELOCITY, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12)); // Velocity
		GLCALL(glVertexAttribPointer(ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24)); // Color
		GLCALL(glVertexAttribPointer(ATTR_LIFE, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)36)); // Lifetime
		GLCALL(glVertexAttribPointer(ATTR_SIZE, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)40)); // Size
		GLCALL(glVertexAttribPointer(ATTR_WEIGHT, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)44)); // Weight
		GLCALL(glVertexAttribPointer(ATTR_TYPE, 1, GL_INT, GL_FALSE, sizeof(Particle), (const GLvoid*)48)); // Type
	}

	m_readBufferIndex = 0;
	m_currentParticles = 1;

	const char* sVaryings[NUM_PARTICLE_ATTRIBUTES] =
	{
		"vPositionOut",
		"vVelocityOut",
		"vColorOut",
		"fLifeTimeOut",
		"fSizeOut",
		"fWeightOut",
		"iTypeOut",
	};

	Shader2 updateVertexShader("Shaders/particles_update.vert", GL_VERTEX_SHADER);
	Shader2 updateGeomShader("Shaders/particles_update.geom", GL_GEOMETRY_SHADER);

	m_updateShaderProgram.createProgram();
	m_updateShaderProgram.addShader(updateVertexShader.ID);
	m_updateShaderProgram.addShader(updateGeomShader.ID);
	for (int i = 0; i < NUM_PARTICLE_ATTRIBUTES; i++)
		glTransformFeedbackVaryings(m_updateShaderProgram.ID, NUM_PARTICLE_ATTRIBUTES, sVaryings, GL_INTERLEAVED_ATTRIBS);
	m_updateShaderProgram.linkProgram();

	// Rendering program

	Shader2 renderVertexShader("Shaders/particles_render.vert", GL_VERTEX_SHADER);
	Shader2 renderGeomShader("Shaders/particles_render.geom", GL_GEOMETRY_SHADER);
	Shader2 renderFragShader("Shaders/particles_render.frag", GL_FRAGMENT_SHADER);

	m_renderShaderProgram.createProgram();

	m_renderShaderProgram.addShader(renderVertexShader.ID);
	m_renderShaderProgram.addShader(renderGeomShader.ID);
	m_renderShaderProgram.addShader(renderFragShader.ID);

	m_renderShaderProgram.linkProgram();

	return true;
}

void TransformFeedbackParticleSystem::setProperties(glm::vec3 a_vGenPosition, glm::vec3 a_vGenVelocityMin, glm::vec3 a_vGenVelocityMax, glm::vec3 a_vGenGravityVector, glm::vec3 a_vGenColor, float a_fGenLifeMin, float a_fGenLifeMax, float a_fGenSize, float fEvery, int a_iNumToGenerate)
{
	vGenPosition = a_vGenPosition;
	vGenVelocityMin = a_vGenVelocityMin;
	vGenVelocityRange = a_vGenVelocityMax - a_vGenVelocityMin;

	vGenGravityVector = a_vGenGravityVector;
	vGenColor = a_vGenColor;
	fGenSize = a_fGenSize;

	fGenLifeMin = a_fGenLifeMin;
	fGenLifeRange = a_fGenLifeMax - a_fGenLifeMin;

	fNextGenerationTime = fEvery;
	fElapsedTime = 0.0f;

	iNumToGenerate = a_iNumToGenerate;
}

void TransformFeedbackParticleSystem::setMatrices(glm::mat4 matProjection, glm::mat4 matView)
{
	m_matProjection = matProjection;
	m_matView = matView;
}

void TransformFeedbackParticleSystem::setCameraProperties(glm::vec3 vEye, glm::vec3 vView, glm::vec3 vUpVector)
{
	m_vQuad1 = glm::cross(vView, vUpVector);
	m_vQuad1 = glm::normalize(m_vQuad1);
	m_vQuad2 = glm::cross(vView, m_vQuad1);
	m_vQuad2 = glm::normalize(m_vQuad2);

	m_vQuad1 = glm::vec3(1, 0, 0);
	m_vQuad2 = glm::vec3(0, 1, 0);
}

float grandf(float fMin, float fAdd)
{
	float fRandom = float(rand() % (RAND_MAX + 1)) / float(RAND_MAX);
	return fMin + fAdd*fRandom;
}


void TransformFeedbackParticleSystem::updateParticles(float fTimePassed)
{
	m_updateShaderProgram.use();

	m_updateShaderProgram.setFloat("fTimePassed", fTimePassed);
	m_updateShaderProgram.setVec3("vGenPosition", vGenPosition);
	m_updateShaderProgram.setVec3("vGenVelocityMin", vGenVelocityMin);
	m_updateShaderProgram.setVec3("vGenVelocityRange", vGenVelocityRange);
	m_updateShaderProgram.setVec3("vGenColor", vGenColor);
	m_updateShaderProgram.setVec3("vGenGravityVector", vGenGravityVector);

	m_updateShaderProgram.setFloat("fGenLifeMin", fGenLifeMin);
	m_updateShaderProgram.setFloat("fGenLifeRange", fGenLifeRange);

	m_updateShaderProgram.setFloat("fGenSize", fGenSize);

	fElapsedTime += fTimePassed;

	if (fElapsedTime > fNextGenerationTime)
	{
		m_updateShaderProgram.setInt("iNumToGenerate", iNumToGenerate);
		fElapsedTime -= fNextGenerationTime;

		glm::vec3 vRandomSeed = glm::vec3(grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f), grandf(-10.0f, 20.0f));
		m_updateShaderProgram.setVec3("vRandomSeed", vRandomSeed);
	}
	else
	{
		m_updateShaderProgram.setInt("iNumToGenerate", 0);
	}

	GLCALL(glEnable(GL_RASTERIZER_DISCARD));
	GLCALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedbackBuffer));

	GLCALL(glBindVertexArray(m_particlesVAOs[m_readBufferIndex]));
	//GLCALL(glEnableVertexAttribArray(ATTR_VELOCITY)); // Re-enable velocity
	//GLCALL(glEnableVertexAttribArray(ATTR_WEIGHT)); // Re-enable weight

	GLCALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleVBOs[1 - m_readBufferIndex]));

	GLCALL(glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_query));
	GLCALL(glBeginTransformFeedback(GL_POINTS));

	GLCALL(glDrawArrays(GL_POINTS, 0, m_currentParticles));

	GLCALL(glEndTransformFeedback());

	GLCALL(glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN));
	GLCALL(glGetQueryObjectiv(m_query, GL_QUERY_RESULT, &m_currentParticles));

	GLbyte qqq[77];
	glBindBuffer(GL_ARRAY_BUFFER, m_particleVBOs[1 - m_readBufferIndex]);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle), qqq);
	Particle* particle = (Particle*)qqq;

	std::cout << "NP: " << m_currentParticles << " Vector position: " << particle->position.x << " " << particle->position.y << " " << particle->position.z << std::endl;

	m_readBufferIndex = 1 - m_readBufferIndex;

	GLCALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

	//std::cout << "Particles on scene: " << m_currentParticles << std::endl;
}

void TransformFeedbackParticleSystem::renderParticles()
{
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glDepthMask(0);

	glDisable(GL_RASTERIZER_DISCARD);

	GLCALL(m_renderShaderProgram.use());
	GLCALL(m_renderShaderProgram.setMat4("projectionMatrix", m_matProjection));
	GLCALL(m_renderShaderProgram.setMat4("viewMatrix", m_matView));
	GLCALL(m_renderShaderProgram.setVec3("vQuad1", m_vQuad1));
	GLCALL(m_renderShaderProgram.setVec3("vQuad2", m_vQuad2));
	//GLCALL(m_renderShaderProgram.setInt("gSampler", 0));

	GLCALL(glBindVertexArray(m_particlesVAOs[m_readBufferIndex]));
	// Disable velocity, because we don't need it for rendering
	//GLCALL(glDisableVertexAttribArray(ATTR_VELOCITY));
	//GLCALL(glDisableVertexAttribArray(ATTR_WEIGHT));

	GLCALL(glDrawArrays(GL_POINTS, 0, m_currentParticles));

	//glDepthMask(1);
	//glDisable(GL_BLEND);
}
