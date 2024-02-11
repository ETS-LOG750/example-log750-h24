#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "OBJLoader.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

MainWindow::MainWindow() :
	m_at(glm::vec3(0, 0,-1)),
	m_up(glm::vec3(0, 1, 0)),
	m_light_position(glm::vec3(0.0, 0.0, 8.0))
{
	updateCameraEye();
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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Obj Loader", NULL, NULL);
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
	// Enable the depth test
	glEnable(GL_DEPTH_TEST);

	// build and compile our shader program
	const std::string directory = SHADERS_DIR;
	m_mainShader = std::make_unique<ShaderProgram>();
	bool mainShaderSuccess = true;
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "basicShader.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "basicShader.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	// Get the uniform locations
	m_mainShaderUniforms.modelview = m_mainShader->uniformLocation("mvMatrix");
	m_mainShaderUniforms.proj = m_mainShader->uniformLocation("projMatrix");
	m_mainShaderUniforms.normal = m_mainShader->uniformLocation("normalMatrix");
	m_mainShaderUniforms.lightPos = m_mainShader->uniformLocation("lightPos");
	m_mainShaderUniforms.Kd = m_mainShader->uniformLocation("Kd");
	m_mainShaderUniforms.Ks = m_mainShader->uniformLocation("Ks");
	m_mainShaderUniforms.Kn = m_mainShader->uniformLocation("Kn");
	if (m_mainShaderUniforms.modelview == -1 || m_mainShaderUniforms.proj == -1 || m_mainShaderUniforms.normal == -1 || m_mainShaderUniforms.lightPos == -1 || m_mainShaderUniforms.Kd == -1 || m_mainShaderUniforms.Ks == -1 || m_mainShaderUniforms.Kn == -1) {
		std::cerr << "Error when getting uniform locations\n";
		return 5;
	}

	// Load the 3D model from the obj file
	loadObjFile();

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
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Obj View");
		
		ImGui::Text("Camera settings");
		bool updateCamera = ImGui::SliderFloat("Longitude", &m_longitude, -180.f, 180.f);
		updateCamera |= ImGui::SliderFloat("Latitude", &m_latitude, -89.f, 89.f);
		updateCamera |= ImGui::SliderFloat("Distance", &m_distance, 2.f, 14.0f);
		if (updateCamera) {
			updateCameraEye();
		}

		ImGui::Separator();
		ImGui::Text("Lighting information");
		ImGui::InputFloat3("Position", &m_light_position.x);
		if (ImGui::Button("Copy camera position")) {
			m_light_position = m_eye;
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MainWindow::RenderScene()
{
	// Clear the frame buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind our vertex/fragment shaders
	glUseProgram(m_mainShader->programId());

	// Get projection and camera transformations
	glm::mat4 LookAt = glm::lookAt(m_eye, m_at, m_up);
	LookAt = glm::scale(LookAt,glm::vec3(0.5));

	// Note: optimized version of glm::transpose(glm::inverse(...))
	glm::mat3 NormalMat = glm::inverseTranspose(glm::mat3(LookAt));

	m_proj = glm::perspective(45.0f, float(SCR_WIDTH) / SCR_HEIGHT, 0.01f, 100.0f);
	m_mainShader->setMat4(m_mainShaderUniforms.modelview, LookAt);
	m_mainShader->setMat4(m_mainShaderUniforms.proj, m_proj);
	m_mainShader->setMat3(m_mainShaderUniforms.normal, NormalMat);
	m_mainShader->setVec3(m_mainShaderUniforms.lightPos, LookAt * glm::vec4(m_light_position, 1.0));

	// Draw the meshes
	for(const MeshGL& m : m_meshesGL)
	{
		// Set its material properties
		m_mainShader->setVec3(m_mainShaderUniforms.Kd, m.diffuse);
		m_mainShader->setVec3(m_mainShaderUniforms.Ks, m.specular);
		m_mainShader->setFloat(m_mainShaderUniforms.Kn, m.specularExponent);

		// Draw the mesh
		glBindVertexArray(m.vao);
		glDrawArrays(GL_TRIANGLES, 0, m.numVertices);
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

	// Clean memory
	// Delete vaos and vbos
	for (const MeshGL& m : m_meshesGL)
	{
		// Set material properties

		// Draw the mesh
		glDeleteVertexArrays(1, &m.vao);
		glDeleteBuffers(1, &m.vbo);
	}
	m_meshesGL.clear();

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

void MainWindow::loadObjFile()
{
	std::string assets_dir = ASSETS_DIR;
	std::string ObjPath = assets_dir + "soccerball.obj";
	// Load the obj file
	OBJLoader::Loader loader(ObjPath);

	// Create a GL object for each mesh extracted from the OBJ file
	// Note that if the 3D object have several different material
	// This will create multiple Mesh objects (one for each different material)
	const std::vector<OBJLoader::Mesh>& meshes = loader.getMeshes();
	const std::vector<OBJLoader::Material>& materials = loader.getMaterials();
	for (unsigned int i = 0; i < meshes.size(); ++i)
	{
		if (meshes[i].vertices.size() == 0)
			continue;

		MeshGL meshGL;
		meshGL.numVertices = meshes[i].vertices.size();

		// Set material properties of the mesh
		const float* Kd = materials[meshes[i].materialID].Kd;
		const float* Ks = materials[meshes[i].materialID].Ks;

		meshGL.diffuse = glm::vec3(Kd[0], Kd[1], Kd[2]);
		meshGL.specular = glm::vec3(Ks[0], Ks[1], Ks[2]);
		meshGL.specularExponent = materials[meshes[i].materialID].Kn;

		// Create its VAO and VBO object
		glGenVertexArrays(1, &meshGL.vao);
		glGenBuffers(1, &meshGL.vbo);

		// Fill VBO with vertices data
		GLsizei dataSize = meshes[i].vertices.size() * sizeof(OBJLoader::Vertex);
		GLsizei stride = sizeof(OBJLoader::Vertex);
		GLsizeiptr positionOffset = 0;
		GLsizeiptr normalOffset = sizeof(OBJLoader::Vertex::position);

		glBindBuffer(GL_ARRAY_BUFFER, meshGL.vbo);
		glBufferData(GL_ARRAY_BUFFER, dataSize, &meshes[i].vertices[0], GL_STATIC_DRAW);

		// Set VAO that binds the shader vertices inputs to the buffer data
		glBindVertexArray(meshGL.vao);

		glUseProgram(m_mainShader->programId());
		int PositionLoc = m_mainShader->attributeLocation("vPosition");
		glVertexAttribPointer(PositionLoc, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(positionOffset));
		glEnableVertexAttribArray(PositionLoc);

		int NormalLoc = m_mainShader->attributeLocation("vNormal");
		glVertexAttribPointer(NormalLoc, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(normalOffset));
		glEnableVertexAttribArray(NormalLoc);

		// Add it to the list
		m_meshesGL.push_back(meshGL);
	}
}
