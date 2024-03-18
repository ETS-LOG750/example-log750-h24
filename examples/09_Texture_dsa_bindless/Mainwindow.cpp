#include "MainWindow.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const GLuint NumVertices = 4;
const GLuint NumColors = 4;
const GLuint NumNormals = 4;
const GLuint NumUvs = 4;

MainWindow::MainWindow() :
	m_at(glm::vec3(0, 0,0)),
	m_up(glm::vec3(0, 1, 0))
{
	updateCameraEye();
}

void MainWindow::FramebufferSizeCallback(int width, int height) {
	m_proj = glm::perspective(45.0f, float(width) / height, 0.01f, 100.0f);
}

int MainWindow::Initialisation()
{
	// OpenGL version (usefull for imGUI and other libraries)
	const char* glsl_version = "#version 460 core";

	// glfw: initialize and configure
    // ------------------------------
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Texture", NULL, NULL);
	if (m_window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(m_window);
	InitializeCallback();

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return 2;
	}

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Other openGL initialization
	// -----------------------------
	return InitializeGL();
}

void MainWindow::InitializeCallback() {
	glfwSetWindowUserPointer(m_window, reinterpret_cast<void*>(this));
	glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
		MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		w->FramebufferSizeCallback(width, height);
		});
}

int MainWindow::InitializeGL()
{
	GLfloat vertices[NumVertices][3] = {
		{-3, -3, -1},
		{ 3, -3, -1},
		{-3,  3, -1},
		{ 3,  3, -1}
	};
	GLfloat uvs[NumColors][2] = {
		{ 0, 0 },
		{ 3, 0 },
		{ 0, 3 },
		{ 3, 3 }
	};
	GLfloat normals[NumNormals][3] = {
		{ 0, 0, 1 },
		{ 0, 0, 1 },
		{ 0, 0, 1 },
		{ 0, 0, 1 }
	};

	glGenVertexArrays(NumVAOs, m_VAOs);
	glBindVertexArray(m_VAOs[Triangles]);
	glGenBuffers(NumBuffers, m_buffers);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs) + sizeof(normals),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uvs), uvs);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(uvs), sizeof(normals), normals);

	// Load texture
	std::string assets_dir = ASSETS_DIR;
	std::string image_diffuse_path = assets_dir + "wood_floor_deck_diff_1k.jpg";
	std::string image_arm_path = assets_dir + "wood_floor_deck_arm_1k.jpg"; // Ambiant + Roughness + Metallic
	// Autre texture:
	// image_diffuse_path = assets_dir + "slab_tiles_diff_1k.jpg";
	// image_arm_path = assets_dir + "slab_tiles_arm_1k.jpg";

	if (!loadTexture(image_diffuse_path, m_textureDiffuseID, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)) {
		std::cerr << "Unable to load texture: " << image_diffuse_path << std::endl;
		return 4;
	}
	else {
		std::cout << "Load texture -- OpenGL ID: " << m_textureDiffuseID << "\n";
	}
	m_handleDiffuse = glGetTextureHandleARB(m_textureDiffuseID);
	glMakeTextureHandleResidentARB(m_handleDiffuse);

	if (!loadTexture(image_arm_path, m_textureARMID, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)) {
		std::cerr << "Unable to load texture: " << image_arm_path << std::endl;
		return 4;
	}
	else {
		std::cout << "Load texture -- OpenGL ID: " << m_textureARMID << "\n";
	}
	m_handleARM = glGetTextureHandleARB(m_textureARMID);
	glMakeTextureHandleResidentARB(m_handleARM);

	// build and compile our shader program
	const std::string directory = SHADERS_DIR;
	m_mainShader = std::make_unique<ShaderProgram>();
	bool mainShaderSuccess = true;
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "triangles.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "triangles.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	// Load uniform variables
	m_uniforms.mvMatrix = m_mainShader->uniformLocation("mvMatrix");
	m_uniforms.projMatrix = m_mainShader->uniformLocation("projMatrix");
	m_uniforms.normalMatrix = m_mainShader->uniformLocation("normalMatrix");
	m_uniforms.textureDiffuse = m_mainShader->uniformLocation("texDiffuse");
	m_uniforms.textureARM = m_mainShader->uniformLocation("texARM");
	m_uniforms.activateARM = m_mainShader->uniformLocation("activateARM");
	if (m_uniforms.mvMatrix == -1 || m_uniforms.projMatrix == -1 || m_uniforms.normalMatrix == -1 ||
		m_uniforms.textureDiffuse == -1 || m_uniforms.textureARM == -1 || m_uniforms.activateARM == -1) {
		std::cerr << "Error when loading uniform variables\n";
		return 5;
	}

	// Setup shader variables
	glUseProgram(m_mainShader->programId());

	// VAO informations
	int locPos = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(locPos, 3, GL_FLOAT,GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(locPos);

	int locUV = m_mainShader->attributeLocation("vUV");
	glVertexAttribPointer(locUV, 2, GL_FLOAT,GL_FALSE, 0, BUFFER_OFFSET(sizeof(vertices)));
	glEnableVertexAttribArray(locUV);

	int locNormal = m_mainShader->attributeLocation("vNormal");
	glVertexAttribPointer(locNormal, 3, GL_FLOAT,GL_TRUE, 0, BUFFER_OFFSET(sizeof(vertices) + sizeof(uvs)));
	glEnableVertexAttribArray(locNormal);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	updateCameraEye();
	FramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);

	return 0;
}

void MainWindow::RenderImgui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//imgui 
	{
		ImGui::Begin("Texture");

		const float SlidersDefaultVal = 50.0f;
		static float LeftRight = SlidersDefaultVal;
		static float UpDown = SlidersDefaultVal;
		static float ForwardBackward = SlidersDefaultVal;
		
		const char* items[] = {
			"REPEAT",
			"CLAMP_EDGE",
			"CLAMP_BORDER",
		};
		bool changed = ImGui::Combo("Mode", &m_mode, items, IM_ARRAYSIZE(items));
		if (changed && m_mode == 0) {
			glTextureParameteri(m_textureDiffuseID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_textureDiffuseID, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTextureParameteri(m_textureARMID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_textureARMID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else if (changed && m_mode == 1) {
			glTextureParameteri(m_textureDiffuseID,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureDiffuseID,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureARMID,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(m_textureARMID,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else if (changed && m_mode == 2) {
			glTextureParameteri(m_textureDiffuseID,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(m_textureDiffuseID,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTextureParameteri(m_textureARMID,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(m_textureARMID,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		}
		ImGui::Checkbox("Activate (ARM)", &m_activateARM);

		bool updateCamera = ImGui::SliderFloat("Left Right Slider", &m_longitude, -180.0f, 180.0f);
		updateCamera |= ImGui::SliderFloat("Up Down Slider", &m_latitude, -89.f, 89.f);
		updateCamera |= ImGui::SliderFloat("Forward Backward Slider", &m_distance, 2.f, 14.f);
		if (updateCamera) {
			updateCameraEye();
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainWindow::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(m_VAOs[Triangles]);

	glUseProgram(m_mainShader->programId());
	m_mainShader->setBool(m_uniforms.activateARM, m_activateARM);

	// Note that binding point are configured inside the shader
	glUniformHandleui64ARB(m_uniforms.textureDiffuse, m_handleDiffuse);
	glUniformHandleui64ARB(m_uniforms.textureARM, m_handleARM);
	
	// Camera specification (for the shader)
	glm::mat4 lookAt = glm::lookAt(m_eye, m_at, m_up);
	m_mainShader->setMat4(m_uniforms.mvMatrix, lookAt);
	m_proj = glm::perspective(45.0f, float(SCR_WIDTH) / SCR_HEIGHT, 0.01f, 100.0f);
	m_mainShader->setMat4(m_uniforms.projMatrix, m_proj);
	glm::mat3 NoramlMat = glm::inverseTranspose(glm::mat3(lookAt));
	m_mainShader->setMat3(m_uniforms.normalMatrix, NoramlMat);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, NumVertices);
}


int MainWindow::RenderLoop()
{
	while (!glfwWindowShouldClose(m_window))
	{
		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		RenderScene();
		RenderImgui();

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_window);
	glfwTerminate();

	return 0;
}

void MainWindow::updateCameraEye()
{
	m_eye = glm::vec3(0, 0, m_distance);
	glm::mat4 longitude(1), latitude(1);
	latitude= glm::rotate(latitude, glm::radians(m_latitude), glm::vec3(1, 0, 0));
	longitude= glm::rotate(longitude, glm::radians(m_longitude), glm::vec3(0, 1, 0));
	m_eye = longitude * latitude * glm::vec4(m_eye,1);
}

bool MainWindow::loadTexture(const std::string& path, unsigned int& textureID, GLint uvMode, GLint minMode, GLint magMode)
{
	// OpenGL 4.6 -- need to specify the texture type
	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

	// Ask the library to flip the image horizontally
	// This is necessary as TexImage2D assume "The first element corresponds to the lower left corner of the texture image"
	// whereas stb_image load the image such "the first pixel pointed to is top-left-most in the image"
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);
	if (data)
	{
		glTextureStorage2D(textureID, 1, GL_RGBA8, width, height);
		glTextureSubImage2D(textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		if (minMode == GL_LINEAR_MIPMAP_LINEAR) {
			glGenerateTextureMipmap(textureID);
		}

		glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, uvMode);
		glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, uvMode);
		glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, minMode);
		glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, magMode);

		stbi_image_free(data);

		std::cout << "Texture loaded at path: " << path << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return false;
	}

	return true;
}