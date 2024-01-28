#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lighting no camera", NULL, NULL);
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
	bool success = true;
	m_phongShader = std::make_unique<ShaderProgram>();
	success &= m_phongShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "phong-shading.vert");
	success &= m_phongShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "phong-shading.frag");
	success &= m_phongShader->link();
	if (!success) {
		std::cerr << "Error when loading Phong shader\n";
		return 4;
	}

	m_gouraudShader = std::make_unique<ShaderProgram>();
	success &= m_gouraudShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "gouraud-shading.vert");
	success &= m_gouraudShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "gouraud-shading.frag");
	success &= m_gouraudShader->link();
	if (!success) {
		std::cerr << "Error when loading Gouraud shader\n";
		return 4;
	}

	// Generate buffers ID
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumBuffers, m_VBOs);
	
	// Create the geometry and upload on the different buffers
	updateGeometry();

	// Setup VAO
	// Note: On peut reutiliser le VAO pour les deux shaders
	//  vu que les locations sont les memes (specifier en GLSL)
	glBindVertexArray(m_VAOs[Triangles]);
	// - VBO Positions
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Position]);
	int PositionLocation = m_phongShader->attributeLocation("vPosition");
	glVertexAttribPointer(PositionLocation, 
		3, // XYZ
		GL_FLOAT, 
		GL_FALSE, 
		0, 
		BUFFER_OFFSET(0));
	glEnableVertexAttribArray(PositionLocation);
	// - VBO Normales
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Normal]);
	int NormalLocation = m_phongShader->attributeLocation("vNormal");
	glVertexAttribPointer(GLuint(NormalLocation), 
		3, // XYZ
		GL_FLOAT,
		GL_TRUE, // true: normalize
		0, 
		BUFFER_OFFSET(0));
	glEnableVertexAttribArray(GLuint(NormalLocation));
	
	// Cleanup (Optional)
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// Other configurations
	// - Test de pronfondeur (pour afficher les triangles dans le bon ordre)
	glEnable(GL_DEPTH_TEST); 
	// - Couleur de fond (apres glClear(...))
	glClearColor(1.0, 1.0, 1.0, 1.0);

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
		
		ImGui::Begin("Plane transformation");
		bool needVerticesUpdate = false;
		needVerticesUpdate |= ImGui::SliderFloat("Rotation X", &m_rotationX, -180.f, 180.f);
		needVerticesUpdate |= ImGui::Checkbox("flatNormal", &m_flatNormals);
		if (needVerticesUpdate) {
			updateGeometry();
		}

		if (ImGui::Checkbox("showNormals", &m_showNormals)) {
			m_phongShader->setUniformValue(0, m_showNormals);
		}
		ImGui::Checkbox("Phong shading", &m_phongShading);

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainWindow::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (m_phongShading) {
		m_phongShader->bind();
	} else {
		m_gouraudShader->bind();
	}
	
	glBindVertexArray(m_VAOs[Triangles]);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
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

void MainWindow::updateGeometry()
{
	// Clear CPU memory
	m_vertices.clear();
	m_normals.clear();

	// Matrix to rotate the torus
	glm::mat4 latitude(1);
	latitude = glm::rotate(latitude, glm::radians(m_rotationX), glm::vec3(1, 0, 0));
	glm::mat3 m = glm::mat3(latitude);

	// Base positions
	const float PI = 3.14159265358979323846f;
	const float offset = PI * 2.f / (float)COUNT;
	for (unsigned int i = 0; i < COUNT; i++) {
		// Calcul des positions
		glm::vec3 p1bot = glm::vec3(
			0.7 * std::cos(offset * i), 
			-0.7, 
			0.7 *  std::sin(offset * i)
		);
		glm::vec3 p1top = glm::vec3(
			0.7 * std::cos(offset * i),
			0.7,
			0.7 * std::sin(offset * i)
		);
		glm::vec3 p2bot = glm::vec3(
			0.7 *  std::cos(offset * (i + 1)), 
			-0.7, 
			0.7 *  std::sin(offset * (i + 1))
		);
		glm::vec3 p2top = glm::vec3(
			0.7 * std::cos(offset * (i + 1)),
			0.7,
			0.7 * std::sin(offset * (i + 1))
		);

		// Triangle 1
		m_vertices.push_back(m * p1top);
		m_vertices.push_back(m * p1bot);
		m_vertices.push_back(m * p2bot);
		// Triangle 2
		m_vertices.push_back(m * p2top);
		m_vertices.push_back(m * p1top);
		m_vertices.push_back(m * p2bot);

		// Calcul des normales
		if (m_flatNormals) {
			// Calcul des normals avec le produit en croix
			// sur un des triangle
			glm::vec3 e1 = p1top - p1bot;
			glm::vec3 e2 = p2bot - p1bot;
			glm::vec3 n = glm::normalize(glm::cross(e1, e2));
			for (int j = 0; j < 6; j++) {
				m_normals.push_back(m * n);
			}
		}
		else {
			// Calcul des normales a chaque sommets
			// Note: p1top et p1bot ont les meme normales
			glm::vec3 n1 = glm::normalize(p1bot - glm::vec3(0, -0.7, 0));
			glm::vec3 n2 = glm::normalize(p2bot - glm::vec3(0, -0.7, 0));
			// Triangle 1
			m_normals.push_back(m * n1);
			m_normals.push_back(m * n1);
			m_normals.push_back(m * n2);
			// Triangle 2
			m_normals.push_back(m * n2);
			m_normals.push_back(m * n1);
			m_normals.push_back(m * n2);
		}
		
	}
	
	// Add data on the GPU (position)
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Position]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * m_vertices.size(),
		m_vertices.data(),
		GL_STATIC_DRAW);
	// Add data on the GPU (normal)
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Normal]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * m_normals.size(),
		m_normals.data(),
		GL_STATIC_DRAW);

	// Optional: Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}