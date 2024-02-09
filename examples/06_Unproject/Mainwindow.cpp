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

MainWindow::MainWindow():
	m_camera(m_windowWidth, m_windowHeight,
		glm::vec3(2.0, 0.0, 2.0),
		glm::vec3(0.0, 0.0, 0.0))
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
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
		MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		w->CursorPositionCallback(xpos, ypos);
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
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumBuffers, m_buffers);
	int resInitGeometry = InitGeometrySpiral();
	if (resInitGeometry != 0) {
		std::cerr << "Error during init geometry spiral creation\n";
		return resInitGeometry;
	}

	//////////////// UNPROJECT
	// Creation de la geometrie pour l'affichage du point
	// calculé par unproject.
	GLfloat vertices[2][3] = { {0,0,0}, {0,0,0} };
	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Ray]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(m_VAOs[VAO_Ray]);
	glVertexAttribPointer(SHADER_POSTION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(SHADER_POSTION_LOCATION);

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
	m_mainShader->setMat4(m_mainShaderLocations.uProjMatrix, m_camera.projectionMatrix());

	for (int i = 0; i < NbSpirals; ++i)
	{

		glm::mat4 currentTransformation = m_camera.viewMatrix();
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

	// UNPROJECT
	// Draw the vector if one spiral is selected
	if (m_selectedSpiral != -1) {
		m_pickingShader->bind();
		m_pickingShader->setMat4(m_pickingShaderLocations.uProjMatrix, m_camera.projectionMatrix());
		m_pickingShader->setMat4(m_pickingShaderLocations.uMatrix, m_camera.viewMatrix());

		glBindVertexArray(m_VAOs[VAO_Ray]);
		m_pickingShader->setVec4(m_pickingShaderLocations.uColor, glm::vec4(0.9f, 0.2f, 0.1f, 1.0f));
		glDrawArrays(GL_POINTS, 0, 2);

		m_pickingShader->setVec4(m_pickingShaderLocations.uColor, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		glDrawArrays(GL_LINES, 0, 2);
	}

	glFlush();
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
		m_camera.keybordEvents(m_window, delta_time);

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

	glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Spiral]);
	glBufferData(GL_ARRAY_BUFFER, DataSize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetVertices, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetColors, sizeof(Colors), Colors);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetSelectedColors, sizeof(SelectedColors), SelectedColors);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetNormals, sizeof(Normals), Normals);

	///////////////////////////////
	// Main shader VAO creation
	m_mainShader->bind();

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
	m_pickingShader->bind();

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
	// This to found easily the object associated to a given color
	struct compUVec4 {
		bool operator()(const glm::uvec4& a, const glm::uvec4& b) const {
			auto ia = (a.a << 24) + (a.r << 16) + (a.g << 8) + a.b;
			auto ib = (b.a << 24) + (b.r << 16) + (b.g << 8) + b.b;
			return ia < ib;
		}
	};
	std::map<glm::uvec4, uint32_t, compUVec4> myMap;
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
	m_pickingShader->setMat4(m_pickingShaderLocations.uProjMatrix, m_camera.projectionMatrix());
	for (uint32_t id = 0; id < NbSpirals; ++id)
	{
		// Save transformations
		glm::mat4 currentTransformation(m_camera.viewMatrix());

		// Translate spiral
		currentTransformation = glm::translate(currentTransformation,
			glm::vec3(cos(2.0f * id * float(M_PI) / static_cast<float>(NbSpirals)),sin(2.0f * id * float(M_PI) / static_cast<float>(NbSpirals)),0.0));

		// For convenience, convert the ID to a color object.
		auto GetRGBA = [](uint32_t v) -> glm::uvec4 {
			unsigned int blue = v & 255;
			unsigned int green = (v >> 8) & 255;
			unsigned int red = (v >> 16) & 255;
			unsigned int alpha = (v >> 24) & 255;

			return glm::uvec4(red, green, blue, alpha);
		};
		glm::uvec4 color = GetRGBA(id);
		myMap.insert({color, id}); 


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
	// Get the value matching the key.
	auto iteratorToPair = myMap.find(pickedColor);
	m_selectedSpiral = (iteratorToPair != myMap.end()) ? iteratorToPair->second : -1;
	std::cout << "m_selectedSpiral: " << m_selectedSpiral << std::endl;


	///////////////// UNPROJECT
	// Read depth information
	float depth = 0;
	glReadPixels(x, m_windowHeight - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	std::cout << "Depth: " << depth << "\n";
	if (depth < 1) {
		// Compute intersection point
		glm::vec3 win = glm::vec3(x, m_windowHeight - 1 - y, depth); 
		glm::vec4 viewport(0, 0, m_windowWidth, m_windowHeight);
		m_point = glm::unProject(win, m_camera.viewMatrix(), m_camera.projectionMatrix(), viewport);
		std::cout << "p: " << m_point.x << " " << m_point.y << " " << m_point.z << "\n";

		glm::vec3 orig = m_camera.position();
		GLfloat vertices[2][3] = {
			 {orig.x,orig.y,orig.z}, 
			 {GLfloat(m_point.x),   GLfloat(m_point.y),   GLfloat(m_point.z)},
		};

		// Update our VBO
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Ray]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}
}


void MainWindow::FramebufferSizeCallback(int width, int height) {
	m_windowWidth = width;
	m_windowHeight = height;
	glViewport(0, 0, width, height);
	m_camera.viewportEvents(width, height);
}

void MainWindow::CursorPositionCallback(double xpos, double ypos) {
	int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
	m_camera.mouseEvents(glm::vec2(xpos, ypos), state == GLFW_PRESS);
}

void MainWindow::MouseButtonCallback(int button, int action, int mods)
{
	std::cout << "Mouse callback: \n";
	std::cout << " - Left? " << (button == GLFW_MOUSE_BUTTON_LEFT ? "true" : "false") << "\n";
	std::cout << " - Pressed? " << (action == GLFW_PRESS ? "true" : "false") << "\n";
	std::cout << " - Shift? " << (mods == GLFW_MOD_SHIFT ? "true" : "false") << "\n";
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mods == GLFW_MOD_SHIFT)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(m_window, &xpos, &ypos);
		std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;

		PerformSelection((int)xpos, (int)ypos);
	}
}