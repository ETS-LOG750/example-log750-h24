#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#ifndef M_PI
#define M_PI (3.14159)
#endif

MainWindow::MainWindow() :
	m_eye(glm::vec3(-2, 4, 8)),
	m_at(glm::vec3(0, 0, -1)),
	m_up(glm::vec3(0, 1, 0)),
	m_lightPosition(0, 0, 0),
	m_lightRadius(4.0),
	m_lightAngle(0.0),
	m_lightHeight(4.0)
{
	UpdateLightPosition(0);
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
	m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shadow Map", NULL, NULL);
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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Create VAOs and VBOs
	glGenVertexArrays(NumVAOs, m_VAOs);
	glGenBuffers(NumVBOs, VBOs);

	// build and compile our shader program
	const std::string directory = SHADERS_DIR;

	m_mainShader = std::make_unique<ShaderProgram>();
	bool mainShaderSuccess = true;
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "triangles.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "triangles.frag");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}
	// Load uniform
	m_mainUniforms.MVMatrix = m_mainShader->uniformLocation("MVMatrix");
	m_mainUniforms.ProjMatrix = m_mainShader->uniformLocation("ProjMatrix");
	m_mainUniforms.MLPMatrix = m_mainShader->uniformLocation("MLPMatrix");
	m_mainUniforms.normalMatrix = m_mainShader->uniformLocation("normalMatrix");
	m_mainUniforms.uColor = m_mainShader->uniformLocation("uColor");
	m_mainUniforms.texShadowMap = m_mainShader->uniformLocation("texShadowMap");
	m_mainUniforms.lightPositionCameraSpace = m_mainShader->uniformLocation("lightPositionCameraSpace");
	m_mainUniforms.biasType = m_mainShader->uniformLocation("biasType");
	m_mainUniforms.biasValue = m_mainShader->uniformLocation("biasValue");
	m_mainUniforms.biasValueMin = m_mainShader->uniformLocation("biasValueMin");
	if(m_mainUniforms.MVMatrix == -1 || m_mainUniforms.ProjMatrix == -1 || m_mainUniforms.MLPMatrix == -1 || m_mainUniforms.normalMatrix == -1 || m_mainUniforms.uColor == -1 || m_mainUniforms.texShadowMap == -1 || m_mainUniforms.lightPositionCameraSpace == -1 || m_mainUniforms.biasType == -1 || m_mainUniforms.biasValue == -1 || m_mainUniforms.biasValueMin == -1) {
		std::cerr << "Error when loading main shader uniforms\n";
		return 5;
	}
	
	m_mainShader->setInt(m_mainUniforms.texShadowMap, 0); // Setup shadow map Tex unit

	m_shadowMapShader = std::make_unique<ShaderProgram>();
	bool shadowMapShaderSuccess = true;
	shadowMapShaderSuccess &= m_shadowMapShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "shadow.vert");
	shadowMapShaderSuccess &= m_shadowMapShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "shadow.frag");
	shadowMapShaderSuccess &= m_shadowMapShader->link();
	if (!shadowMapShaderSuccess) {
		std::cerr << "Error when loading shadow map shader\n";
		return 4;
	}
	m_shadowMapUniforms.MLP = m_shadowMapShader->uniformLocation("MLP");
	if (m_shadowMapUniforms.MLP == -1) {
		std::cerr << "Error when loading shadow map shader uniforms\n";
		return 5;
	}

	m_debugShader = std::make_unique<ShaderProgram>();
	bool debugShaderSuccess = true;
	debugShaderSuccess &= m_debugShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "debug.vert");
	debugShaderSuccess &= m_debugShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "debug.frag");
	debugShaderSuccess &= m_debugShader->link();
	if (!debugShaderSuccess) {
		std::cerr << "Error when loading debug shader\n";
		return 4;
	}
	m_debugUniforms.tex = m_debugShader->uniformLocation("tex");
	m_debugUniforms.scale = m_debugShader->uniformLocation("scale");
	if (m_debugUniforms.tex == -1 || m_debugUniforms.scale == -1) {
		std::cerr << "Error when loading debug shader uniforms\n";
		return 5;
	}
	m_debugShader->setInt(m_debugUniforms.tex, 0);  // Setup debug tex unit

	// Check if locations for positions matches
	// so we can use a single VAO
	int locP1 = m_mainShader->attributeLocation("vPosition");
	int locP2 = m_shadowMapShader->attributeLocation("vPosition");
	int locP3 = m_debugShader->attributeLocation("vPosition");
	bool posLocEqual = (locP1 == locP2) && (locP2 == locP3);
	if (!posLocEqual) {
		std::cerr << "Location of vPosition between shaders mismatch" << std::endl;
		std::cerr << "Main shader           : " << locP1 << std::endl;
		std::cerr << "Shadow map gen. shader: " << locP2 << std::endl;
		std::cerr << "Debug shader          : " << locP3 << std::endl;
		return 10;
	}

	// Initialize the geometry
	int GeometryCubeReturn = InitGeometryCube();
	if (GeometryCubeReturn != 0)
	{
		return GeometryCubeReturn;
	}

	int GeometryFloorReturn = InitGeometryFloor();
	if (GeometryFloorReturn != 0)
	{
		return GeometryFloorReturn;
	}

	int GeometryPlane2DReturn = InitPlane2D();
	if (GeometryPlane2DReturn != 0) {
		return GeometryPlane2DReturn;
	}

	////////////////////////////////
	// Create a framebuffer object
	glGenFramebuffers(1, &DepthMapFBO);

	// 1) create depth texture
	glGenTextures(1, &TextureId);
	glBindTexture(GL_TEXTURE_2D, TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE_X, SHADOW_SIZE_Y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 2) attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureId, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer is not ok" << std::endl;
		return 6; // Error
	}
	else
	{
		std::cout << "framebuffer is ok" << std::endl;
	}

	// Tell the main shader that we will 
	// use the texShadowMap at texture unit 0
	glUseProgram(m_mainShader->programId());
	m_mainShader->setInt(m_mainUniforms.texShadowMap, 0);

	// Initialize camera... etc
	FramebufferSizeCallback(SCR_WIDTH, SCR_HEIGHT);

	return 0;
}

void MainWindow::RenderScene()
{
	// Compute camera
	glm::mat4 lookAt = glm::lookAt(m_eye, m_at, m_up);

	/////////////// 
	//  Shadow pass
	///////////////
	ShadowRender();

	////////////////////
	// Normal rendering pass
	////////////////////
	
	// Clear buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_mainShader->programId());

	// Matrices and lighting informations
	m_mainShader->setMat4(m_mainUniforms.MVMatrix, lookAt);
	m_mainShader->setMat4(m_mainUniforms.ProjMatrix, m_proj);
	m_mainShader->setVec3(m_mainUniforms.lightPositionCameraSpace, lookAt * glm::vec4(m_lightPosition, 1.0));
	// Shadow map
	m_mainShader->setMat4(m_mainUniforms.MLPMatrix, m_lightViewProjMatrix);
	// Bias configuration
	m_mainShader->setInt(m_mainUniforms.biasType, m_biasType);
	m_mainShader->setFloat(m_mainUniforms.biasValue, m_biasValue);
	m_mainShader->setFloat(m_mainUniforms.biasValueMin, m_biasValueMin);


	// Activate texture containing the shadow map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureId);

	// Draw WHITE floor
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 modelViewMatrix = lookAt * modelMatrix;
	glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat4(modelViewMatrix));
	m_mainShader->setVec4(m_mainUniforms.uColor, glm::vec4(1.0, 1.0, 1.0, 1.0));
	m_mainShader->setMat4(m_mainUniforms.MVMatrix, modelViewMatrix);
	m_mainShader->setMat3(m_mainUniforms.normalMatrix, normalMatrix);
	m_mainShader->setMat4(m_mainUniforms.MLPMatrix, m_lightViewProjMatrix * modelMatrix);
	glBindVertexArray(m_VAOs[FloorVAO]);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NumVerticesFloor);

	// Draw RED cube
	modelMatrix = glm::translate(modelMatrix, m_cubePosition);   // translate up by 1.0
	modelViewMatrix = lookAt * modelMatrix;
	normalMatrix = glm::inverseTranspose(glm::mat4(modelViewMatrix));
	m_mainShader->setVec4(m_mainUniforms.uColor, glm::vec4(1.0, 0.0, 0.0, 1.0));
	m_mainShader->setMat4(m_mainUniforms.MVMatrix, modelViewMatrix);
	m_mainShader->setMat3(m_mainUniforms.normalMatrix, normalMatrix);
	m_mainShader->setMat4(m_mainUniforms.MLPMatrix, m_lightViewProjMatrix * modelMatrix);

	glBindVertexArray(m_VAOs[CubeVAO]);
	glDrawElements(GL_TRIANGLES, 3 * NumTriCube, GL_UNSIGNED_INT, 0);

	if (m_debug) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(m_debugShader->programId());
		m_debugShader->setFloat(m_debugUniforms.scale, m_debugScale);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureId);

		glBindVertexArray(m_VAOs[Plane2DVAO]);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

void MainWindow::RenderImgui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//imgui 
	{
		ImGui::Begin("Shadow mapping");
		
		ImGui::Checkbox("Debug", &m_debug);
		ImGui::InputFloat("debugScale", &m_debugScale);

		ImGui::Separator();
		ImGui::Text("Camera");
		ImGui::Checkbox("Animate", &m_lightAnimation);
		ImGui::InputFloat3("Position Light", &m_lightPosition[0]);
		ImGui::InputFloat("Near", &m_lightNear);
		ImGui::InputFloat("Far", &m_lightFar);


		ImGui::Separator();
		ImGui::Text("Cube");
		ImGui::InputFloat3("Position Cube", &m_cubePosition[0]);

		ImGui::Separator();
		ImGui::Text("Bias configuration: ");
		const char* items[] = { "No bias", "Constant", "Cosine-based" };
		ImGui::ListBox("Type", &m_biasType, items, IM_ARRAYSIZE(items));
		ImGui::InputFloat("Value", &m_biasValue, 0.01f, 1.0f, "%.6f");
		ImGui::InputFloat("Value Min", &m_biasValueMin, 0.01f, 1.0f, "%.6f");

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


int MainWindow::RenderLoop()
{
	float time = (float)glfwGetTime();
	while (!glfwWindowShouldClose(m_window))
	{
		float new_time = (float)glfwGetTime();
		const float delta_time = new_time - time;
		time = new_time;

		UpdateLightPosition(delta_time);

		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);

		RenderScene();
		RenderImgui();

		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	glfwDestroyWindow(m_window);
	glfwTerminate();

	return 0;
}

int MainWindow::InitPlane2D() {
	GLfloat VerticesPlane2D[4][3] = {
		{-1.0, -1.0, -.5},
		{3.0, -1.0, -.5},
		{-1.0, 3.0, -.5},
	};

	glBindVertexArray(m_VAOs[Plane2DVAO]);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[Plane2DVBO]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesPlane2D), VerticesPlane2D, GL_STATIC_DRAW);
	
	int locPos = m_debugShader->attributeLocation("vPosition");
	glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(locPos);

	return 0;
}

int MainWindow::InitGeometryCube()
{
	// Create cube vertices and faces
	GLfloat VerticesCube[NumVerticesCube][3] = {
	  { -0.5, -0.5,  0.5 },	// Front face
	  {  0.5, -0.5,  0.5 },
	  {  0.5,  0.5,  0.5 },
	  { -0.5,  0.5,  0.5 },
	  { -0.5, -0.5, -0.5 },	// Left face
	  { -0.5, -0.5,  0.5 },
	  { -0.5,  0.5,  0.5 },
	  { -0.5,  0.5, -0.5 },
	  {  0.5, -0.5,  0.5 },	// Right face
	  {  0.5, -0.5, -0.5 },
	  {  0.5,  0.5, -0.5 },
	  {  0.5,  0.5,  0.5 },
	  {  0.5, -0.5, -0.5 },	// Back face
	  { -0.5, -0.5, -0.5 },
	  { -0.5,  0.5, -0.5 },
	  {  0.5,  0.5, -0.5 },
	  { -0.5,  0.5,  0.5 },	// Top face
	  {  0.5,  0.5,  0.5 },
	  {  0.5,  0.5, -0.5 },
	  { -0.5,  0.5, -0.5 },
	  { -0.5, -0.5, -0.5 },	// Bottom face
	  {  0.5, -0.5, -0.5 },
	  {  0.5, -0.5,  0.5 },
	  { -0.5, -0.5,  0.5 }
	};
	GLfloat NormalsCube[NumVerticesCube][3] = {
	  {  0.0,  0.0,  1.0 },	// Front face
	  {  0.0,  0.0,  1.0 },
	  {  0.0,  0.0,  1.0 },
	  {  0.0,  0.0,  1.0 },
	  { -1.0,  0.0,  0.0 },	// Left face
	  { -1.0,  0.0,  0.0 },
	  { -1.0,  0.0,  0.0 },
	  { -1.0,  0.0,  0.0 },
	  {  1.0,  0.0,  0.0 },	// Right face
	  {  1.0,  0.0,  0.0 },
	  {  1.0,  0.0,  0.0 },
	  {  1.0,  0.0,  0.0 },
	  {  0.0,  0.0, -1.0 },	// Back face
	  {  0.0,  0.0, -1.0 },
	  {  0.0,  0.0, -1.0 },
	  {  0.0,  0.0, -1.0 },
	  {  0.0,  1.0,  0.0 },	// Top face
	  {  0.0,  1.0,  0.0 },
	  {  0.0,  1.0,  0.0 },
	  {  0.0,  1.0,  0.0 },
	  {  0.0, -1.0,  0.0 },	// Bottom face
	  {  0.0, -1.0,  0.0 },
	  {  0.0, -1.0,  0.0 },
	  {  0.0, -1.0,  0.0 }
	};

	GLuint IndicesCube[NumTriCube][3] = {
	  { 0, 2, 3 },	// Front face
	  { 0, 1, 2 },
	  { 4, 6, 7 },	// Left face
	  { 4, 5, 6 },
	  { 8, 10, 11 },	// Right face
	  { 8, 9, 10 },
	  { 12, 14, 15 },	// Back face
	  { 12, 13, 14 },
	  { 16, 18, 19 },	// Top face
	  { 16, 17, 18 },
	  { 20, 22, 23 },	// Bottom face
	  { 20, 21, 22 }
	};

	// Set VAO
	glBindVertexArray(m_VAOs[CubeVAO]);

	// Fill vertex VBO
	GLsizeiptr OffsetVertices = 0;
	GLsizeiptr OffsetNormals = sizeof(VerticesCube);
	GLsizeiptr DataSize = sizeof(VerticesCube) + sizeof(NormalsCube);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[CubeVBO]);
	glBufferData(GL_ARRAY_BUFFER, DataSize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetVertices, sizeof(VerticesCube), VerticesCube);
	glBufferSubData(GL_ARRAY_BUFFER, OffsetNormals, sizeof(NormalsCube), NormalsCube);

	// Setup shader variables
	int locPos = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetVertices));
	glEnableVertexAttribArray(locPos);
	int locNormal = m_mainShader->attributeLocation("vNormal");
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetNormals));
	glEnableVertexAttribArray(locNormal);

	// Fill in indices VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs[CubeEBO]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesCube), IndicesCube, GL_STATIC_DRAW);

	return 0;
}

int MainWindow::InitGeometryFloor()
{
	GLfloat VerticesFloor[NumVerticesFloor][3] = {
		{-4, 0, -4},
		{ 4, 0, -4},
		{ 4, 0, 4},
		{ -4,  0, 4}
	};

	GLfloat NormalsFloor[NumVerticesFloor][3] = {
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0},
		{0, 1, 0}
	};

	// Set VAO
	glBindVertexArray(m_VAOs[FloorVAO]);

	// Fill vertex VBO
	GLsizeiptr OffsetVertices = 0;
	GLsizeiptr OffsetNormals = sizeof(VerticesFloor);
	GLsizeiptr DataSize = sizeof(VerticesFloor) + sizeof(NormalsFloor);

	glBindBuffer(GL_ARRAY_BUFFER, VBOs[FloorVBO]);
	glBufferData(GL_ARRAY_BUFFER, DataSize, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(VerticesFloor), VerticesFloor);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(VerticesFloor), sizeof(NormalsFloor), NormalsFloor);

	int locPos = m_mainShader->attributeLocation("vPosition");
	glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetVertices));
	glEnableVertexAttribArray(locPos);
	int locNormal = m_mainShader->attributeLocation("vNormal");
	glVertexAttribPointer(locNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(OffsetNormals));
	glEnableVertexAttribArray(locNormal);

	return 0;
}

void MainWindow::ShadowRender()
{
	// Save viewport information
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Render the scene from the light's point of view.
	// Create a viewport that matches the size of the shadow map FBO.
	glViewport(0, 0, SHADOW_SIZE_X, SHADOW_SIZE_Y);

	// Setup the offscreen frame buffer we'll use to store the depth image.
	glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Compute the light projection matrix.
	GLfloat LightFOV = 90.0f;
	glm::mat4 LightProjMatrix = glm::perspective(LightFOV, 1.0f, m_lightNear, m_lightFar);

	// Compute the light view matrix.
	glm::vec3 At(0.0, 0.0, 0.0);    // Center of the scene
	glm::vec3 Up(0.0, 1.0, 0.0);    // Up direction.
	glm::vec3 LightPos(m_lightPosition.x, m_lightPosition.y, m_lightPosition.z);
	glm::mat4 LightViewMatrix = glm::lookAt(LightPos, At, Up);
	m_lightViewProjMatrix = LightProjMatrix * LightViewMatrix;

	// Bind the shadow shader program.
	glUseProgram(m_shadowMapShader->programId());

	// Draw the floor
	glm::mat4 ModelMatrix = glm::mat4(1.0f);

	m_shadowMapShader->setMat4(m_shadowMapUniforms.MLP, m_lightViewProjMatrix * ModelMatrix);
	glBindVertexArray(m_VAOs[FloorVAO]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, NumVerticesFloor);

	// Draw the cube
	ModelMatrix = glm::translate(ModelMatrix, m_cubePosition);
	m_shadowMapShader->setMat4(m_shadowMapUniforms.MLP, m_lightViewProjMatrix * ModelMatrix);
	glBindVertexArray(m_VAOs[CubeVAO]);
	glDrawElements(GL_TRIANGLES, 3 * NumTriCube, GL_UNSIGNED_INT, nullptr);

	//Finish drawing and release the framebuffer.
	glFinish();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glClearColor(0, 0, 0, 1);
}

void MainWindow::UpdateLightPosition(float delta_time)
{
	if (m_lightAnimation) {
		m_lightAngle += delta_time;
		m_lightPosition.x = float(m_lightRadius * cos(m_lightAngle));
		m_lightPosition.z = float(-m_lightRadius * sin(m_lightAngle));
		m_lightPosition.y = float(m_lightHeight);
	}
}