#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "Teapot.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

MainWindow::MainWindow() :
	m_at(glm::vec3(0, 0,-1)),
	m_up(glm::vec3(0, 1, 0))
{
	updateCameraEye();

	// Generate random colors for patches
	for (int i = 0; i < nbPatch; ++i)
	{
		m_colors[i] = glm::vec4(1.0f * rand() / RAND_MAX, rand() * 1.0f / RAND_MAX, rand() * 1.0f / RAND_MAX, 1.0f);
	}
}

void MainWindow::FramebufferSizeCallback(int width, int height) {
	m_proj = glm::perspective(45.0f, float(width) / height, 0.01f, 100.0f);
}


int MainWindow::Initialisation()
{
	// OpenGL version (usefull for imGUI and other libraries)
	const char* glsl_version = "#version 430 core";

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tesselation Teapot", NULL, NULL);
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
	glGenVertexArrays(NumVAOs, m_VAOs);
	glBindVertexArray(m_VAOs[Triangles]);

	glGenBuffers(NumVertexBuffers, m_buffers);

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(teapotVertices), teapotVertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[ElementBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teapotPatches), teapotPatches, GL_STATIC_DRAW);

	std::cout << sizeof(teapotVertices) << " " << sizeof(teapotPatches) << "\n";

	// build and compile our shader program
	const std::string directory = SHADERS_DIR;

	m_mainShader = std::make_unique<ShaderProgram>();
	bool mainShaderSuccess = true;
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "teapot.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "teapot.frag");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_TESS_EVALUATION_SHADER, directory + "teapot.eval");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_TESS_CONTROL_SHADER, directory + "teapot.cont");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}
	m_mainShader_showNormal = m_mainShader->uniformLocation("showNormal");
	m_mainShader_inner = m_mainShader->uniformLocation("uInner");
	m_mainShader_outer = m_mainShader->uniformLocation("uOuter");
	if (m_mainShader_showNormal < 0 || m_mainShader_inner < 0 || m_mainShader_outer < 0) {
		std::cerr << "Unable to find uniform location for showNormal or uInner or uOuter\n";
		return 3;
	}

	m_constantColorShader = std::make_unique<ShaderProgram>();
	bool constantColorShaderSuccess = true;
	constantColorShaderSuccess &= m_constantColorShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "constantColor.vert");
	constantColorShaderSuccess &= m_constantColorShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "constantColor.frag");
	constantColorShaderSuccess &= m_constantColorShader->link();
	if (!constantColorShaderSuccess) {
		std::cerr << "Error when loading constant color shader\n";
		return 4;
	}
	
	// Setup shader variables
	m_constantColorShader->bind();
	int loc = m_constantColorShader->attributeLocation("vPosition");
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(loc);

	m_mainShader->bind();
	loc = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(loc, 3, GL_FLOAT,GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(loc);

	// Number of vertices for the patch
	// Here we do 4x4 bezier patches
	glPatchParameteri(GL_PATCH_VERTICES, 16);
	glPointSize(4);

	glEnable(GL_DEPTH_TEST);

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
		ImGui::Begin("Tesselation Teapot");

		ImGui::Separator();

		bool camChange = ImGui::SliderFloat("Left Right Slider", &m_longitude, -180, 180);
		camChange |= ImGui::SliderFloat("Up Down Slider", &m_latitude, -89, 89);
		camChange |= ImGui::SliderFloat("Forward Backward Slider", &m_distance, 2.f, 14.f);
		if (camChange)
		{
			updateCameraEye();
		}

		ImGui::Separator();

		ImGui::SliderFloat("Inner", &m_inner, 0, 16);
		ImGui::SliderFloat("Outer", &m_outer, 0, 16);

		ImGui::Separator();
		if (ImGui::Checkbox("Show normal", &m_showNormal)) {
			if (m_showNormal) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
		}


		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainWindow::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_VAOs[Triangles]);
	m_mainShader->bind();

	// Compute transformation and projection matrix
	glm::mat4 lookAt =glm::lookAt(m_eye, m_at, m_up);
	lookAt = glm::translate(lookAt, glm::vec3(0, -1, 0)); // Hard coded world translation

	m_mainShader->setMat4(SHADER_MATRIX, lookAt);
	m_mainShader->setMat3(SHADER_MATRIX_NORMAL, glm::inverseTranspose(glm::mat3(lookAt)));

	m_mainShader->setFloat(m_mainShader_inner, m_inner);
	m_mainShader->setFloat(m_mainShader_outer, m_outer);

	m_proj = glm::perspective(45.0f, float(SCR_WIDTH) / SCR_HEIGHT, 0.01f, 100.0f);
	m_mainShader->setMat4(SHADER_PROJECTION, m_proj);
	m_mainShader->setBool(m_mainShader_showNormal, m_showNormal);


	for (int i = 0; i < nbPatch; ++i)
	{
		glm::vec4 color = m_colors[i];
		m_mainShader->setVec4(SHADER_COLOR, color);
		glDrawElements(GL_PATCHES, 16, GL_UNSIGNED_INT, BUFFER_OFFSET(i * sizeof(GLuint) * 16));
	}

	if (!m_showNormal) {
		m_constantColorShader->bind();
		m_constantColorShader->setMat4(SHADER_MATRIX, lookAt);
		m_constantColorShader->setMat4(SHADER_PROJECTION, m_proj);

		for (int i = 0; i < nbPatch; ++i)
		{
			glm::vec4 color = m_colors[i];
			m_constantColorShader->setVec4(SHADER_COLOR, color);
			glDrawElements(GL_POINTS, 16, GL_UNSIGNED_INT, BUFFER_OFFSET(i * sizeof(GLuint) * 16));
		}
	}
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