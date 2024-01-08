#include "MainWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

MainWindow::MainWindow()
{
}


int MainWindow::Initialisation()
{
	// OpenGL version (usefull for imGUI and other libraries)
	const char* glsl_version = "#version 460 core";

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

	// Request OpenGL 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Triangles Bindless", NULL, NULL);
	if (m_window == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(m_window);

	// int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	// if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	// {
	// 	std::cout << "Debug context created\n";
	// 	// initialize debug output 
	// 	glEnable(GL_DEBUG_OUTPUT);
	// 	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	// 	glDebugMessageCallback(glDebugOutput, nullptr);
	// 	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	// } else {
	// 	std::cout << "Debug context not created\n";
	// }


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
	} else {
		std::cout << "pos location: " << vPosLoc << "\n";
	}

	// Generate the geometry and store it inside m_vertices
	CreateVertices();
	
	// Generate VAO and VBO
	glCreateVertexArrays(1, &m_vao);
	glCreateBuffers(1, &m_vbo_position);

	// Load vertices information on GPU (allocation and copy)
	glNamedBufferData(m_vbo_position, long(sizeof(GLfloat) * m_vertices.size()), m_vertices.data(), GL_STATIC_DRAW);

	// Activate VAO and specify the layout
	glEnableVertexArrayAttrib(m_vao, vPosLoc); // Enable the attribute at location vPosLoc
	glVertexArrayAttribBinding(m_vao, vPosLoc, 0); // Map the attribute at location vPosLoc to binding point 0
	glVertexArrayAttribFormat(m_vao, 
		vPosLoc, // Attribute index 
		2, // Number of components
		GL_FLOAT, // Type 
		GL_FALSE, // Normalize 
		0 // Offset of the first component
	);
	glVertexArrayVertexBuffer(m_vao, 
		0, // Binding point 
		m_vbo_position, // VBO 
		0, // Offset 
		2 * sizeof(GLfloat) // Stride
	);

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