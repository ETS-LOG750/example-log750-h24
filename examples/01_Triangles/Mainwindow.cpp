#include "MainWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangles", NULL, NULL);
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
	// Shader loading
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

	// Get vertex attribute location
	int vPosLoc;
	if ((vPosLoc = m_mainShader->attributeLocation("pos")) < 0) {
		std::cerr << "Unable to find shader location for " << "pos" << std::endl;
		return 3;
	}

	// Generate the geometry and store it inside m_vertices
	CreateVertices();
	
	// Generate VAO and VBO
	glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_vbo_position);

	// Load vertices information on GPU (allocation and copy)
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_position);
	glBufferData(GL_ARRAY_BUFFER, long(sizeof(GLfloat) * m_vertices.size()),
		m_vertices.data(), GL_STATIC_DRAW);

	// Activate VAO and specify the layout
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_position); // Unecessary if already binded
	glVertexAttribPointer(vPosLoc, 2,    // 2 components, XY
		GL_FLOAT, // type 
		GL_FALSE, // normalize 
		0, // stride 
		BUFFER_OFFSET(0) // ptr (where the data starts)
	);
	glEnableVertexAttribArray(vPosLoc);

	// Clean up (optional)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return 0;
}

void MainWindow::RenderScene()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT);

	m_mainShader->bind(); // Might be not necessary if already bind 
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLES, 0, m_numVertices);

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

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

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

void MainWindow::CreateVertices()
{
	m_vertices.clear();

	// Triangle 1
	// P1
	m_vertices.push_back(-0.90f); // X
	m_vertices.push_back(-0.90f); // Y
	// P2
	m_vertices.push_back(0.85f);
	m_vertices.push_back(-0.90f);
	// P3
	m_vertices.push_back(-0.90f);
	m_vertices.push_back(0.85f);

	// Triangle 2
	// P1
	m_vertices.push_back(0.90f);
	m_vertices.push_back(-0.85f);
	// P2
	m_vertices.push_back(0.90f);
	m_vertices.push_back(0.90f);
	// P3
	m_vertices.push_back(-0.85f);
	m_vertices.push_back(0.90f);

	// Number of vertices = Number of floats / 2 floats per vertex
	m_numVertices = GLint(m_vertices.size() / 2);
}