#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#ifndef M_PI
#define M_PI (3.14159)
#endif

// Constant for spiral drawing
const int NbStepsSpiral = 100;
const int NbVerticesSpiral = NbStepsSpiral * 2;
const int NbSpirals = 10;

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
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Picking", NULL, NULL);
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
	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
		MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		w->MouseButtonCallback(button, action, mods);
		});

}

int MainWindow::InitializeGL()
{
	const std::string directory = SHADERS_DIR;

	// Main shader loading
	bool mainShaderSuccess = true;
	m_mainShader = std::make_unique<ShaderProgram>();
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "triangles.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "triangles.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	// Get locations of the uniform variables
	m_mainShaderLocations.uProjMatrix = m_mainShader->uniformLocation("uProjMatrix");
	m_mainShaderLocations.uMatrix = m_mainShader->uniformLocation("uMatrix");
	m_mainShaderLocations.uNormalMatrix = m_mainShader->uniformLocation("uNormalMatrix");
	if (m_mainShaderLocations.uProjMatrix < 0 || m_mainShaderLocations.uMatrix < 0 || m_mainShaderLocations.uNormalMatrix < 0) {
		std::cerr << "Unable to find shader location for uProjMatrix, uMatrix or uNormalMatrix" << std::endl;
		return 3;
	}

	// Picking shader
	bool pickingSuccess = true;
	m_pickingShader = std::make_unique<ShaderProgram>();
	pickingSuccess &= m_pickingShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "constantColor.vert");
	pickingSuccess &= m_pickingShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "constantColor.frag");
	pickingSuccess &= m_pickingShader->link();
	if (!pickingSuccess) {
		std::cerr << "Error when loading pikcing shader\n";
		return 4;
	}

	// Get locations of the uniform variables
	m_pickingShaderLocations.uProjMatrix = m_pickingShader->uniformLocation("uProjMatrix");
	m_pickingShaderLocations.uColor = m_pickingShader->uniformLocation("uColor");
	m_pickingShaderLocations.uMatrix = m_pickingShader->uniformLocation("uMatrix");
	if (m_pickingShaderLocations.uProjMatrix < 0 || m_pickingShaderLocations.uColor < 0 || m_pickingShaderLocations.uMatrix < 0) {
		std::cerr << "Unable to find shader location for uProjMatrix, uColor or uMatrix" << std::endl;
		return 3;
	}

	// Create our VertexArrays Objects and VertexBuffer Objects
	int resInitGeometry = InitGeometrySpiral();
	if (resInitGeometry != 0) {
		std::cerr << "Error during init geometry spiral creation\n";
		return resInitGeometry;
	}

	// Init GL properties
	glPointSize(10.0f);
	glEnable(GL_DEPTH_TEST);

	return 0;
}

void MainWindow::RenderScene()
{
	// Clear the buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our vertex/fragment shaders
	m_mainShader->bind();

	// Draw the spirals
	glBindVertexArray(m_VAOs[VAO_Spiral]);
	m_mainShader->setMat4(m_mainShaderLocations.uProjMatrix, m_projectionMatrix);

	for (int i = 0; i < NbSpirals; ++i)
	{

		glm::mat4 currentTransformation = m_modelViewMatrix;
		currentTransformation = glm::translate(currentTransformation,
			// Translation vector
			// based on spherical coordinates
			glm::vec3(cos(2.0f * i * float(M_PI) / static_cast<float>(NbSpirals)), 
					  sin(2.0f * i * float(M_PI) / static_cast<float>(NbSpirals)), 
				  	  0.0)
		);

		// Draw selected spiral differently
		bool isSelected = (m_selectedSpiral == i);
		if (isSelected)
			glBindVertexArray(m_VAOs[VAO_SpiralSelected]);

		// Draw the spiral
		m_mainShader->setMat4(m_mainShaderLocations.uMatrix, currentTransformation);
		glm::mat3 NormalMat = glm::inverseTranspose(glm::mat3(currentTransformation));
		m_mainShader->setMat3(m_mainShaderLocations.uNormalMatrix, NormalMat);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, NbVerticesSpiral);

		// Restore original VAO if necessary
		if (isSelected)
			glBindVertexArray(m_VAOs[VAO_Spiral]);
	}

	glFlush();
}

int MainWindow::RenderLoop()
{
	while (!glfwWindowShouldClose(m_window))
	{
		// Setup the camera
		// Get projection and camera transformations
		// TODO: Add possibility to navigate inside the scene
		m_projectionMatrix = glm::perspective(glm::radians(45.0f), float(m_windowWidth) / m_windowHeight, 0.01f, 100.0f);
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 CameraPosition = glm::vec3(-2.0f, 4.0f, 8.0f);
		m_modelViewMatrix = glm::lookAt(CameraPosition, Front, Up);

		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		RenderScene();

		// If we press D, we show the debug view
		if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
		{
			PerformSelection(0,0);
		}

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	// Cleanup
	glfwDestroyWindow(m_window);
	glfwTerminate();

	return 0;
}

int MainWindow::InitGeometrySpiral()
{
	// Generate the spiral's vertices
	GLfloat Vertices[NbVerticesSpiral][3];
	GLfloat Colors[NbVerticesSpiral][3];
	GLfloat SelectedColors[NbVerticesSpiral][3];
	GLfloat Normals[NbVerticesSpiral][3];

	for (int i = 0; i < NbStepsSpiral; ++i)
	{
		float ratio = static_cast<float>(i) / static_cast<float>(NbStepsSpiral);
		float angle = 21.0f * ratio;
		float c = cos(angle);
		float s = sin(angle);
		float r1 = 0.5f - 0.3f * ratio;
		float r2 = 0.3f - 0.3f * ratio;
		float alt = ratio - 0.5f;
		const float nor = 0.5f;
		const float up = float(sqrt(1.0f - nor * nor));

		// Generate vertices' position
		Vertices[i * 2][0] = r2 * c;
		Vertices[i * 2][1] = r2 * s;
		Vertices[i * 2][2] = alt + 0.05f;

		Vertices[i * 2 + 1][0] = r1 * c;
		Vertices[i * 2 + 1][1] = r1 * s;
		Vertices[i * 2 + 1][2] = alt;

		// Generate vertices' color
		Colors[i * 2][0] = 1.0f - ratio;
		Colors[i * 2][1] = 0.2f;
		Colors[i * 2][2] = ratio;

		Colors[i * 2 + 1][0] = 1.0f - ratio;
		Colors[i * 2 + 1][1] = 0.2f;
		Colors[i * 2 + 1][2] = ratio;

		SelectedColors[i * 2][0] = 1.0f - ratio;
		SelectedColors[i * 2][1] = 0.8f;
		SelectedColors[i * 2][2] = ratio / 2.0f;

		SelectedColors[i * 2 + 1][0] = 1.0f - ratio;
		SelectedColors[i * 2 + 1][1] = 0.8f;
		SelectedColors[i * 2 + 1][2] = ratio / 2.0f;

		// Generate vertices' normal
		Normals[i * 2][0] = nor * c;
		Normals[i * 2][1] = nor * s;
		Normals[i * 2][2] = up;

		Normals[i * 2 + 1][0] = nor * c;
		Normals[i * 2 + 1][1] = nor * s;
		Normals[i * 2 + 1][2] = up;
	}

	// Transfer our vertices to the graphic card memory (in our VBO)
	GLsizeiptr DataSize = sizeof(Vertices) + sizeof(Colors) + sizeof(SelectedColors) + sizeof(Normals);
	GLsizeiptr OffsetVertices = 0;
	GLsizeiptr OffsetColors = sizeof(Vertices);
	GLsizeiptr OffsetSelectedColors = OffsetColors + long(sizeof(Colors));
	GLsizeiptr OffsetNormals = OffsetSelectedColors + long(sizeof(SelectedColors));

	// Create VAO and VBO
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumBuffers, m_buffers);

	// Copy our vertices to the VBO
	// Note multiple VBOs can be used to store different attributes (position, color, ...)
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Spiral]);
	glBufferData(GL_ARRAY_BUFFER, DataSize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetVertices, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetColors, sizeof(Colors), Colors);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetSelectedColors, sizeof(SelectedColors), SelectedColors);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetNormals, sizeof(Normals), Normals);

	///////////////////////////////
	// Main shader VAO creation

	// 1) Create VAO for rendering spirals
	glBindVertexArray(m_VAOs[VAO_Spiral]);
	glVertexAttribPointer(SHADER_POSTION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetVertices));
	glEnableVertexAttribArray(SHADER_POSTION_LOCATION); 
	glVertexAttribPointer(SHADER_COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetColors));
	glEnableVertexAttribArray(SHADER_COLOR_LOCATION);
	glVertexAttribPointer(SHADER_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetNormals));
	glEnableVertexAttribArray(SHADER_NORMAL_LOCATION);

	// 2) Create VAO for rendering selected spirals
	glBindVertexArray(m_VAOs[VAO_SpiralSelected]);
	glVertexAttribPointer(SHADER_POSTION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetVertices));
	glEnableVertexAttribArray(SHADER_POSTION_LOCATION);
	// Note here we use offsetSelectedColor here (compared to previous VAO)
	glVertexAttribPointer(SHADER_COLOR_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetSelectedColors));
	glEnableVertexAttribArray(SHADER_COLOR_LOCATION); 
	glVertexAttribPointer(SHADER_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetNormals));
	glEnableVertexAttribArray(SHADER_NORMAL_LOCATION);

	///////////////////////////////
	// Picking shader VAO creation
	
	// Create VAO for spirals during picking (with constant color shader)
	// The color will be set using uniform
	glBindVertexArray(m_VAOs[VAO_SpiralPicking]);
	glVertexAttribPointer(SHADER_POSTION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetVertices));
	glEnableVertexAttribArray(SHADER_POSTION_LOCATION);

	return 0;
}

void MainWindow::PerformSelection(int x, int y)
{
	// Map (dictionnary) used to store the correspondences between colors and spiral number.
	std::map<unsigned int, int> myMap;
	// Identificator ID used as a color for rendering and as a key for the map.
	std::cout << "Viewer::performSelection(" << x << ", " << y << ")" << std::endl;

	// Selection is performed by drawing the spirals with a color that matches their ID
	// Note: Because we are drawing outside the draw() function, the back buffer is not
	//       swapped after this function is called.

	// Clear back buffer. Since we set a differet clear color, we save the
	// previous value and restore it after the backbuffer has been cleared.
	{
		float clearColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);
		glClearColor(1, 1, 1, 1);	// This value should not match any existing index
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	}
	
	// Bind our vertex/fragment shaders
	m_pickingShader->bind();

	// Draw the spirals
	// Note that we use dedicated VAO in this case
	glBindVertexArray(m_VAOs[VAO_SpiralPicking]);
	m_pickingShader->setMat4(m_pickingShaderLocations.uProjMatrix, m_projectionMatrix);
	for (int i = 0; i < NbSpirals; ++i)
	{
		// Save transformations
		glm::mat4 currentTransformation(m_modelViewMatrix);

		// Translate spiral
		currentTransformation = glm::translate(currentTransformation,
			glm::vec3(cos(2.0f * i * float(M_PI) / static_cast<float>(NbSpirals)),sin(2.0f * i * float(M_PI) / static_cast<float>(NbSpirals)),0.0));

		// For convenience, convert the ID to a color object.
		glm::uvec4 color = GetRGBA(i);
		// Get the equivalent of the color as an unsigned long.
		unsigned int key = GetIntFromRGBA(color);
		// Insert the key (unsigned long) in the map.
		myMap.insert({ key, i });

		// Set the color value for the shader.
		// Need to send color where each channel is between [0, 1]
		glm::vec4 color_float = glm::vec4(color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0) ;
		m_pickingShader->setVec4(m_pickingShaderLocations.uColor, color_float);

		// Draw the spiral
		m_pickingShader->setMat4(m_pickingShaderLocations.uMatrix, currentTransformation);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, NbVerticesSpiral);
	}

	// Wait until all drawing commands are done
	glFinish();

	// Read the pixel under the cursor
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	unsigned char pixelData[4];
	glReadPixels(x, m_windowHeight - 1 - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

	std::cout << "Selected pixelData: " << int(pixelData[0]) << ", "
		<< int(pixelData[1]) << ", "
		<< int(pixelData[2]) << ", "
		<< int(pixelData[3]) << std::endl;

	// For convenience, construct a color object matching what was read in the frame buffer.
	glm::uvec4 pickedColor(pixelData[0], pixelData[1], pixelData[2], pixelData[3]);
	// Get the equivalent of the color as an unsigned int. This is the key we stored earlier in the map.
	unsigned int key = GetIntFromRGBA(pickedColor);
	// Get the value matching the key.
	auto iteratorToPair = myMap.find(key);
	m_selectedSpiral = (iteratorToPair != myMap.end()) ? iteratorToPair->second : -1;
	std::cout << "m_selectedSpiral: " << m_selectedSpiral << std::endl;
}


void MainWindow::FramebufferSizeCallback(int width, int height) {
	m_windowWidth = width;
	m_windowHeight = height;
	glViewport(0, 0, width, height);
}

void MainWindow::MouseButtonCallback(int button, int action, int mods)
{
	// std::cout << "Mouse callback: \n";
	// std::cout << " - Left? " << (button == GLFW_MOUSE_BUTTON_LEFT ? "true" : "false") << "\n";
	// std::cout << " - Pressed? " << (action == GLFW_PRESS ? "true" : "false") << "\n";
	// std::cout << " - Shift? " << (mods == GLFW_MOD_SHIFT ? "true" : "false") << "\n";
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_SHIFT)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(m_window, &xpos, &ypos);
		std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;

		PerformSelection((int)xpos, (int)ypos);
	}
}