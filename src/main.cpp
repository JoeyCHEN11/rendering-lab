#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using glm::mat4;
using glm::vec3;
using glm::normalize;
using glm::cross;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void updateDeltaTime();
void initImgui(GLFWwindow* window);
void showGui();
unsigned int loadTexture(char const * path);

static unsigned int SCR_WIDTH = 1800;
static unsigned int SCR_HEIGHT = 1800;

// Exercise 4
static float mixValue = 0.2f;

// delta time
static float deltaTime = 0.0f;
static float lastFrameTime = 0.0f;

// mouse positions
static float lastX = SCR_WIDTH / 2;
static float lastY = SCR_HEIGHT / 2;
static bool firstMouse = true;
static Camera myCamera({ 0.0f, 0.0f, 3.0f });

// point light position
static vec3 lightPos(1.5f, 1.5f, 1.5f);
static int spotLightCount = 4;

static vec3 backgroundColor { 0.1f, 0.1f, 0.1f };

int main()
{
	// Initialize and configure the opengl version, mode
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "A Box", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// set behavior of resizing the window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// gl global status
	glEnable(GL_DEPTH_TEST);

	// set stbi image loading direction
	stbi_set_flip_vertically_on_load(true);

	// initiate imgui
	initImgui(window);
	ImGuiIO& io = ImGui::GetIO();

	float boxVertices[]{
		// positions		 // normal vectors  //texture coords
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, 0.0f,  1.0f,  0.0f, 0.0f, 1.0f
	};

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// genereate and bind a VAO

	unsigned int lightVAO;

	glGenVertexArrays(1, &lightVAO);

	unsigned int lightVBO;
	glGenBuffers(1, &lightVBO);

	// buffer vertex data via VBO
	// bind the current vertex buffer to VBO
	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

	glBindVertexArray(lightVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	Shader lightCubeShader{ "assets/shaders/light.vert", "assets/shaders/light.frag" };
	lightCubeShader.use();
	lightCubeShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
	lightCubeShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
	lightCubeShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

	Model packModel = Model("assets/models/backpack/backpack.obj");

	Shader packShader("assets/shaders/backpack.vert", "assets/shaders/backpack.frag");

	while (!glfwWindowShouldClose(window))
	{
		// rendering
		// set clear color configuration
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
		// clear color buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Start the Dear ImGui frame
		//--------------------------------
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		showGui();
		//--------------------------------

		updateDeltaTime();

		// input process
		processInput(window);

		// line mode of fill mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		mat4 projection = glm::perspective(glm::radians(myCamera.Fov), (float) SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		mat4 view = myCamera.GetViewMatrix();


		packShader.use();
		packShader.setVec3("viewPos", myCamera.Position);
		packShader.setFloat("material.shininess", 256.0f);

		// directional light (Warm Light)
		packShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		packShader.setVec3("dirLight.ambient", 0.1f, 0.08f, 0.05f);
		packShader.setVec3("dirLight.diffuse", 0.7f, 0.6f, 0.4f);
		packShader.setVec3("dirLight.specular", 0.5f, 0.45f, 0.35f);


		packShader.setInt("spotLightCount", spotLightCount);
		for (int i = 0; i < spotLightCount; i++)
		{
			std::string idx_s = std::to_string(i);

			packShader.setVec3("pointLights[" + idx_s + "].position", pointLightPositions[i]);
			packShader.setVec3("pointLights[" + idx_s + "].ambient", 0.05f, 0.05f, 0.05f);
			packShader.setVec3("pointLights[" + idx_s + "].diffuse", 0.8f, 0.5f, 0.5f);
			packShader.setVec3("pointLights[" + idx_s + "].specular", 1.0f, 1.0f, 1.0f);
			packShader.setFloat("pointLights[" + idx_s + "].constant", 1.0f);
			packShader.setFloat("pointLights[" + idx_s + "].linear", 0.09f);
			packShader.setFloat("pointLights[" + idx_s + "].quadratic", 0.032f);
		}

		packShader.setVec3("spotLight.position", myCamera.Position);
		packShader.setVec3("spotLight.direction", myCamera.Front);
		packShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		packShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		packShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		packShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		packShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		packShader.setFloat("spotLight.constant", 1.0f);
		packShader.setFloat("spotLight.linear", 0.09f);
		packShader.setFloat("spotLight.quadratic", 0.032f);

		// set mvp and draw boxes
		packShader.setMat4("projection", projection);
		packShader.setMat4("view", view);

		mat4 model{ 1.0 };
		packShader.setMat4("model", model);
		// change light color
		packModel.Draw(packShader);



		lightCubeShader.use();

		//float angle = static_cast<float>(45.0 * glm::radians(glfwGetTime()));
		//lightPos = { cos(angle), lightPos.y, sin(angle) };

		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		// draw lights
		glBindVertexArray(lightVAO);
		for (int i = 0; i < 4; i++)
		{
			mat4 lightModel{ 1.0f };
			lightModel = glm::translate(lightModel, pointLightPositions[i]);
			lightModel = glm::scale(lightModel, vec3(0.2f));
			lightCubeShader.setMat4("model", lightModel);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			
		}

		// rendering imgui frame
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// handle events, refresh buffer
		glfwPollEvents();
		glfwSwapBuffers(window);


	}

	// shutdown imgui
	//--------------------------------
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	//--------------------------------
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// set the size of rendering size
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
		return;
	float fxpos = static_cast<float>(xpos);
	float fypos = static_cast<float>(ypos);

	// avoid sudden jump
	if (firstMouse)
	{
		lastX = fxpos;
		lastY = fypos;
		firstMouse = false;
	}

	float xOffset = fxpos - lastX;
	float yOffset = lastY - fypos; // in window: y range from up to down
	//std::cout << "\r{ last y:" << lastY << "current y:" << ypos << std::string(50, ' ') << std::flush;;

	lastX = fxpos;
	lastY = fypos;

	myCamera.ProcessMouseMovement(xOffset, yOffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;
	myCamera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window)
{
	ImGuiIO io = ImGui::GetIO();

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		if (!io.WantCaptureMouse)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			firstMouse = true;

		}
	}
	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			mixValue = std::min(mixValue + 0.001f, 1.0f);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			mixValue = std::max(mixValue - 0.001f, 0.0f);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			myCamera.ProcessKeyboard(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			myCamera.ProcessKeyboard(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			// go left
			myCamera.ProcessKeyboard(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			myCamera.ProcessKeyboard(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			// go up (simulate the movement in Unity Scene)
			myCamera.ProcessKeyboard(UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			myCamera.ProcessKeyboard(DOWN, deltaTime);
	}
}

void updateDeltaTime()
{
	float currentTime = static_cast<float>(glfwGetTime());
	deltaTime = currentTime - lastFrameTime;
	lastFrameTime = currentTime;
}

unsigned int loadTexture(char const* path)
{
	// generate texture
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int texWidth, texHeight, nrChannels;
	unsigned char* texData = stbi_load(path, &texWidth, &texHeight, &nrChannels, 0);
	if (texData)
	{
		GLenum format = GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		//std::cout << nrChannels << std::endl;
		glBindTexture(GL_TEXTURE_2D, textureID);
		// store (copy) texure data in bound texture object
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set warpping/filtering options (for current texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load texture " << path << std::endl;
	}
	// free data pointer
	stbi_image_free(texData);
	return textureID;
}

void initImgui(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	io.FontGlobalScale = 1.5f;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void showGui()
{
	ImGui::Begin("Render Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::ColorEdit3("Background Color", glm::value_ptr(backgroundColor));

	ImGui::End();
}