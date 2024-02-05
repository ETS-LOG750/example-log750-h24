#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "OBJLoader.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const GLuint NumVertices = 9;

MainWindow::MainWindow() 
{
}

int MainWindow::Initialisation()
{
	// OpenGL version (usefull for imGUI and other libraries)
	const char* glsl_version = "#version 430 core";

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

	// Request OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Transformation", NULL, NULL);
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

	// imGui: create interface
	// ---------------------------------------
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
	const std::string directory = SHADERS_DIR;
	bool shaderSuccess = true;
	m_mainShader = std::make_unique<ShaderProgram>();
	shaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "lighting.vert");
	shaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "lighting.frag");
	shaderSuccess &= m_mainShader->link();
	if (!shaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	m_normalShader = std::make_unique<ShaderProgram>();
	shaderSuccess &= m_normalShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "normal.vert");
	shaderSuccess &= m_normalShader->addShaderFromSource(GL_GEOMETRY_SHADER, directory + "normal.geo");
	shaderSuccess &= m_normalShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "normal.frag");
	shaderSuccess &= m_normalShader->link();
	if (!shaderSuccess) {
		std::cerr << "Error when loading normal shader\n";
		return 4;
	}

	OBJLoader::Loader object(directory + "susane.obj");
	if (!object.isLoaded()) {
		std::cerr << "Impossible de load the object (susane.obj)\n";
		return 5;
	}
	// Get the first mesh
	OBJLoader::Mesh m = object.getMeshes()[0];
	const float scale = 0.7f;
	const glm::vec3 offset(0.0, 0.0, 0.0);
	// -- Put all vertices inside a vector
	std::vector<GLfloat> vertices;
	m_nbVertices = 0;
	for (const OBJLoader::Vertex& v : m.vertices) {
		
		vertices.push_back(v.position[0] * scale + offset.x);
		vertices.push_back(v.position[1] * scale + offset.y);
		vertices.push_back(v.position[2] * scale + offset.z);
		vertices.push_back(v.normal[0]);
		vertices.push_back(v.normal[1]);
		vertices.push_back(v.normal[2]);
		m_nbVertices += 1;
	}

	glGenVertexArrays(NumVAOs, m_VAOs);
	glBindVertexArray(m_VAOs[Triangles]);
	glGenBuffers(NumBuffers, m_buffers);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
		vertices.data(), GL_STATIC_DRAW);
	// Position
	int PositionLocation = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(PositionLocation, 
		3, 
		GL_FLOAT,
		GL_FALSE, 
		sizeof(GLfloat) * 6, // Stride: taille d'un sommet
		BUFFER_OFFSET(0)
	);
	glEnableVertexAttribArray(PositionLocation);
	int NormalLocation = m_mainShader->attributeLocation("vNormal");
	glVertexAttribPointer(NormalLocation,
		3,
		GL_FLOAT,
		GL_TRUE,
		sizeof(GLfloat) * 6, // Stride: taille d'un sommet
		BUFFER_OFFSET(3 * sizeof(GLfloat))
	);
	glEnableVertexAttribArray(NormalLocation);

	glEnable(GL_DEPTH_TEST);

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
		ImGui::Begin("Transformations");

		ImGui::SliderAngle("Rotation x", &m_rot1.x);
		ImGui::SliderAngle("Rotation y", &m_rot1.y);
		ImGui::SliderAngle("Rotation z", &m_rot1.z);
	
		if (ImGui::Button("Reset")) {
			m_rot1 = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
		}

		ImGui::Checkbox("Show normal", &m_showNormal);
		ImGui::SliderFloat("Scale", &m_scale, 0.01f, 2.0f);

		
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


	// Compute transformation
	glm::mat4 m = glm::mat4(1);	
	m = glm::eulerAngleXYZ(m_rot1.x, m_rot1.y, m_rot1.z);

	m_mainShader->setMat4(SHADER_MATRIX, m);
	m_mainShader->setMat3(SHADER_MATRIX_NORMAL, glm::inverseTranspose(glm::mat3(m)));
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_nbVertices);

	if (m_showNormal) {
		m_normalShader->bind();
		m_normalShader->setMat4(SHADER_MATRIX, m);
		m_normalShader->setMat3(SHADER_MATRIX_NORMAL, glm::inverseTranspose(glm::mat3(m)));
		m_normalShader->setFloat(SHADER_SCALE, m_scale);
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_nbVertices);
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

void MainWindow::FramebufferSizeCallback(int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}