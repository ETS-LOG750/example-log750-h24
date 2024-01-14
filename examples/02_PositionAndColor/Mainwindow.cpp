#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

using namespace std;

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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Position and colors", NULL, NULL);
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

	// Get locations
	string shaderParameter = "vPosition";
	if ((m_vPositionLocation = m_mainShader->attributeLocation(shaderParameter)) < 0)
	{
		cerr << "Unable to find shader location for " << shaderParameter << endl;
		return 5;
	}

	shaderParameter = "vColor";
	if ((m_vColorLocation = m_mainShader->attributeLocation(shaderParameter)) < 0)
	{
		cerr << "Unable to find shader location for " << shaderParameter << endl;
		return 5;
	}

	shaderParameter = "uColor";
	if ((m_uniformColorLocation = m_mainShader->uniformLocation(shaderParameter)) < 0)
	{
		cerr << "Unable to find shader location for " << shaderParameter << endl;
		return 5;
	}

	std::vector<glm::vec3> vertices = {
		glm::vec3(-0.9f,  0.1f, 0.0f), // Triangle 1
		glm::vec3(0.0f,  0.9f, 0.0f),
		glm::vec3(0.9f,  0.1f, 0.0f),
		glm::vec3(-0.1f, -0.1f, 0.0f), // Triangle 2 (no color)
		glm::vec3(-0.1f, -0.9f, 0.0f),
		glm::vec3(-0.9f, -0.1f, 0.0f),
		glm::vec3(0.1f, -0.1f, 0.0f), // Triangle 3 (no color)
		glm::vec3(0.9f, -0.9f, 0.0f),
		glm::vec3(0.1f, -0.9f, 0.0f)
	};

	// couleur uniquement pour le premier triangle
	std::vector<glm::vec4> colors = {
		glm::vec4(0, 1, 1, 1), // s1
		glm::vec4(1, 0, 1, 1), // s2
		glm::vec4(1, 1, 0, 1)  // s3
	};

	// Generation des VAO et VBO
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumBuffers, m_buffers);

	////////////////////
	// Configuration des VAO / VBO
	glBindVertexArray(m_VAOs[Triangles]);
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);

	// Allocation du buffer m_buffers[ArrayBuffer]
	glBufferData(GL_ARRAY_BUFFER, 
		long(sizeof(glm::vec3) * vertices.size() + sizeof(glm::vec4) * colors.size()), // Taille
		nullptr, // Données a copier
		GL_STATIC_DRAW // Si les données sont modifiées frequemment, utiliser GL_DYNAMIC_DRAW
	);
	// Upload vertex informations (vertices, color)
	glBufferSubData(GL_ARRAY_BUFFER, 
		0, // Position dans le buffer (GPU) 
		long(sizeof(glm::vec3) * vertices.size()), // Taille des données 
		vertices.data() // Données
	);
	glBufferSubData(GL_ARRAY_BUFFER, 
		long(sizeof(glm::vec3) * vertices.size()), // Position dans le buffer (GPU) -- après les vertices
		long(sizeof(glm::vec4) * colors.size()),  // Taille des données
		colors.data() // Données
	);
	// Data layout (VAO)
	// ---- Positions (dans le shader: m_vPositionLocation)
	glVertexAttribPointer(m_vPositionLocation, 
		3, // 3 components (XYZ)
		GL_FLOAT, // Type
		GL_FALSE, // Normalize
		0, // Stride (0 = tightly packed)
		BUFFER_OFFSET(0) // Offset
	);
	glEnableVertexAttribArray(m_vPositionLocation);
	glVertexAttribPointer(m_vColorLocation, 
		4, // 4 components (RGBA)
		GL_FLOAT, // Type 
		GL_FALSE, // Normalize 
		0, // Stride (0 = tightly packed)
		BUFFER_OFFSET(long(sizeof(glm::vec3) * vertices.size())) // Offset (after vertices)
	);

	// Clean-up (optional)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Background color
	glClearColor(0.5f, 0.5f, 0.5f, 1.0);

	return 0;
}

void MainWindow::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(m_VAOs[Triangles]);
	m_mainShader->bind();

	// First triangle, vertex color
	glEnableVertexAttribArray(m_vColorLocation);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// The default color will be black vec4(0,0,0,1)
	glDisableVertexAttribArray(m_vColorLocation);

	// Second triangle, uniform color
	m_mainShader->setUniformValue(m_uniformColorLocation, glm::vec4( 0, 1, 0, 1));
	glDrawArrays(GL_TRIANGLES, 3, 3);

	// Third triangle, fragment shader color (alpha = 0)
	m_mainShader->setUniformValue(m_uniformColorLocation, glm::vec4( 1, 0, 0, 0));
	glDrawArrays(GL_TRIANGLES, 6, 3);

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
