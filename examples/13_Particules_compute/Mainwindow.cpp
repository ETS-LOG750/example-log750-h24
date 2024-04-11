#include "MainWindow.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

// For images
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace {
	const GLuint numFacesCube = 6;
	const GLuint numTriCube = numFacesCube * 2;
	const GLuint numVerticesCube = 4 * numFacesCube;
}

MainWindow::MainWindow():
	m_camera(m_windowWidth, m_windowHeight,
		glm::vec3(2.0, 2.0, 2.0),
		glm::vec3(0.0, 0.0, 0.0))
{
}

int MainWindow::Initialisation()
{
	// OpenGL version (usefull for imGUI and other libraries)
	const char* glsl_version = "#version 460 core";

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

	// Request OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Particules", NULL, NULL);
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
	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
		MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
		w->CursorPositionCallback(xpos, ypos);
		});
}

int MainWindow::InitializeGL()
{
	// Load and create shaders
	const std::string directory = SHADERS_DIR;
	bool mainShaderSuccess = true;
	m_mainShader = std::make_unique<ShaderProgram>();
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "particules.vert");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "particules.frag");
	mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_GEOMETRY_SHADER, directory + "particules.geo");
	mainShaderSuccess &= m_mainShader->link();
	if (!mainShaderSuccess) {
		std::cerr << "Error when loading main shader\n";
		return 4;
	}

	m_mainUniforms.projMatrix = m_mainShader->uniformLocation("projMatrix");
	m_mainUniforms.viewMatrix = m_mainShader->uniformLocation("viewMatrix");
	m_mainUniforms.globalSize = m_mainShader->uniformLocation("globalSize");
	m_mainUniforms.globalTransparency = m_mainShader->uniformLocation("globalTransparency");
	m_mainUniforms.texture = m_mainShader->uniformLocation("texture");
	m_mainUniforms.useTexture = m_mainShader->uniformLocation("useTexture");
	m_mainUniforms.time = m_mainShader->uniformLocation("time");	
	if(m_mainUniforms.projMatrix == -1 || m_mainUniforms.viewMatrix == -1 || m_mainUniforms.globalSize == -1 || m_mainUniforms.globalTransparency == -1 || m_mainUniforms.texture == -1 || m_mainUniforms.useTexture == -1 || m_mainUniforms.time == -1) {
		std::cerr << "Error when loading main shader uniforms\n";
		return 5;
	}

	// Create compute 
	bool computeShaderSuccess = true;
	m_computeShader = std::make_unique<ShaderProgram>();
	computeShaderSuccess &= m_computeShader->addShaderFromSource(GL_COMPUTE_SHADER, directory + "particules.comp");
	computeShaderSuccess &= m_computeShader->link();
	if (!computeShaderSuccess) {
		std::cerr << "Error when loading compute shader\n";
		return 6;
	}
	m_computeUniforms.dt = m_computeShader->uniformLocation("dt");
	m_computeUniforms.gravity = m_computeShader->uniformLocation("gravity");
	if(m_computeUniforms.dt == -1 || m_computeUniforms.gravity == -1) {
		std::cerr << "Error when loading compute shader uniforms\n";
		std::cerr << "dt: " << m_computeUniforms.dt << " gravity: " << m_computeUniforms.gravity << std::endl;
		return 7;
	}


	// Create the VAO
	glCreateVertexArrays(1, m_VAOs);
	glBindVertexArray(m_VAOs[Particules]);

	// Initialise and create the buffers
	initializeParticles();

	glGenTextures(1, &m_textureID);

	// Ask the library to flip the image vertically
	// This is necessary as TexImage2D assume "The first element corresponds to the lower left corner of the texture image"
	// whereas stb_image load the image such "the first pixel pointed to is top-left-most in the image"
	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;
	std::string img_path = directory + "Particle.png";
	unsigned char* data = stbi_load(img_path.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);
	if (data)
	{
		for (int i = 0; i < width * height; i++) {
			std::cout << (int)data[i] << " ";
		}
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		std::cout << "Texture loaded at path: " << img_path << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << img_path << std::endl;
		stbi_image_free(data);
		return false;
	}

	// Setup projection matrix (a bit hacky)
	FramebufferSizeCallback(m_windowWidth, m_windowHeight);

	return 0;
}

void MainWindow::initializeParticles()
{
	std::cout << "Initialize the particules ... " << m_numberParticles << "\n";
	m_particles.resize(m_numberParticles);
	for (int i = 0; i < m_numberParticles; ++i)
	{
		m_particles[i] = m_settings.createNewParticle();
	}

	if(m_particleBufferCreated) {
		glDeleteBuffers(1, &m_particleBuffer);
		glDeleteBuffers(1, &m_spawnBuffer);
	}
	glCreateBuffers(1, &m_particleBuffer);
	glCreateBuffers(1, &m_spawnBuffer);
	std::cout << " - Create buffer of size: " << m_numberParticles * sizeof(Particle) << "\n";
	glNamedBufferStorage(m_particleBuffer, m_numberParticles * sizeof(Particle), (const void*)m_particles.data(), GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(m_spawnBuffer, m_numberParticles * sizeof(Particle), (const void*)m_particles.data(), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particleBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_spawnBuffer);
	m_particleBufferCreated = true;
}

void MainWindow::RenderImgui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//imgui 
	{
		ImGui::Begin("Particules");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
            1000.0/double(ImGui::GetIO().Framerate), double(ImGui::GetIO().Framerate));

		ImGui::Checkbox("Animate", &m_animate);
		ImGui::Checkbox("Additive blend", &m_useAdditiveBlending);
		if (ImGui::InputInt("Number particules", &m_numberParticles)) {
			m_numberParticles = std::max(0, m_numberParticles);
			initializeParticles();
		}
		ImGui::InputFloat("Speed", &m_speed);
		ImGui::InputFloat("Global Size", &m_size);
		ImGui::InputFloat("Transparency", &m_transparency);
		ImGui::Checkbox("Use Texture?", &m_useTexture);
		m_speed = std::max(0.f, m_speed);
		m_size = std::max(0.000001f, m_size);
		m_transparency = std::max(0.f, std::min(1.f, m_transparency));

		bool changed = false;
		ImGui::Separator();
		ImGui::Text("Generator:");
		changed |= ImGui::InputFloat("Size", &m_settings.size);
		ImGui::Text("Velocity:");
		changed |= ImGui::InputFloat("v_min: ", &m_settings.velocityMin);
		changed |= ImGui::InputFloat("v_max", &m_settings.velocityMax);
		ImGui::Text("Life:");
		changed |= ImGui::InputFloat("l_min", &m_settings.lifeMin);
		changed |= ImGui::InputFloat("l_max", &m_settings.lifeMax);
		m_settings.sanitize();
		if(changed) {
			initializeParticles();
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void MainWindow::RenderScene(float time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_mainShader->bind();
	m_mainShader->setMat4(m_mainUniforms.projMatrix, m_camera.projectionMatrix());
	m_mainShader->setMat4(m_mainUniforms.viewMatrix, m_camera.viewMatrix());
	m_mainShader->setFloat(m_mainUniforms.globalSize, m_size);
	m_mainShader->setFloat(m_mainUniforms.globalTransparency, m_transparency);
	m_mainShader->setFloat(m_mainUniforms.time, glfwGetTime() * 2.f);
	glEnable(GL_BLEND);
	// Choose the blending method
	if (m_useAdditiveBlending)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Activate and use texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_textureID);
	m_mainShader->setInt(m_mainUniforms.texture, 0); // Unit 0
	m_mainShader->setBool(m_mainUniforms.useTexture, m_useTexture);


	// Draw the particles
	glDrawArrays(GL_POINTS, 0, m_numberParticles);

	// Note here we sort async to avoid locking the rendering system
	// Sort according to distance from the camera.
	const glm::vec3 eyePos = m_camera.position();
	std::sort(m_particles.begin(), m_particles.end(),
		[&eyePos](const Particle& a, const Particle& b)
		{
			return glm::dot(eyePos - a.p, eyePos - a.p) > glm::dot(eyePos - b.p, eyePos - b.p);
		});

	glDisable(GL_BLEND);

}

int MainWindow::RenderLoop()
{
	float time = glfwGetTime();
	while (!glfwWindowShouldClose(m_window))
	{
		// Compute delta time between two frames
		float new_time = float(glfwGetTime());
		const float delta_time = new_time - time;
		time = new_time;

		// Check inputs: Does ESC was pressed?
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(m_window, true);
		if (!m_imGuiActive) {
			m_camera.keybordEvents(m_window, delta_time);
		}

		if (m_animate) {
			const glm::vec3 gravity(0, -9.8, 0); // acceleration due to gravity
			m_computeShader->bind();
			m_computeShader->setFloat(m_computeUniforms.dt, delta_time * m_speed);
			m_computeShader->setVec3(m_computeUniforms.gravity, gravity);
			glDispatchCompute(m_numberParticles / 256, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
		RenderScene(time);
		RenderImgui();

		// Show rendering and get events
		glfwSwapBuffers(m_window);
		glfwPollEvents();
		m_imGuiActive = ImGui::IsAnyItemActive();
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
	m_camera.viewportEvents(width, height);
}

void MainWindow::CursorPositionCallback(double xpos, double ypos) {
	if (!m_imGuiActive) {
		int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
		m_camera.mouseEvents(glm::vec2(xpos, ypos), state == GLFW_PRESS);
	}
}