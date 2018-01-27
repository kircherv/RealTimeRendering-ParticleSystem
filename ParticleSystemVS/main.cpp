//	Working Phong Shading 
// Local Headers
#include "glitter.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 
#include <Shader.h>
#include <filesystem.h>
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <camera.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
//Def methods outside of main for framebuffer Mouse input&zoom and Keyboard input
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// ------------------------------------------------------------------
// To Check for errors put GLCALL arround every gl funktion and become a breakpoint with loc reading
// ------------------------------------------------------------------

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
//------------------------------------------------------------------
// settings
//------------------------------------------------------------------

// Camera Code abstracted to CAmera.h -> // Create Camera Object with camera position as Parameter(+ to go back, - to go to the front)
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//For Mouse Input
bool firstMouse = true;			//to check first movement of Mouse, so it does not clip 
float lastX = 800.0f / 2.0;		//Mouse Offset Since Last Frame
float lastY = 600.0 / 2.0;		//Mouse Offset Since Last Frame

// timing
float deltaTime = 0.0f;	// Time between current frame and last frame 
float lastFrame = 0.0f; // Time of last frame

//Positions in World Space
//light source position in world space
glm::vec3 lightPosition(0.0f, 0.7f, 2.0f);
glm::vec3 floorPosition(0.0f, -1.5f, 0.0f);
glm::vec3 particleEmitterPosition(-1.2f, 0.5f, 0.0f);
glm::vec3 particleEmitterNormalPosition(-1.2f, 0.5f, 0.0f);
glm::vec3 cubePosition(2.0f, 0.5f, 0.7f);
glm::vec3 particlePosition(0.0f, 0.5f, 0.0f);
glm::vec3 BillboardPosition(0.3f, 0.5f, 0.0f);
glm::vec3 noGSPosition(2.3f, 0.5f, 0.0f);
glm::vec3 oneBillboardPosition(0.6f, 0.5f, 0.0f);
glm::vec3 multipleBillboardPosition(0.8f, 0.5f, 0.0f);
glm::vec3 testParticlePosition(1.0f, 0.5f, 0.0f);
glm::vec3 instancedQuadPosition(-1.0f, 0.5f, 0.0f);



//particle properties
float particleSize = 10.0f;
glm::vec3 uniColor(1.0f, 1.0f, 1.0f);


//particle properties for instanced Quad
//only at a particle size greater than 10 it billboards corretly, ansonsten zu langsam und nur ein bisschen drehung
float instancedParticleSize = 1.0f;
glm::vec3 instancedUniColor(0.0f, 0.0f, 1.0f);

//// CPU representation of a particle
//struct Particle {
//	glm::vec3 pos, speed;
//	unsigned char r, g, b, a; // Color
//	float size, angle, weight;
//	float life; // Remaining life of the particle. if <0 : dead and unused.
//	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f
//
//	bool operator<(const Particle& that) const {
//		// Sort in reverse order : far particles drawn first.
//		return this->cameradistance > that.cameradistance;
//	}
//};
//
//const int MaxParticles = 100000;
//Particle ParticlesContainer[MaxParticles];
//int LastUsedParticle = 0;
//
//// Finds a Particle in ParticlesContainer which isn't used yet.
//// (i.e. life < 0);
//int FindUnusedParticle() {
//
//	for (int i = LastUsedParticle; i<MaxParticles; i++) {
//		if (ParticlesContainer[i].life < 0) {
//			LastUsedParticle = i;
//			return i;
//		}
//	}
//
//	for (int i = 0; i<LastUsedParticle; i++) {
//		if (ParticlesContainer[i].life < 0) {
//			LastUsedParticle = i;
//			return i;
//		}
//	}
//
//	return 0; // All particles are taken, override the first one
//}
//
//void SortParticles() {
//	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
//}

// ------------------------------------------------------------------
//const unsigned int SCR_WIDTH = 800;    because now resizeable
//const unsigned int SCR_HEIGHT = 600;

// ------------------------------------------------------------------
//ShaderSettings now Abstracted to Shader.h and individual Shader Files under Shaders/ -> in  output folder Abgabe/glitter/Debug/shaders (exe folder) : gl_position gives use the Clip Space
// ------------------------------------------------------------------

// -----------------------------------------------------------------
// ------------------------------------------------------------------
//								MAIN
// ------------------------------------------------------------------
// ------------------------------------------------------------------
int main(int argc, char * argv[]) {

	// ------------------------------------------------------------------
	//		create ContextWindow
	// ------------------------------------------------------------------

	// Load GLFW and Create a Window
	glfwInit();
	//glfwWindowHint(Enum with (GLWF Prefix) What option to configure, Integer with value of that option)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//so I dont need the void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//Create Window for Context 
	auto mWindow = glfwCreateWindow(mWidth, mHeight, "Mein OpenGL Program", nullptr, nullptr);

	// Check for Valid Context
	if (mWindow == nullptr) {
		fprintf(stderr, "Failed to Create OpenGL Context");
		return EXIT_FAILURE;
	}

	// Create Context and Load OpenGL Functions
	glfwMakeContextCurrent(mWindow);
	//set Viewport size to program size
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	//Current Mouse position x,y 
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	//Set Scroll Callback
	glfwSetScrollCallback(mWindow, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Init Glad to enable Opengl function calling
	gladLoadGL();


	//depth test
	glEnable(GL_DEPTH_TEST);

	//Create Shader Object(vertexShaderPath, fragmentShaderPath)
	Shader cubewLightingShader("Shaders/2.2.basic_lighting.vs", "Shaders/2.2.basic_lighting.fs");
	Shader lampShader("Shaders/2.2.lamp.vs", "Shaders/2.2.lamp.fs");
	Shader floorShader("Shaders/floor.vs", "Shaders/floor.fs");
	Shader particleEmitterShader("Shaders/particleEmiter.vs", "Shaders/particleEmiter.fs");
	Shader particleEmitterNormalShader("Shaders/particleEmiter.vs", "Shaders/particleEmiter.fs", "Shaders/particleEmiter.gs");
	Shader particleShader("Shaders/particle.vs", "Shaders/particle.fs");
	Shader colorBillboardShader("Shaders/billboard.vs", "Shaders/billboard.fs", "Shaders/billboard.gs");
	Shader BillboardNoGShader("Shaders/noGSBillboard.vs", "Shaders/noGSBillboard.fs");
	Shader multipleBillboardShader("Shaders/billboard.vs", "Shaders/billboard.fs", "Shaders/billboard.gs");
	Shader particlewNormalShader("Shaders/multipleBillboards.vs", "Shaders/multipleBillboards.fs", "Shaders/multipleBillboards.gs");
	Shader testShader("Shaders/test.vs", "Shaders/test.fs", "Shaders/test.gs");
	Shader instancedQuadShader("Shaders/InstancedQuadParticle.vs", "Shaders/InstancedQuadParticle.fs");

	

	// ------------------------------------------------------------------
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	//Create Cube, it has just Position and Texture Coordinates. so the vertexbufferattrib has been tweaked accordingly.
	float vertices[] = {

		// Positions				Normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	float floorVertices[] = {
		// positions		// texture coords
		0.5f,  0.5f, 0.0f,		1.0f, 1.0f,		// top right
		0.5f, -0.5f, 0.0f,		1.0f, 0.0f,		// bottom right
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,		 // bottom left,
		-0.5f,  0.5f, 0.0f,		0.0f, 1.0f		  // top left ,

	};

	unsigned int floorIndices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float particleEmitterVertices[] = {
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
	};

	unsigned int particleEmitterIndices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float billboardwColorVertices[] = {
		// positions			// colors
		 0.5f,  0.5f, 0.0f,  	0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, 0.0f,  	1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.0f, 	1.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 1.0f
	};

	unsigned int noGSindices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float oneBillboardwColorVertices[] = {
		// positions			// colors
		0.0f,  0.0f, 0.0f,  0.0f, 1.0f, 1.0f
	};

	float BillboardswNormalsVertices[] = {
		// positions			//normals	
		 0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 0.5f,
		 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 0.5f,
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 0.5f,
		-0.5f,  0.5f, 0.0f,	 0.0f, 0.0f, 0.5f
	};		

	float instancedQuadVertices[] = {
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
	};

	unsigned int instancedQuadIndices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	

	// world space positions of our points
	glm::vec3 BillboardPositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -4.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -3.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -3.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	//VAO(Vertex Array Object): call to GLEnable/DisableVertexAttribArray, vertex attrb config(glVertexAttribPointer), VBO(glVertexAttribPointer)
	// we Create to simplify a Vertex Array Object (VAO) to store all our VBO so that when we configure vertex Attribute pointers, we only have to make those calls once!
	//to draw we bind the corresponding VAO, this mkaes switching between different vertex data and attr. as easy as binding different VAO ALL STATES WE SET; WE STORE IN VAO
	//create : VBO(Vertex Buffer Object): to manage vertices in GPU memory , Create Element BufferObject : buffer to store indices to draw, VAO as container
	unsigned int cubeVBO, cubeVAO;
	//generate VertexBufferobject with bufferId & ElementBufferObject & VertexArray for VBO & EBO
	glGenBuffers(1, &cubeVBO);
	glGenVertexArrays(1, &cubeVAO);
	//Every time we want to Draw some Object we Do Steps 0 to 3 for VBO or EBO
	// 0.of VAO: bind VertexArrayObject
	glBindVertexArray(cubeVAO);
	// 0.of VBO & EBO: bind new buffer VBO to GL_ARRAY_BUFFER(this is the TriangleType)
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	//draw Triangle
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	//Any buffer call(of target GL_ARRAY_BUFFER) will be used to configure its currently bound buffer, VBO	// 0.of VBO & EBO: Copy User Defined Vertex Data into currently bound Buffer memory
	//glBufferData: (type of buffer we want to copy into, dataSize(bytes), Data to send, forthParameter)//4thParameter: how to manage given Data : static -> data will not change. dynamic -> data likely to change alot. stream-> change on every draw	//Different VAO dont need the same amount of attributes
	// 1. tell Opengl how it should Interpret the Vertex Data(per vertex Attribute)
	// glVertexAttribPointer:----------------------------------------------------------------------------------------------------------------------
	//Param1: which vertex Attr we want to configure, since we specified the location of the position vertex Attr. in the Vertex Shader iwth layout, we pass 0 to set the location of the vertex Attr to 0(because we want to pass info to this)
	//param2: size of vertex Attr(vec3(so 3 values)//param3: type of data(here GL_FLOAT because in GLSL a vec consists of floats values)
	//param4: Bool if we want to normalize the data(if GL_TRUE-> all data that has a value not between 0 and 1(or-1 for signed data) will be mapped to those values. so we set false
	//param5: stride->if 0->OpenGL determines the stride. else-> space between consecutive vertex Attr. sets. Since the next Setof position data is exactly 2 times the size of a float,  + texture = 20= 5*float
	//param6: requares wierd cast cause of void* -> offset of where the position data begins in the buffer, since position data is at the sart of the data array this avlue is 0
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//// color attribute
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
	//Texture Attribute
	/*glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);*/

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube) (Position)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	//Floor (Position,Texture)
	unsigned int floorVBO, floorVAO, floorIndexEBO;
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorIndexEBO);
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorIndexEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//particleEmitter
	unsigned int particleEmitterVBO, particleEmitterVAO, particleEmitterIndexEBO;
	glGenBuffers(1, &particleEmitterVBO);
	glGenBuffers(1, &particleEmitterIndexEBO);
	glGenVertexArrays(1, &particleEmitterVAO);
	glBindVertexArray(particleEmitterVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, particleEmitterVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleEmitterVertices), particleEmitterVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEmitterIndexEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(particleEmitterIndices), particleEmitterIndices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	
	////particle
	unsigned int billboardVBO, billboardVAO;
	glGenBuffers(1, &billboardVBO);
	glGenVertexArrays(1, &billboardVAO);
	glBindVertexArray(billboardVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, billboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboardwColorVertices), billboardwColorVertices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// try to just do 1 particle instead of 4
	unsigned int oneBillboardVBO, oneBillboardVAO;
	glGenBuffers(1, &oneBillboardVBO);
	glGenVertexArrays(1, &oneBillboardVAO);
	glBindVertexArray(oneBillboardVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, oneBillboardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(oneBillboardwColorVertices), oneBillboardwColorVertices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int BillboardNoGSVBO, BillboardNoGSVAO, BillboardNoGSEBO;
	glGenBuffers(1, &BillboardNoGSVBO);
	glGenBuffers(1, &BillboardNoGSEBO);
	glGenVertexArrays(1, &BillboardNoGSVAO);
	glBindVertexArray(BillboardNoGSVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, BillboardNoGSVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(billboardwColorVertices), billboardwColorVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BillboardNoGSEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(noGSindices), noGSindices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	

	////particle2
	unsigned int particlewNormalVBO, particlewNormalVAO;
	glGenBuffers(1, &particlewNormalVBO);
	glGenVertexArrays(1, &particlewNormalVAO);
	glBindVertexArray(particlewNormalVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, particlewNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BillboardswNormalsVertices), BillboardswNormalsVertices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	////particletest: skip normals and set it as -cameraview
	unsigned int testVBO, testVAO;
	glGenBuffers(1, &testVBO);
	glGenVertexArrays(1, &testVAO);
	glBindVertexArray(testVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, testVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BillboardswNormalsVertices), BillboardswNormalsVertices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);



	//instancedQuad 4 points for 1 particle start for struct
	unsigned int instancedQuadVBO, instancedQuadVAO, instancedQuadEBO;
	glGenBuffers(1, &instancedQuadVBO);
	glGenBuffers(1, &instancedQuadEBO);
	glGenVertexArrays(1, &instancedQuadVAO);
	glBindVertexArray(instancedQuadVAO);
	// we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
	glBindBuffer(GL_ARRAY_BUFFER, instancedQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(instancedQuadVertices), instancedQuadVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleEmitterIndexEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(instancedQuadIndices), instancedQuadIndices, GL_STATIC_DRAW);
	// note that we update the floor's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	////Test for Particle Struct
	//
	//static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	//static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];
	//
	//for (int i = 0; i<MaxParticles; i++) {
	//	ParticlesContainer[i].life = -1.0f;
	//	ParticlesContainer[i].cameradistance = -1.0f;
	//}

	// ------------------------------------------------------------------
	//					load and create a texture 
	// ------------------------------------------------------------------
	//TEXTURE 1: ref by ID
	unsigned int texture1, texture2;
	//generate texture: (anzahl textures, save in this array)
	glGenTextures(1, &texture1);
	//bind Texture1
	glBindTexture(GL_TEXTURE_2D, texture1);
	// all upcoming GL_TEXTURE_2D operations now have effect on this texture object

	//TEXTURE WRAPPING -> (s,t,r Coordinate Axis). param1: TextureTarger. Param2: which option & axis(here option: wrap& s,t Axis). param3: wrapping Mode
	// set the texture wrapping parameters: GL_REPEAT: The default behavior for textures.Repeats the texture image.	GL_MIRRORED_REPEAT : Same as GL_REPEAT but mirrors the image with each repeat.GL_CLAMP_TO_EDGE : Clamps the coordinates between 0 and 1. The result is that higher coordinates become clamped to the edge, resulting in a stretched edge pattern.GL_CLAMP_TO_BORDER : Coordinates outside the range are now given by user - specified border color with float[]={r,g,b,a}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//set TEXTURE FILTERING parameters ->GL_NEAREST(default: closest to center). GL_LINEAR: bilinear(mix colors close to center)
	//magnify & minify filter operations
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// tell stb_image.h to flip loaded texture's on the y-axis.Since we give the Space depth we dont need these anymore to straiten the picture ->	//
	stbi_set_flip_vertically_on_load(true);
	// The FileSystem::getPath(D:/UNI/RealtimeRendering/learnopengl/Glitter) here we put the resources 
	unsigned char *data = stbi_load(FileSystem::getPath("resources/textures/marble.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		//Start to genTexture using data: param1: Textarget(on current bound TexObejct)		//param2: mipmap level->0=base. param3: in what texture Format we want to store(since container just hast rgb, we put rgb)	//param4&5: Tex height & width. param6: ALWAYS 0//param7&8: format & datatype of source image -> Since image has rgb as chars(bytes), we pass that		//param9: actual image data -> here data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//Generate Mipmap levels
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture1: " << stbi_failure_reason() << std::endl;

	}

	////after generate texture andmipmap ->free image Memory
	stbi_image_free(data);

	

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	floorShader.use(); // don't forget to activate/use the shader before setting uniforms! // either set it manually like so://glUniform1i(glGetUniformLocation(translationShader.ID, "texture1"), 0); // or set it via the texture class
	floorShader.setInt("texture1", 0);

	//output in console Opengl version
	fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));
	//fprintf(stderr, "Timer:", currentFrame);

	

	

	//---------------------------------------------------------------------------------------------------------
	//											Rendering Loop,             
	//---------------------------------------------------------------------------------------------------------
	//only close with Esc or X button
	while (glfwWindowShouldClose(mWindow) == false) {
		if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(mWindow, true);

		// per-frame time logic fps
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//Input
		processInput(mWindow);

		//renderingCommands

		// ---------------------------------------------------------------------------------------------------------
		//										RENDER
		// ---------------------------------------------------------------------------------------------------------
		// Background Fill Color: Grey   state setting function
		glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
		//which buffer to clear (color,depth or stencil   state using function
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// also clear the depth buffer now!

		 // ---------------------------------------------------------------------------------------------------------
		 //									Draw 
		 // ---------------------------------------------------------------------------------------------------------
		 //Activate Program Object
		 // bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		

		// ---------------------------------------------------------------------------------------------------------
		//					Coordinate System Transformations with Matrices
		// ---------------------------------------------------------------------------------------------------------
		//Draw Rotating Cube with lighting properties
		cubewLightingShader.use();
		//set values to uniforms in shaders for Phong Shading
		cubewLightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		cubewLightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		cubewLightingShader.setVec3("lightPosition", lightPosition);
		cubewLightingShader.setVec3("povPosition", camera.Position);
		// pass projection matrix to shader //projection View param1: FOV param2: aspect ratio. param3: near plane param4: far plane
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		projectionMatrix = glm::perspective(glm::radians(camera.Zoom), (float)mWidth / (float)mHeight, 0.1f, 100.0f);
		cubewLightingShader.setMat4("projectionMatrix", projectionMatrix);
		//pass view matrix to shader -> Camera View //param1:position(origin), right vector, up vector
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		viewMatrix = camera.GetViewMatrix();
		cubewLightingShader.setMat4("viewMatrix", viewMatrix);		
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, cubePosition);
		modelMatrix = glm::rotate(modelMatrix, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
		cubewLightingShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		

		// draw the lamp object
		lampShader.use();
		lampShader.setMat4("projectionMatrix", projectionMatrix);
		lampShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		// change the light's position values over time (can be done anywhere in the render loop actually, but try to do it at least before using the light source positions)
		lightPosition.x =  sin(glfwGetTime()) * 2.0f;
		//lightPosition.z =  sin(glfwGetTime()) * 1.0f;
		//lightPosition.y = sin(glfwGetTime() / 2.0f) * 1.0f;
		modelMatrix = glm::translate(modelMatrix, lightPosition);
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f)); // a smaller cube
		lampShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//draw the Floor object
		floorShader.use();
		floorShader.setMat4("projectionMatrix", projectionMatrix);
		floorShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, floorPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f)); // Big floor
		floorShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(floorVAO);
		GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

		//ParticleEmitter 
		particleEmitterShader.use();
		particleEmitterShader.setMat4("projectionMatrix", projectionMatrix);
		particleEmitterShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, particleEmitterPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f)); // Big floor
		particleEmitterShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(particleEmitterVAO);
		GLCALL(glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0));

		//ParticleEmitter Normal vector
		particleEmitterNormalShader.use();
		particleEmitterNormalShader.setMat4("projectionMatrix", projectionMatrix);
		particleEmitterNormalShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, particleEmitterNormalPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f)); // Big floor
		particleEmitterNormalShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(particleEmitterVAO);
		GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		// ------------------------------------------------------------------
		// JUST POINTS Vertex Shader with Position and COLOR Vectors OUT in Matrix(Loc 0 & 1)  NO GEOMETRY SHADER, fragment shader  takes just color,
		// ---------------------------------------------------------------------------------------------------------
		particleShader.use();
		GLCALL(particleShader.setFloat("particleSize", particleSize));
		particleShader.setMat4("projectionMatrix", projectionMatrix);
		particleShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, particlePosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f)); // Big floor
		particleShader.setMat4("modelMatrix", modelMatrix);
		glBindVertexArray(billboardVAO);
		//als punkte darstellen
		GLCALL(glDrawArrays(GL_POINTS, 0, 4));
		//draw Quad
		//GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));


		// ------------------------------------------------------------------
		//All Shaders with a geometry shader processes the Fragment Position now, instead of skipping and giving it to the fragment shader
		// ------------------------------------------------------------------

		//GREEN BLUE Vertical Gradient  BILLBOARD Vertex Shader with Position and COLOR Vectors OUT in Matrix(Loc 0 & 1)  Geometry Shader CALCULATES normals ONCE and passes with out to FS, fragment shader  takes fragment shader,
		// ---------------------------------------------------------------------------------------------------------
		colorBillboardShader.use();
		GLCALL(colorBillboardShader.setVec3("lightPosition", lightPosition));
		GLCALL(colorBillboardShader.setVec3("cameraPosition", camera.Position));
		GLCALL(colorBillboardShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f));
		GLCALL(colorBillboardShader.setFloat("particleSize", particleSize));
		colorBillboardShader.setMat4("projectionMatrix", projectionMatrix);
		colorBillboardShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, BillboardPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Big floor
		colorBillboardShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(oneBillboardVAO));
		GLCALL(glDrawArrays(GL_POINTS, 0, 4));



		// this one has no geometry shader
		//MultiColor  Gradient  BILLBOARD Vertex Shader with Position and COLOR Vectors OUT in Matrix(Loc 0 & 1) 
		// ---------------------------------------------------------------------------------------------------------
		BillboardNoGShader.use();
		GLCALL(BillboardNoGShader.setVec3("lightPosition", lightPosition));
		GLCALL(BillboardNoGShader.setVec3("cameraPosition", camera.Position));
		GLCALL(BillboardNoGShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f));
		GLCALL(BillboardNoGShader.setFloat("particleSize", particleSize));
		BillboardNoGShader.setMat4("projectionMatrix", projectionMatrix);
		BillboardNoGShader.setMat4("viewMatrix", viewMatrix);
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, noGSPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f)); // Big floor
		BillboardNoGShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(BillboardNoGSVAO));
		GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

		// ------------------------------------------------------------------
		//GREEN BLUE Multiple single BILLBOARD Vertical Gradient (same shaders)  BILLBOARD Vertex Shader with Position and COLOR Vectors OUT in Matrix(Loc 0 & 1)  Geometry Shader CALCULATES normals ONCE and passes with out to FS, fragment shader  takes fragment shader,
		// ---------------------------------------------------------------------------------------------------------
		multipleBillboardShader.use();
		GLCALL(multipleBillboardShader.setVec3("lightPosition", lightPosition));
		GLCALL(multipleBillboardShader.setVec3("cameraPosition", camera.Position));
		GLCALL(multipleBillboardShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f));
		GLCALL(multipleBillboardShader.setFloat("particleSize", particleSize));
		multipleBillboardShader.setMat4("projectionMatrix", projectionMatrix);
		multipleBillboardShader.setMat4("viewMatrix", viewMatrix);
		//modelMatrix = glm::mat4(1.0f);
		//modelMatrix = glm::translate(modelMatrix, oneBillboardPosition);
		//modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
		multipleBillboardShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(billboardVAO));
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			modelMatrix = glm::mat4(1.0f);
			//glm::vec3 speed = speed + glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
			//glm::vec3 ppos = BillboardPositions[i] + speed * deltaTime;
			modelMatrix = glm::translate(modelMatrix, BillboardPositions[i]);
			float angle = 20.0f * i;
			//modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, (float)glfwGetTime(), glm::vec3(0.5f, 0.5f, 0.5f));
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Big floor
			multipleBillboardShader.setMat4("modelMatrix", modelMatrix);

			GLCALL(glDrawArrays(GL_POINTS, 0, 1));
		}

		// ------------------------------------------------------------------
		// 4 Yellow BILLBOARD Vertex Shader with Position and Normal Vectors OUT in Matrix(Loc 0 & 1) IN layout normal[] Geometry Shader and passes normals Per Point to fragment shader,
		// ---------------------------------------------------------------------------------------------------------
		GLCALL(particlewNormalShader.use());
		GLCALL(particlewNormalShader.setVec3("uniColor", uniColor));
		GLCALL(particlewNormalShader.setVec3("lightPosition", lightPosition));
		GLCALL(particlewNormalShader.setVec3("cameraPosition", camera.Position));
		GLCALL(particlewNormalShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f));
		//std::cout << "ourColor: " << stbi_failure_reason() << std::endl;
		GLCALL(particlewNormalShader.setFloat("particleSize", particleSize));
		particlewNormalShader.setMat4("projectionMatrix", projectionMatrix);
		particlewNormalShader.setMat4("viewMatrix", viewMatrix);
		
		modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, multipleBillboardPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Big floor
		particlewNormalShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(particlewNormalVAO));
		GLCALL(glDrawArrays(GL_POINTS, 0, 4));
		

		// ------------------------------------------------------------------
		// 3 RED AND 1 Green Vertex Shader with just Position in Matrix(Loc 0 ) SKIPS geometry Shader(no normal in GS), fragment shader sets viewDirection as Normalvector,
		// ---------------------------------------------------------------------------------------------------------
		testShader.use();
		testShader.setVec3("uniColor", uniColor);
		testShader.setVec3("lightPosition", lightPosition);
		testShader.setVec3("normalV", -camera.Front);
		testShader.setVec3("cameraPosition", camera.Position);
		testShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		//std::cout << "ourColor: " << stbi_failure_reason() << std::endl;
		GLCALL(testShader.setFloat("particleSize", particleSize));
		testShader.setMat4("projectionMatrix", projectionMatrix);
		testShader.setMat4("viewMatrix", viewMatrix);
		//Create many Particles with no Motion
		modelMatrix = glm::mat4(1.0f);
		//glm::vec3 speed = speed + glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
		//glm::vec3 ppos = BillboardPositions[i] + speed * deltaTime;
		modelMatrix = glm::translate(modelMatrix, testParticlePosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f)); // Big floor
		testShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(testVAO));
		GLCALL(glDrawArrays(GL_POINTS, 0, 4));
		

		// ------------------------------------------------------------------
		// 4 BLUE InstancedQuad Vertex Shader with Position and Normal Vectors OUT in Matrix(Loc 0 & 1) IN layout normal[] Geometry Shader and passes normals Per Point to fragment shader,
		// ---------------------------------------------------------------------------------------------------------
		instancedQuadShader.use();
		instancedQuadShader.setVec3("CameraRight_worldspace",camera.Right);
		instancedQuadShader.setVec3("CameraUp_worldspace", camera.WorldUp);
		instancedQuadShader.setVec3("uniColor", instancedUniColor);
		GLCALL(instancedQuadShader.setFloat("particleSize", instancedParticleSize));
		//std::cout << "ourColor: " << stbi_failure_reason() << std::endl;
		instancedQuadShader.setMat4("projectionMatrix", projectionMatrix);
		instancedQuadShader.setMat4("viewMatrix", viewMatrix);
		//Create many Particles with no Motion
		modelMatrix = glm::mat4(1.0f);
		//glm::vec3 speed = speed + glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime * 0.5f;
		//glm::vec3 ppos = BillboardPositions[i] + speed * deltaTime;
		modelMatrix = glm::translate(modelMatrix, instancedQuadPosition);
		modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f)); // Big floor
		instancedQuadShader.setMat4("modelMatrix", modelMatrix);
		GLCALL(glBindVertexArray(instancedQuadVAO));
		GLCALL(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));

		
		


		// Flip Buffers and Draw
		glfwSwapBuffers(mWindow);
		//Check for inputEvent, updte WindowState, call needed function
		glfwPollEvents();
		//Clean,delete all allocated resources
	}
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteVertexArrays(1, &particleEmitterVAO);
	glDeleteVertexArrays(1, &billboardVAO);
	glDeleteVertexArrays(1, &oneBillboardVAO);
	glDeleteVertexArrays(1, &particlewNormalVAO);
	glDeleteVertexArrays(1, &testVAO);
	glDeleteVertexArrays(1, &instancedQuadVAO);
	glDeleteVertexArrays(1, &BillboardNoGSVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &floorIndexEBO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &particleEmitterIndexEBO);
	glDeleteBuffers(1, &particleEmitterVBO);
	glDeleteBuffers(1, &billboardVBO);
	glDeleteBuffers(1, &oneBillboardVBO);
	glDeleteBuffers(1, &particlewNormalVBO);
	glDeleteBuffers(1, &testVBO);
	glDeleteBuffers(1, &instancedQuadVBO);
	glDeleteBuffers(1, &BillboardNoGSVBO);




	glfwTerminate();
	return EXIT_SUCCESS;
}


// ------------------------------------------------------------------
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	//exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// ---------------------------------------------------------------------------------------------
	// Camera Input abstracted to Camera.h
	// ---------------------------------------------------------------------------------------------

	//WASD
	// ---------------------------------------------------------------------------------------------
	//Add/Subtract direction vector from position vector for front/ back.   
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	//Mouse Input
	// ---------------------------------------------------------------------------------------------

	//Joystick Input
	// ---------------------------------------------------------------------------------------------
	glfwGetJoystickName(GLFW_JOYSTICK_1);
	//fwGetJoystickAxes(GLFW_JOYSTICK_1);
	//glfwGetJoystickButtons(GLFW_JOYSTICK_1);
	
	// Visual Input
	// ---------------------------------------------------------------------------------------------
	// Press 1 to show Wireframe
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Press 2 to Fill Rectangle
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Press M for mouseControl
	// tell GLFW to capture our mouse
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	// tell GLFW to capture our mouse
	//Press C for Camera
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


// ------------------------------------------------------------------
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
