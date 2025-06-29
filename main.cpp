#include <iostream>

#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaders/shader.h"
#include "libs/stb_image.h"

#include "camera.h"

#include "libs/glm/glm.hpp"
#include "libs/glm/gtc/matrix_transform.hpp"
#include "libs/glm/gtc/type_ptr.hpp"

#include <imGui/imgui.h>
#include <imGui/backends/imgui_impl_glfw.h>
#include <imGui/backends/imgui_impl_opengl3.h>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);
void SetLightsToShader(Shader& cubeShader);
void RenderLightEditor();

//settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

//camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//delta time vars
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//so we can play with time
static bool timePaused = false;
static float engineTime = 0.0f;
static float timeScale = 1.0f;

//light pos
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//n of point lights
const int POINT_LIGHT_AMOUNT = 4;

//mouse toggle var
bool cursorVisible = false;
static bool tabPressedLastFrame = false;

//light struct

struct LightSettings {
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
	bool enabled = true;
};

LightSettings pointLights[POINT_LIGHT_AMOUNT] = {
	{ glm::vec3(1.2f, 1.0f, 2.0f) },
	{ glm::vec3(2.0f, 1.0f, -3.0f) },
	{ glm::vec3(-1.0f, 2.0f, 1.0f) },
	{ glm::vec3(0.0f, 3.0f, 2.0f) }
};

int selectedLight = 0;

//debug settings
struct DebugSettings {
	bool showLightDirs = false;
	bool showNormals = false;
	bool showWireframe = false;
};
DebugSettings debug;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL) 
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	//capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;

	}

	// Init ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	// Init GLFW + OpenGL3 bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	io.FontGlobalScale = 1.5f;

	//depth testing
	glEnable(GL_DEPTH_TEST);

	//compile shader program
	Shader cubeShader("shaders/vertex.glsl", "shaders/fragmentLight.glsl");
	Shader lightSourceShader("shaders/vertex.glsl", "shaders/lightSourceFragmentShader.glsl");

	//triangle stuff

	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
	};

	unsigned int cubeVAO, VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);


	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//pos attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture attributte
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int diffuseMap, specularMap;

	//Texture1
	glGenTextures(1, &diffuseMap);
	glBindTexture(GL_TEXTURE_2D, diffuseMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);	

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("container2.png", &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	//Texture2
	glGenTextures(1, &specularMap);
	glBindTexture(GL_TEXTURE_2D, specularMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("container2_specular.png", &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	//Shader program instancing
	cubeShader.use();
	cubeShader.setInt("material.diffuse", 0);

	
	//Render loop
	while (!glfwWindowShouldClose(window))
	{
		//delta time calculation
		float currentFrame = glfwGetTime();

		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!timePaused) {
			engineTime += deltaTime * timeScale;
		}

		//input
		processInput(window);

		//Start imGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		//-------------------------------------------------------------------IMGUI------------------------------------------------------------
		static ImVec4 clear_color = ImVec4(0.32f, 0.27f, 0.27f, 0.5f);


		ImGui::Begin("Debug");
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "World");
		ImGui::Separator();

		// Background color picker
		ImGui::ColorEdit4("Bg Color", (float*)&clear_color, ImGuiColorEditFlags_NoInputs);

		// Wireframe mode


		ImGui::BeginGroup();
		if (ImGui::Checkbox("Wireframe Mode", &debug.showWireframe)) {}
		ImGui::SameLine();
		if (ImGui::Checkbox("Show Light Directions", &debug.showLightDirs)) {
			
		}
		ImGui::EndGroup();

		ImGui::Separator();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Time");
		ImGui::Separator();


		ImGui::Text("Time: %.2f", engineTime);
		ImGui::SameLine();
		ImGui::Checkbox("Pause Time", &timePaused);
		ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 3.0f);


		ImGui::End();

		ImGui::Begin("Performance");
		ImGui::Text("FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
		ImGui::End();

		glPolygonMode(GL_FRONT_AND_BACK, debug.showWireframe == true ? GL_LINE : GL_FILL);

		RenderLightEditor();
		SetLightsToShader(cubeShader);

		//-------------------------------------------------------------------IMGUI------------------------------------------------------------

		//render
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//bind textures

		//make cube matrix 
		cubeShader.use();
		////dir light
		//cubeShader.setVec3("dirLight.direction",-0.2, -0.2, -0.2);
		//cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		//cubeShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
		//cubeShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);

		////point light
		//for (int i = 0; i < POINT_LIGHT_AMOUNT; i++)
		//{
		//	std::string nPointLight = "pointLights[" + std::to_string(i) + "]";
		//	cubeShader.setVec3(nPointLight + ".position", pointLightPositions[i]);

		//	cubeShader.setFloat(nPointLight + ".constant", 1.0f);
		//	cubeShader.setFloat(nPointLight + ".linear", 0.09f);
		//	cubeShader.setFloat(nPointLight + ".quadratic", 0.032f);

		//	cubeShader.setVec3(nPointLight + ".ambient", 0.05f, 0.05f, 0.05f);
		//	cubeShader.setVec3(nPointLight + ".diffuse", 0.8f, 0.8f, 0.8f);
		//	cubeShader.setVec3(nPointLight + ".specular", 0.5f, 0.5f, 0.5f);
		//}

		////spotlight
		////cubeShader.setVec3("spotLight.position", camera.Position);
		////cubeShader.setVec3("spotLight.direction", camera.Front);

		////cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		////cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

		////cubeShader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
		////cubeShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
		////cubeShader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);

		//extra
		cubeShader.setVec3("viewPos", camera.Position);


		//material uniforms
		cubeShader.setInt("material.diffuse", 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);

		cubeShader.setInt("material.specular", 1);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		//--------------------------------------------------------------------------------------------------------
		//static glm::vec3 lightAmbient = glm::vec3(0.1f);
		//static glm::vec3 lightDiffuse = glm::vec3(0.8f);
		//static glm::vec3 lightSpecular = glm::vec3(1.0f);
		//static int exponent = 0; // from 0 (2^0) to 7 (2^7 = 128)

		//ImGui::Begin("Debug");

		//ImGui::TextColored(ImVec4(1, 1, 0, 1), "Lighting");
		//ImGui::Separator();

		//ImGui::SliderFloat3("Light Ambient", (float*)&lightAmbient, 0.0f, 1.0f);
		//ImGui::SliderFloat3("Light Diffuse", (float*)&lightDiffuse, 0.0f, 1.0f);
		//ImGui::SliderFloat3("Light Specular", (float*)&lightSpecular, 0.0f, 1.0f);

	
		////material uniform
		//ImGui::SliderInt("Shininess (2^x)", &exponent, 0, 7);
		//float shininess = pow(2.0f, (float)exponent); // 1.0 → 128.0
		//ImGui::Text("Shininess: %.1f", shininess);

		//ImGui::End();

		//cubeShader.setFloat("material.shininess", shininess); 

		//cubeShader.setVec3("light.ambient", lightAmbient);
		//cubeShader.setVec3("light.diffuse", lightDiffuse);
		//cubeShader.setVec3("light.specular", lightSpecular);

		//--------------------------------------------------------------------------------------------------------------------

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		cubeShader.setMat4("projection", projection);
		cubeShader.setMat4("view", view);

		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;

			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.5f, 1.0f, 0.0f));

			cubeShader.setMat4("model", model);

			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glm::mat4 model = glm::mat4(1.0f);

		//make light source cube
		lightSourceShader.use();
		lightSourceShader.setMat4("projection", projection);
		lightSourceShader.setMat4("view", view);

		for (int i = 0; i < POINT_LIGHT_AMOUNT; i++)
		{
			if (!pointLights[i].enabled) continue;

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, pointLights[i].position);
			model = glm::scale(model, glm::vec3(0.2));
			//model = glm::rotate(glm::mat4(1.0f), engineTime, glm::vec3(0.0f, 1.0f, 0.0f)) * model;

			lightSourceShader.setMat4("model", model);
			lightSourceShader.setVec3("DiffuseColor", pointLights[i].diffuse);

			//glm::vec4 localLighSource = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			//lightPos = glm::vec3(model * localLighSource);

			glBindVertexArray(lightVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		


		//kebab con carne, pollo y salsa picante 🥙

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// Render ImGui
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	//close imGui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yOffset));
}

void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn)
{
	float xPos = static_cast<float>(xPosIn);
	float yPos = static_cast<float>(yPosIn);

	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;
	lastX = xPos;
	lastY = yPos;

	if (!cursorVisible)
	camera.ProcessMouseMovement(xOffset, yOffset);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS ? GL_LINE : GL_FILL);

	//toggle mouse
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{


		if (!tabPressedLastFrame) {
			cursorVisible = !cursorVisible;

			if (cursorVisible)
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			else
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		tabPressedLastFrame = true;
	}
	else {
		tabPressedLastFrame = false;
	}

	//Camera controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FOWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void RenderLightEditor() {
	ImGui::Begin("Light Controls");

	const char* items[POINT_LIGHT_AMOUNT] = { "Light 1", "Light 2", "Light 3", "Light 4" };
	ImGui::Combo("Select Light", &selectedLight, items, POINT_LIGHT_AMOUNT);

	LightSettings& light = pointLights[selectedLight];

	ImGui::Checkbox("Enabled", &light.enabled);
	ImGui::SliderFloat3("Position", &light.position.x, -10.0f, 10.0f);
	ImGui::ColorEdit3("Ambient", &light.ambient.x);
	ImGui::ColorEdit3("Diffuse", &light.diffuse.x);
	ImGui::ColorEdit3("Specular", &light.specular.x);
	ImGui::SliderFloat("Constant", &light.constant, 0.0f, 2.0f);
	ImGui::SliderFloat("Linear", &light.linear, 0.0f, 1.0f);
	ImGui::SliderFloat("Quadratic", &light.quadratic, 0.0f, 1.0f);

	ImGui::End();
}

void SetLightsToShader(Shader& cubeShader) {
	cubeShader.use();
	cubeShader.setVec3("viewPos", camera.Position);

	cubeShader.setVec3("dirLight.direction", -0.2f, -0.2f, -0.2f);
	cubeShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
	cubeShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
	cubeShader.setVec3("dirLight.specular", 0.2f, 0.2f, 0.2f);

	for (int i = 0; i < POINT_LIGHT_AMOUNT; i++) {
		std::string base = "pointLights[" + std::to_string(i) + "]";

		if (!pointLights[i].enabled) {
			cubeShader.setVec3(base + ".ambient", 0.0f, 0.0f, 0.0f);
			cubeShader.setVec3(base + ".diffuse", 0.0f, 0.0f, 0.0f);
			cubeShader.setVec3(base + ".specular", 0.0f, 0.0f, 0.0f);
			continue;
		}

		cubeShader.setVec3(base + ".position", pointLights[i].position);
		cubeShader.setVec3(base + ".ambient", pointLights[i].ambient);
		cubeShader.setVec3(base + ".diffuse", pointLights[i].diffuse);
		cubeShader.setVec3(base + ".specular", pointLights[i].specular);
		cubeShader.setFloat(base + ".constant", pointLights[i].constant);
		cubeShader.setFloat(base + ".linear", pointLights[i].linear);
		cubeShader.setFloat(base + ".quadratic", pointLights[i].quadratic);
	}

	//cubeShader.setVec3("spotLight.position", camera.Position);
	//cubeShader.setVec3("spotLight.direction", camera.Front);
	//cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
	//cubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
	//cubeShader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
	//cubeShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
	//cubeShader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);


}
