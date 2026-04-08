#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using glm::mat4;
using glm::vec3;
using glm::normalize;
using glm::cross;

static const int POINTLIGHT_COUNT = 4;

struct RenderConfig
{
	struct General
	{
		unsigned int screenWidth = 1920;
		unsigned int screenHeight = 1080;

		// screen clear color
		glm::vec3 clearColor { 0.1f, 0.1f, 0.1f };

	} general;

	CameraConfig camera;

	struct Lights
	{
		LightConfig directionalLight;
		LightConfig pointLights[POINTLIGHT_COUNT];
		LightConfig spotLight;
	} lights;

	struct Shader
	{
		bool useToonShader = false;
		float toonThreshold = 0.05;
		float toonSmoothness = 0.05;
	} shader;

	struct Model
	{
		bool isPackActive = true;
		glm::vec3 posPack { 2.5f, 0.0f, 0.0f };
		
		bool isAliciaActive = true;
		glm::vec3 posAlicia { 0.0f, -2.5f, 0.0f };

	} models;
};
static RenderConfig renderConfig;
// delta time
static float deltaTime = 0.0f;
static float lastFrameTime = 0.0f;

// mouse positions
static float lastX = renderConfig.general.screenWidth / 2;
static float lastY = renderConfig.general.screenHeight / 2;
static bool firstMouse = true;
static Camera myCamera({ 0.0f, 0.0f, 3.0f });

// point light position
static vec3 lightPos(1.5f, 1.5f, 1.5f);

// shaders
static Shader* phongShader = nullptr;
static Shader* toonShader;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void setupLights(const RenderConfig::Lights& config, const Shader& shader);
Shader* setShader( const RenderConfig::Shader& shaderConfig);
void updateDeltaTime();
void initImgui(GLFWwindow* window);
void showGui();
unsigned int loadTexture(char const * path);


/// <summary>
/// Entry point for the OpenGL rendering application. Initializes GLFW, GLAD, ImGui, loads shaders and models, and runs the main rendering loop.
/// </summary>
/// <returns>Returns 0 on successful execution, or -1 if initialization fails.</returns>
int main()
{
	// Initialize and configure the opengl version, mode
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(renderConfig.general.screenWidth, renderConfig.general.screenHeight, "Rendering Lab", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// set window icon
	int width, height, nrChannels;
	unsigned char* pixels = stbi_load("assets/icons/game-icons--soap-experiment.png", &width, &height, &nrChannels, 4);
	if (pixels == nullptr)
		std::cout << "Failed to load icon image" << std::endl;
	GLFWimage images[1];
	images[0].width = width;
	images[0].height = height;
	images[0].pixels = pixels;
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(pixels);

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

	phongShader = new Shader("assets/shaders/phong.vert", "assets/shaders/phong.frag");
	toonShader = new Shader("assets/shaders/toon.vert", "assets/shaders/toon.frag");

	// gl global status
	glEnable(GL_DEPTH_TEST);


	// initiate imgui
	initImgui(window);
	ImGuiIO& io = ImGui::GetIO();

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// initiate lights
	renderConfig.lights.directionalLight.direction = { -0.2f, -1.0f, -0.3f };
	renderConfig.lights.directionalLight.isActive = true;

	for (int i = 0; i < POINTLIGHT_COUNT; i++ )
		renderConfig.lights.pointLights[i].position = pointLightPositions[i];

	Model lightBulb = Model("assets/models/sphere/scene.gltf");
	Shader bulbShader{ "assets/shaders/light.vert", "assets/shaders/light.frag" };

	phongShader->use();
	phongShader->setFloat("material.shininess", 256.0f);

	toonShader->use();
	toonShader->setFloat("material.shininess", 256.0f);

	// flip stbi image loading direction
	stbi_set_flip_vertically_on_load(true);
	Model packModel = Model("assets/models/backpack/backpack.obj");
	stbi_set_flip_vertically_on_load(false);

	Model aliciaModel = Model("assets/models/Alicia/VRM/AliciaSolid.vrm");

	while (!glfwWindowShouldClose(window))
	{
		// rendering
		// set clear color configuration
		glClearColor(renderConfig.general.clearColor.x, renderConfig.general.clearColor.y, renderConfig.general.clearColor.z, 1.0f);
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
		myCamera.updateCameraConfig(renderConfig.camera);

		// input process
		processInput(window);

		// line mode of fill mode
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		mat4 projection = glm::perspective(
			glm::radians(myCamera.Fov), 
			(float) renderConfig.general.screenWidth / 
					renderConfig.general.screenHeight,
			0.1f, 100.0f
		);
		mat4 view = myCamera.GetViewMatrix();
		mat4 model{ 1.0f };

		//draw backpack
		if (renderConfig.models.isPackActive)
		{
			Shader* packShader = setShader(renderConfig.shader);
			packShader->use();
			// set mvp and draw boxes
			packShader->setMat4("projection", projection);
			packShader->setMat4("view", view);

			model = { 1.0f };
			model = glm::translate(model, renderConfig.models.posPack);
			model = glm::scale(model, vec3(0.5f));
			packShader->setMat4("model", model);

			// change light color
			setupLights(renderConfig.lights, *packShader);

			packModel.Draw(*packShader);
		}

		// draw Alicia
		if (renderConfig.models.isAliciaActive)
		{
			Shader* aliciaShader = setShader(renderConfig.shader);
			aliciaShader->use();
			aliciaShader->setMat4("projection", projection);
			aliciaShader->setMat4("view", view);

			model = { 1.0f };
			model = glm::translate(model, renderConfig.models.posAlicia);
			model = glm::scale(model, vec3(2.0f));
			model = glm::rotate(model, glm::radians(180.0f), { 0.0f, 1.0f, 0.0f });

			aliciaShader->setMat4("model", model);
			//aliciaShader->setVec3("viewPos", myCamera.Position);
			setupLights(renderConfig.lights, *aliciaShader);
			aliciaModel.Draw(*aliciaShader);
		}

		// draw point lights
		bulbShader.use();
		bulbShader.setMat4("projection", projection);
		bulbShader.setMat4("view", view);

		for (const LightConfig& pointLight : renderConfig.lights.pointLights)
		{
			if (!pointLight.isActive)
				continue;

			mat4 lightModel{ 1.0f };
			lightModel = glm::translate(lightModel, pointLight.position);
			lightModel = glm::scale(lightModel, vec3(0.03f));

			bulbShader.setMat4("model", lightModel);
			bulbShader.setVec3("light.ambient", pointLight.ambient);
			bulbShader.setVec3("light.diffuse", pointLight.diffuse);
			//bulbShader.setVec3("light.specular", pointLight.specular);

			lightBulb.Draw(bulbShader);
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

/// <summary>
/// Callback function that handles framebuffer resize events for a GLFW window.
/// </summary>
/// <param name="window">Pointer to the GLFW window whose framebuffer was resized.</param>
/// <param name="width">The new width of the framebuffer in pixels.</param>
/// <param name="height">The new height of the framebuffer in pixels.</param>
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// set the size of rendering size
	renderConfig.general.screenWidth = width;
	renderConfig.general.screenHeight = height;
	glViewport(0, 0, width, height);
}

/// <summary>
/// GLFW callback function that handles mouse movement for camera control.
/// </summary>
/// <param name="window">Pointer to the GLFW window receiving the mouse input.</param>
/// <param name="xpos">The current x-coordinate of the mouse cursor in screen coordinates.</param>
/// <param name="ypos">The current y-coordinate of the mouse cursor in screen coordinates.</param>
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

/// <summary>
/// Callback function for handling mouse scroll events to adjust the camera field of view.
/// </summary>
/// <param name="window">Pointer to the GLFW window that received the scroll event.</param>
/// <param name="xoffset">The horizontal scroll offset.</param>
/// <param name="yoffset">The vertical scroll offset used to adjust the camera FOV.</param>
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	// mouse scroll
	renderConfig.camera.fov = glm::clamp(renderConfig.camera.fov - static_cast<float>(yoffset), CAM_FOV_MIN, CAM_FOV_MAX);
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

/// <summary>
/// Setup Light Paramaters
/// </summary>
/// <param name="lights"></param>
/// <param name="shader"></param>
void setupLights(const RenderConfig::Lights& lights, const Shader& shader)
{
	int directActive = (int)lights.directionalLight.isActive;

	// directional light 
	//std::cout << directActive << std::endl;
	shader.setInt("directLightCount", directActive);
	shader.setVec3("dirLight[0].direction", lights.directionalLight.direction );
	shader.setVec3("dirLight[0].ambient", lights.directionalLight.ambient );
	shader.setVec3("dirLight[0].diffuse", lights.directionalLight.diffuse );
	shader.setVec3("dirLight[0].specular", lights.directionalLight.specular );

	int pointCnt = 0;
	for (const LightConfig& l : lights.pointLights)
		if (l.isActive) pointCnt++;
	//std::cout << pointCnt << std::endl;
	shader.setInt("pointLightCount", pointCnt);
	for (int i = 0; i < pointCnt; i++)
	{
		std::string idx_s = std::to_string(i);

		shader.setVec3("pointLights[" + idx_s + "].position", lights.pointLights[i].position);
		shader.setVec3("pointLights[" + idx_s + "].ambient", lights.pointLights[i].ambient);
		shader.setVec3("pointLights[" + idx_s + "].diffuse", lights.pointLights[i].diffuse);
		shader.setVec3("pointLights[" + idx_s + "].specular", lights.pointLights[i].specular);
		shader.setFloat("pointLights[" + idx_s + "].constant", lights.pointLights[i].constant);
		shader.setFloat("pointLights[" + idx_s + "].linear", lights.pointLights[i].linear);
		shader.setFloat("pointLights[" + idx_s + "].quadratic", lights.pointLights[i].quadratic);
	}

	int spotActive = (int)lights.spotLight.isActive;
	shader.setInt("spotLightCount", spotActive);
	shader.setVec3("spotLight[0].position", myCamera.Position);
	shader.setVec3("spotLight[0].direction", myCamera.Front);
	shader.setFloat("spotLight[0].cutOff", lights.spotLight.cutOff);
	shader.setFloat("spotLight[0].outerCutOff", lights.spotLight.outerCutOff);
	shader.setVec3("spotLight[0].ambient", lights.spotLight.ambient);
	shader.setVec3("spotLight[0].diffuse", lights.spotLight.diffuse);
	shader.setVec3("spotLight[0].specular", lights.spotLight.specular);
	shader.setFloat("spotLight[0].constant", lights.spotLight.constant);
	shader.setFloat("spotLight[0].linear", lights.spotLight.linear);
	shader.setFloat("spotLight[0].quadratic", lights.spotLight.quadratic);
}

/// <summary>
/// Configures and returns a shader based on the configuration.
/// </summary>
/// <param name="shaderConfig">The shader configuration containing the shader type and parameters to apply.</param>
/// <returns>A pointer to the configured shader (either toon shader or phong shader).</returns>
Shader* setShader(const RenderConfig::Shader& shaderConfig)
{
	Shader* shader;
	if (shaderConfig.useToonShader) {
		shader = toonShader;
		shader->use();
		shader->setFloat("toonThreshold", shaderConfig.toonThreshold);
		shader->setFloat("toonSmoothness", shaderConfig.toonSmoothness);
	}
	else
	{
		shader = phongShader;
		shader->use();
		shader->setVec3("viewPos", myCamera.Position);
	}

	return shader;
}


void updateDeltaTime()
{
	float currentTime = static_cast<float>(glfwGetTime());
	deltaTime = currentTime - lastFrameTime;
	lastFrameTime = currentTime;
}

void initImgui(GLFWwindow* window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // IF using Docking Branch

	io.FontGlobalScale = 1.5f;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void showGui()
{
	ImGui::Begin("Render Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::ColorEdit3("Background Color", glm::value_ptr(renderConfig.general.clearColor));
	if (ImGui::CollapsingHeader("Camera Setting", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::SliderFloat("FOV", &renderConfig.camera.fov, CAM_FOV_MIN, CAM_FOV_MAX);
		ImGui::SliderFloat("Sensitivity", &renderConfig.camera.mouseSensitivity, 0.01f, 0.1f);
		ImGui::SliderFloat("Speed", &renderConfig.camera.movementSpeed, 0.1f, 5.0f);

	}

	if (ImGui::CollapsingHeader("Model Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID("Backpack");
		ImGui::Checkbox("##Toggle", &renderConfig.models.isPackActive);
		ImGui::SameLine();
		if (ImGui::TreeNodeEx("Backpack", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!renderConfig.models.isPackActive)
				ImGui::BeginDisabled();
			ImGui::SliderFloat3("Position", glm::value_ptr(renderConfig.models.posPack), -10.0f, 10.0f);

			if (!renderConfig.models.isPackActive)
				ImGui::EndDisabled();
			ImGui::TreePop();
		}
		ImGui::PopID();

		ImGui::PushID("Alicia");
		ImGui::Checkbox("##Toggle", &renderConfig.models.isAliciaActive);
		ImGui::SameLine();
		if (ImGui::TreeNodeEx("Alicia", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!renderConfig.models.isAliciaActive)
				ImGui::BeginDisabled();
			ImGui::SliderFloat3("Position", glm::value_ptr(renderConfig.models.posAlicia), -10.0f, 10.0f);
		
			if (!renderConfig.models.isAliciaActive)
				ImGui::EndDisabled();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}


	if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushID("DirLight");
		LightConfig& dirLight = renderConfig.lights.directionalLight;

		ImGui::Checkbox("##Toggle", &dirLight.isActive);
		ImGui::SameLine();

		if (ImGui::TreeNodeEx("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
		{

			if (!dirLight.isActive) {
				ImGui::BeginDisabled();
			}

			ImGui::ColorEdit3("Color", glm::value_ptr(dirLight.diffuse));

			ImGui::DragFloat3("Direction", glm::value_ptr(dirLight.direction), 0.01f, -1.0f, 1.0f);

			if (!dirLight.isActive) {
				ImGui::EndDisabled();
			}

			ImGui::TreePop();
		}
		
		ImGui::PopID();


		ImGui::PushID("PointLights");
		if (ImGui::TreeNodeEx("Point Lights", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < POINTLIGHT_COUNT; i++)
			{
				std::string idStr =  std::to_string(i);
				ImGui::PushID(("PointLight" + idStr).c_str());

				LightConfig& pointLight = renderConfig.lights.pointLights[i];

				ImGui::Checkbox("##Toggle", &pointLight.isActive);
				ImGui::SameLine();

				if (ImGui::TreeNodeEx(("Point Light " + idStr).c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (!pointLight.isActive) 
						ImGui::BeginDisabled();

					ImGui::ColorEdit3("Color", glm::value_ptr(pointLight.diffuse));

					ImGui::DragFloat3("Position", glm::value_ptr(pointLight.position), 0.01f);

					if (!pointLight.isActive)
						ImGui::EndDisabled();

					ImGui::TreePop();
				}

				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();

		ImGui::PushID("SpotLight");
		LightConfig& spotLight = renderConfig.lights.spotLight;

		ImGui::Checkbox("##Toggle", &spotLight.isActive);
		ImGui::SameLine();

		if (ImGui::TreeNodeEx("Flashlight", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!spotLight.isActive)
				ImGui::BeginDisabled();

				ImGui::ColorEdit3("Color", glm::value_ptr(spotLight.diffuse));



			if (!spotLight.isActive)
				ImGui::EndDisabled();

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Shader Settings"), ImGuiTreeNodeFlags_DefaultOpen)
	{
		ImGui::PushID("Shaders");
		RenderConfig::Shader& shaderConfig = renderConfig.shader;
		ImGui::Checkbox("##Toggle", &shaderConfig.useToonShader);
		ImGui::SameLine();
		
		if (ImGui::TreeNodeEx("Use Toon Shader", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (!shaderConfig.useToonShader)
				ImGui::BeginDisabled();

			ImGui::SliderFloat("Toon Threshold", &shaderConfig.toonThreshold, 0.0f, 1.0f);
			ImGui::SliderFloat("Toon Smoothness", &shaderConfig.toonSmoothness, 0.1f, 0.5f);

			if (!shaderConfig.useToonShader)
				ImGui::EndDisabled();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}

	ImGui::End();
}