#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const GLuint NumVertices = 9;

MainWindow::MainWindow() :
	m_scale(glm::vec3(1.0, 1.0, 1.0)),
	m_rot(glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f))),
	m_translate(glm::vec3(0.0, 0.0, 0.0))
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
	bool mainShaderSuccess = true;
	m_mainShader = std::make_unique<ShaderProgram>();
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "triangles.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "triangles.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	// Check uniform
	m_mainShader_matrix = m_mainShader->uniformLocation("uMatrix");
	m_mainShader_color = m_mainShader->uniformLocation("uColor");
	if(m_mainShader_matrix == -1 || m_mainShader_color == -1) {
		std::cerr << "Error when loading main shader (uniform)\n";
		return 4;
	}

	GLfloat vertices[NumVertices][3] = {
		{ -0.2f,  0.1f, 0.0f },
		{  0.0f,  0.4f, 0.0f },
		{  0.2f,  0.1f, 0.0f } };

	glGenVertexArrays(NumVAOs, m_VAOs);
	glBindVertexArray(m_VAOs[Triangles]);
	glGenBuffers(NumBuffers, m_buffers);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
		vertices, GL_STATIC_DRAW);
	int PositionLocation = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(PositionLocation, 3, GL_FLOAT,GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(PositionLocation);

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
		ImGui::SliderFloat3("Translate", &m_translate[0], -1.0, 1.0);
		ImGui::SliderAngle("Rotation x", &m_rot.x);
		ImGui::SliderAngle("Rotation y", &m_rot.y);
		ImGui::SliderAngle("Rotation z", &m_rot.z);
		ImGui::SliderFloat3("Scale", &m_scale.x, -10.0, 10.0); 

		if (ImGui::Button("Reset")) {
			m_scale = glm::vec3(1.0, 1.0, 1.0);
			m_rot = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
			m_translate = glm::vec3(0.0, 0.0, 0.0);
		}

		// Affichage des informations
		ImGui::Separator();
		std::string desc[MAX_COLORS];
		desc[0] = "T";
		desc[1] = "R";
		desc[2] = "S";
		desc[3] = "R*T";
		desc[4] = "T*R";
		desc[5] = "T*R*S";
		for (auto i = 0; i < 6; i++) {
			ImGui::TextColored(
				ImVec4(COLORS[i][0], COLORS[i][1], COLORS[i][2], COLORS[i][3]), 
				desc[i].c_str()
			);
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

	// Different transformations
	// 1) Scale
	glm::mat4 scale(1);
	scale = glm::scale(scale,m_scale);
	// 2) Rotation (from quaternion)
	glm::quat rotQ = glm::quat_cast(glm::eulerAngleXYZ(m_rot.x, m_rot.y, m_rot.z));
	// ... do something with the quaternion
	glm::mat4 rot = glm::mat4(glm::mat3_cast(rotQ));
	// 3) Translation
	glm::mat4 translate(1);
	translate = glm::translate(translate,m_translate);

	// Different triangle matrices and colors
	glm::mat4 transformations[MAX_COLORS];
	transformations[0] = translate;
	transformations[1] = rot;
	transformations[2] = scale;
	transformations[3] = rot * translate;
	transformations[4] = translate * rot;
	transformations[5] = translate * rot * scale;

	// Displacements (translation) of the triangles	
	float horiz[MAX_COLORS] = { -0.5, 0, 0.5,  -0.5,  0, 0.5 };
	float vert[MAX_COLORS] = { 0, 0, 0, -0.5, -0.5, -0.5 };

	glm::mat4 position;
   for (int i = 0; i <= 5; ++i)
	{
		// Different position of the triangles
		position = glm::mat4(1); ;
		position= glm::translate(position, glm::vec3(horiz[i], vert[i], 0.0));
		glm::mat4 final = position * transformations[i];
		m_mainShader->setUniformValue(m_mainShader_matrix, final);
		m_mainShader->setUniformValue(m_mainShader_color, COLORS[i]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	glFlush();
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