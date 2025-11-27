#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

#include "../assets/shaders/shader.h"

#include <iostream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
void processInput(GLFWwindow *window);

const char* appWindowName = "Pennys Minecraft";
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

// camera
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0;
float fov   =  45.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// screen size
bool isFullscreen = false;
double lastToggleTime = glfwGetTime();
int windowPosX, windowPosY;
int windowWidth = SCR_WIDTH;
int windowHeight = SCR_HEIGHT;


const char* vs_path = "../assets/shaders/shader.vs";
const char* fs_path = "../assets/shaders/shader.fs";

GLFWwindow* initialiseWindow() {
    	// glfw: initialize and configure
    	// ------------------------------
    	if(!glfwInit()) {
		return NULL;
	}
    	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    	// glfw window creation
    	// --------------------
    	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, appWindowName, NULL, NULL);
    	if (window == NULL) {
        	std::cout << "Failed to create GLFW window" << std::endl;
        	glfwTerminate();
        	return NULL;
    	}
    	glfwMakeContextCurrent(window);
    	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; 
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;
	if(pitch > 89.0f) {
		pitch = 89.0f;
	} if(pitch < -89.0f) {
		pitch = -89.0f;
	}
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

//Given a slice of our chunk array:
//int[] chunk = {0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1}
//Calculate if we need to draw the faces
//
//You index by going i*4 + j to get the original block. +- 4 for left/right and +- 1 for front/back
//
//Returns a 4 boolean long array, saying which sides face into other blocks
std::array<int, 4> facingBlockXZ(std::array<int, 256> chunk, int i, int j) {
	std::array<int, 4> results = {0, 0, 0, 0};
	
	//Check all four sides
	
	int id = i*16 + j;
	
    	// FRONT face (i-1)
    	if (i > 0 && chunk[id - 16] != 0)
        	results[3] = 1;

    	// BACK face (i+1)
    	if (i < 15 && chunk[id + 16] != 0)
        	results[2] = 1;

    	// LEFT face (j-1)
    	if (j > 0 && chunk[id - 1] != 0)
        	results[1] = 1;

    	// RIGHT face (j+1)
    	if (j < 1 && chunk[id + 1] != 0)
        	results[0] = 1;

	return results;
}

void fillChunk(std::array<int, 256> chunk) {
	for(int i = 0; i < 256; i++) { 
		int random = rand();
		chunk[i] = random % 2;
	}	
}

int main() {
	std::cout << "Starting Engine!\n";
	GLFWwindow* window = initialiseWindow();
    	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        	std::cout << "Failed to initialize GLAD" << std::endl;
        	return -1;
    	}
	Shader ourShader(vs_path, fs_path);
	
	// set up vertex data (and buffer(s)) and configure vertex attributes
    	// ------------------------------------------------------------------

	float vertices[] = {
	    // Front face (z = 0.5)
	    -0.5f, -0.5f, 0.5f, 0,0,
	     0.5f, -0.5f, 0.5f, 1,0,
	     0.5f,  0.5f, 0.5f, 1,1,
	     0.5f,  0.5f, 0.5f, 1,1,
	    -0.5f,  0.5f, 0.5f, 0,1,
	    -0.5f, -0.5f, 0.5f, 0,0,

	    // Back face (z = -0.5)
	    0.5f, -0.5f, -0.5f, 0,0,
	   -0.5f, -0.5f, -0.5f, 1,0,
	   -0.5f,  0.5f, -0.5f, 1,1,
	   -0.5f,  0.5f, -0.5f, 1,1,
	    0.5f,  0.5f, -0.5f, 0,1,
	    0.5f, -0.5f, -0.5f, 0,0,

	    // Left face (x = -0.5)
	   -0.5f, -0.5f, -0.5f, 0,0,
	   -0.5f, -0.5f,  0.5f, 1,0,
	   -0.5f,  0.5f,  0.5f, 1,1,
	   -0.5f,  0.5f,  0.5f, 1,1,
	   -0.5f,  0.5f, -0.5f, 0,1,
	   -0.5f, -0.5f, -0.5f, 0,0,

	    // Right face (x = 0.5)
	    0.5f, -0.5f,  0.5f, 0,0,
	    0.5f, -0.5f, -0.5f, 1,0,
	    0.5f,  0.5f, -0.5f, 1,1,
	    0.5f,  0.5f, -0.5f, 1,1,
	    0.5f,  0.5f,  0.5f, 0,1,
	    0.5f, -0.5f,  0.5f, 0,0,

	    // Bottom face (y = -0.5)
	   -0.5f, -0.5f, -0.5f, 0,0,
	    0.5f, -0.5f, -0.5f, 1,0,
	    0.5f, -0.5f,  0.5f, 1,1,
	    0.5f, -0.5f,  0.5f, 1,1,
	   -0.5f, -0.5f,  0.5f, 0,1,
	   -0.5f, -0.5f, -0.5f, 0,0,

	    // Top face (y = 0.5)
	   -0.5f, 0.5f,  0.5f, 0,0,
	    0.5f, 0.5f,  0.5f, 1,0,
	    0.5f, 0.5f, -0.5f, 1,1,
	    0.5f, 0.5f, -0.5f, 1,1,
	   -0.5f, 0.5f, -0.5f, 0,1,
	   -0.5f, 0.5f,  0.5f, 0,0
	};


	// world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// load and create a texture 
	// -------------------------
	unsigned int texture1, texture2, texture3;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("../assets/textures/blocks/grass_block.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	// texture 2
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("../assets/textures/blocks/grass_block_side.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	//texture 3
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	data = stbi_load("../assets/textures/blocks/dirt_block.png", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use(); 
	ourShader.setInt("texture1", 0);

	while (!glfwWindowShouldClose(window)) {
	        float currentFrame = static_cast<float>(glfwGetTime());
        	deltaTime = currentFrame - lastFrame;
        	lastFrame = currentFrame;

		    // input
		    // -----
		    processInput(window);
		    glfwSetCursorPosCallback(window, mouse_callback);

		    // render
		    // ------
		    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		    // bind textures on corresponding texture units
		    glActiveTexture(GL_TEXTURE0);
		    glBindTexture(GL_TEXTURE_2D, texture1);
		    glActiveTexture(GL_TEXTURE1);
		    glBindTexture(GL_TEXTURE_2D, texture2);

		    // activate shader
		    ourShader.use();
		    glEnable(GL_DEPTH_TEST);
		    //glCullFace(GL_BACK);
		    glEnable(GL_CULL_FACE);
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		    
		    // pass projection matrix to shader (note that in this case it could change every frame)
		    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		    ourShader.setMat4("projection", projection);

		    // camera/view transformation
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		ourShader.setMat4("view", view);
		
		std::array<int, 256> chunk; 		
		fillChunk(chunk);
		
		std::array<int, 256> layer_2;
		fillChunk(layer_2);
		// render boxes
        	glBindVertexArray(VAO);
        	for (unsigned int i = 0; i < 16; i++) {
			for(unsigned int j = 0; j < 16; j ++) {
				for(unsigned int z = 0; z < 2; z++) {
					// calculate the model matrix for each object and pass it to shader before drawing
					if (z == 0 && chunk[i*16 + j] == 0) {
						continue;
					} else if (z == 1 && layer_2[i*16 + j] == 0) {
						continue;
					}
					glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
					model = glm::translate(model, glm::vec3(float(i), -float(z), float(j)));//cubePositions[i]);
					//float angle = 20.0f * i;
					//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
					
					ourShader.setMat4("model", model);
					ourShader.setInt("textureSampler", 0);
					glActiveTexture(GL_TEXTURE0);
					if(z == 1 && chunk[i*16 + j] != 0) {
						glBindTexture(GL_TEXTURE_2D, texture3); // dirt
					} else {
						glBindTexture(GL_TEXTURE_2D, texture2); // grass side texture
					}
					std::array<int, 4> results;
					if(z == 0) {
						results = facingBlockXZ(chunk, i, j);
					} else {
						results = facingBlockXZ(layer_2, i, j);	
					}
					//Front 
					if (results[0] == false) {
						glDrawArrays(GL_TRIANGLES, 0, 6);
					}
					//Back
					if (results[1] == false) {
						glDrawArrays(GL_TRIANGLES, 6, 6);
					}
					//Left
					if (results[3] == false) {
						glDrawArrays(GL_TRIANGLES, 12, 6);
					}
					//Right
					if (results[2] == false) {
						glDrawArrays(GL_TRIANGLES, 18, 6);	
					}
					
					glBindTexture(GL_TEXTURE_2D, texture3); // dirt
					glDrawArrays(GL_TRIANGLES, 24, 6);
					
					if (z == 1 && chunk[i*16 + j] != 0) {	
						continue;
					}
					glBindTexture(GL_TEXTURE_2D, texture1); // grass top
					glDrawArrays(GL_TRIANGLES, 30, 6);
				}
                	}
    		}
		    glfwSwapBuffers(window);
            glfwPollEvents();	
	};

	glfwTerminate();
	return 0;
}
