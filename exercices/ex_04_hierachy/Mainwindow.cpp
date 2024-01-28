#include "MainWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


// v: animation entre [0, 1] -- boucle toutes les 5 secondes
// world size: [-5, 5] x [-5, 5]
void MainWindow::exerice(float v) const {
	// TODO: Appeler drawCircle pour dessiner un cercle de rayon 1.0 centré en (0, 0)
	// Sx, Sy: scale en x et y pour le cercle
}

void MainWindow::drawCircle(float Sx, float Sy, glm::mat4 T, glm::vec3 color) const {
	glm::mat4 finalT = glm::scale(T, glm::vec3(Sx, Sy, 1.0));
	m_mainShader->setMat4(m_mainShader_matrix, finalT);
	m_mainShader->setVec4(m_mainShader_color, glm::vec4(color, 1.0)); // rgba
	glBindVertexArray(m_VAOs[CircleVAO]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 362);
}

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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Draw squares", NULL, NULL);
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
	// For other callbacks:
	// https://www.glfw.org/docs/3.3/input_guide.html
}

int MainWindow::InitializeGL()
{
	// Setup framebuffer size (as we do additional work)
	FramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);

	// Load and create shaders
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

	// Check uniform locations
	m_mainShader_matrix = m_mainShader->uniformLocation("uMatrix");
	m_mainShader_color = m_mainShader->uniformLocation("uColor");

	// Vertex shader location
	int vPositionLocation;
	if ((vPositionLocation = m_mainShader->attributeLocation("vPosition")) < 0) {
		std::cerr << "Unable to find shader location for " << "vPosition" << std::endl;
		return 3;
	}


	// Generate all buffers
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumBuffers, m_buffers);

	/////////////////////////
	// Setup circle drawing
	glBindVertexArray(m_VAOs[CircleVAO]);
	std::vector<glm::vec3> positionsCircle;
	positionsCircle.push_back(glm::vec3(0.0, 0.0, 0.0));
	for (int i = 0; i < 361; i++) {
		float angle = i * 2.0 * 3.14 / 360.0;
		positionsCircle.push_back(glm::vec3(std::cos(angle), std::sin(angle), 0.0));
	}

	// Positions
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[Circle]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positionsCircle.size(), positionsCircle.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPositionLocation);

	////////////////////////
	// Lines
	std::vector<glm::vec3> positionsLines;
	for (int i = -5; i < 5;  i++) {
		positionsLines.push_back(glm::vec3(i, -5.0, 0.0));
		positionsLines.push_back(glm::vec3(i, 5.0, 0.0));
	}
	for (int i = -5; i < 5; i++) {
		positionsLines.push_back(glm::vec3(-5.0, i, 0.0));
		positionsLines.push_back(glm::vec3(5.0, i, 0.0));
	}

	glBindVertexArray(m_VAOs[LineVAO]);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[LinesBuffers]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positionsLines.size(), positionsLines.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(vPositionLocation, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPositionLocation);

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
		ImGui::Begin("Exercice");
		ImGui::Checkbox("Animate", &m_animate);
		ImGui::SliderFloat("Time", &m_time, 0.0, 1.0);

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void MainWindow::RenderScene(float time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_mainShader->bind();

	if (m_animate) {
		m_time = std::fmod(time / 5.0, 1.0);
	}

	// Draw the lines
	{
		m_mainShader->setMat4(m_mainShader_matrix, glm::mat4(1.0));
		m_mainShader->setVec4(m_mainShader_color, glm::vec4(0.5, 0.5, 0.5, 1.0)); // rgba
		glBindVertexArray(m_VAOs[LineVAO]);
		glDrawArrays(GL_LINES, 0, 40);
	}

	exerice(m_time);
	
}

int MainWindow::RenderLoop()
{
	float time = glfwGetTime();
	while (!glfwWindowShouldClose(m_window))
	{
		// Compute delta time between two frames
		float new_time = glfwGetTime();
		const float delta_time = new_time - time;
		time = new_time;

		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		RenderScene(time);
		RenderImgui();

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Cleanup
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