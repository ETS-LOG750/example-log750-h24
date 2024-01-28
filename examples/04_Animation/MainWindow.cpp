#include "MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const GLuint NumVertices = 9;

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
	m_mainShader_matrix = m_mainShader->uniformLocation("uMatrix");
	m_mainShader_matrixNormal = m_mainShader->uniformLocation("uMatrixNormal");
	if (m_mainShader_matrix == -1 || m_mainShader_matrixNormal == -1) {
		std::cerr << "Error when loading main shader (uniform uMatrix or uMatrixNormal)\n";
		return 4;
	}

	// Create VAO / VBO
	glGenVertexArrays(NumVAOs, m_VAOs);
	glBindVertexArray(m_VAOs[Triangles]);
	glGenBuffers(NumBuffers, m_VBOs);

	// Update geometry load it on the GPU
	updateGeometry();

	// Position
	glBindBuffer(GL_ARRAY_BUFFER, m_VAOs[Position]);
	int PositionLocation = m_mainShader->attributeLocation("vPosition");
	if (PositionLocation == -1) {
		std::cerr << "Error when loading main shader (attribute vPosition)\n";
		return 4;
	}
	glVertexAttribPointer(PositionLocation, 
		3, 
		GL_FLOAT,
		GL_FALSE, 
		sizeof(glm::vec3), // Stride: glm::vec3
		BUFFER_OFFSET(0)
	);
	glEnableVertexAttribArray(PositionLocation);

	// Normal
	glBindBuffer(GL_ARRAY_BUFFER, m_VAOs[Normal]);
	int NormalLocation = m_mainShader->attributeLocation("vNormal");
	if (NormalLocation == -1) {
		std::cerr << "Error when loading main shader (attribute vNormal)\n";
		return 4;
	}
	glVertexAttribPointer(NormalLocation,
		3,
		GL_FLOAT,
		GL_TRUE,
		sizeof(glm::vec3), // Stride: taille d'un sommet
		BUFFER_OFFSET(0)
	);
	glEnableVertexAttribArray(NormalLocation);

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
		// Astuce memoire 
		static float angle1 = 0.0;
		static float angle2 = 0.0;

		ImGui::Begin("Transformations");
		const char* items[] = { 
			"Euler", 
			"Quaternion", 
			"Interpolation Euler",
			"Interpolation Quat"
		};
		ImGui::Combo("Mode", &m_selected_mode, items, IM_ARRAYSIZE(items));

		if (m_selected_mode == 0) {
			// Euler
			ImGui::SliderAngle("Rotation x", &m_rot1.x);
			ImGui::SliderAngle("Rotation y", &m_rot1.y);
			ImGui::SliderAngle("Rotation z", &m_rot1.z);
		}
		else if (m_selected_mode == 1) {
			// Quaternion
			if (ImGui::SliderFloat("Angle", &angle1, 0, 360)) {
				m_quat1_angle = glm::radians(angle1);
			}
			if (ImGui::SliderFloat3("Axis", &m_quat1_axis[0], -1, 1)) {
				// Avoid 0 division
				if (glm::length(m_quat1_axis) < 0.00001) {
					m_quat1_axis = glm::vec3(0, 0, 1);
				}
				m_quat1_axis = glm::normalize(m_quat1_axis);
			}
		}
		else if (m_selected_mode == 2) {
			ImGui::Text("Rotation 1:");
			ImGui::SliderAngle("Rotation 1 x", &m_rot1.x);
			ImGui::SliderAngle("Rotation 1 y", &m_rot1.y);
			ImGui::SliderAngle("Rotation 1 z", &m_rot1.z);
			ImGui::Separator();
			ImGui::Text("Rotation 2:");
			ImGui::SliderAngle("Rotation 2 x", &m_rot2.x);
			ImGui::SliderAngle("Rotation 2 y", &m_rot2.y);
			ImGui::SliderAngle("Rotation 2 z", &m_rot2.z);
		}
		else if (m_selected_mode == 3) {
			ImGui::Text("Quaternion 1:");
			if (ImGui::SliderFloat("Angle 1", &angle1, 0, 360)) {
				m_quat1_angle = glm::radians(angle1);
			}
			if (ImGui::SliderFloat3("Axis 1", &m_quat1_axis[0], -1, 1)) {
				// Avoid 0 division
				if (glm::length(m_quat1_axis) < 0.00001) {
					m_quat1_axis = glm::vec3(0, 0, 1);
				}
				m_quat1_axis = glm::normalize(m_quat1_axis);
			}
			ImGui::Separator();
			ImGui::Text("Quaternion 2:");
			if (ImGui::SliderFloat("Angle 2", &angle2, 0, 360)) {
				m_quat2_angle = glm::radians(angle2);
			}
			if (ImGui::SliderFloat3("Axis 2", &m_quat2_axis[0], -1, 1)) {
				// Avoid 0 division
				if (glm::length(m_quat2_axis) < 0.00001) {
					m_quat1_axis = glm::vec3(0, 0, 1);
				}
				m_quat2_axis = glm::normalize(m_quat2_axis);
			}
			ImGui::Checkbox("Linear", &m_linear);
		}

		if (m_selected_mode == 2 || m_selected_mode == 3) {
			ImGui::Separator();
			ImGui::SliderFloat("Time", &m_time, 0, 1);
			ImGui::Checkbox("Animate", &m_animate);
		}
		
		if (ImGui::Button("Reset")) {
			m_rot1 = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
			m_rot2 = glm::vec3(glm::radians(0.0f), glm::radians(0.0f), glm::radians(0.0f));
			m_quat1_angle = 0;
			m_quat1_axis = glm::vec3(0, 0, 1);
			m_quat2_angle = 0;
			m_quat2_axis = glm::vec3(0, 0, 1);
		}

		static bool from_euler = true;
		ImGui::Checkbox("From euler", &from_euler);

		if(ImGui::Button("Copy configuration")) {
			// q1 
			if (from_euler) {
				glm::quat q = glm::quat(glm::eulerAngleXYZ(m_rot1.x, m_rot1.y, m_rot1.z));
				m_quat1_axis = glm::axis(q);
				m_quat1_angle = glm::angle(q);
			} else {
				m_rot1 = glm::eulerAngles(glm::quat(m_quat1_angle, m_quat1_axis));
			}
			// q2
			if (from_euler) {
				glm::quat q = glm::quat(glm::eulerAngleXYZ(m_rot2.x, m_rot2.y, m_rot2.z));
				m_quat2_axis = glm::axis(q);
				m_quat2_angle = glm::angle(q);
			} else {
				m_rot2 = glm::eulerAngles(glm::quat(m_quat2_angle, m_quat2_axis));
			}

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

	// Ici au lieu de definir un delta
	// utilisation du temps absolu
	if (m_animate) {
		float t = (float)glfwGetTime();
		t = std::fmod((t / 3.0f), 1.0f);
		m_time = t;
	}

	// Compute transformation
	glm::mat4 m = glm::mat4(1);
	if (m_selected_mode == 0) {
		m = glm::eulerAngleXYZ(m_rot1.x, m_rot1.y, m_rot1.z);
	}
	else if (m_selected_mode == 1) {
		m = glm::mat4_cast(glm::angleAxis(m_quat1_angle, m_quat1_axis));
	}
	else if (m_selected_mode == 2) {
		glm::vec3 angle = m_time * m_rot2 + (1 - m_time) * m_rot1;
		m = glm::eulerAngleXYZ(angle.x, angle.y, angle.z);
	}
	else if (m_selected_mode == 3) {
		glm::quat q1 = glm::angleAxis(m_quat1_angle, m_quat1_axis);
		glm::quat q2 = glm::angleAxis(m_quat2_angle, m_quat2_axis);
		glm::quat q;
		if (m_linear) {
			// Lerp
			q = glm::normalize(m_time * q2 + (1 - m_time) * q1); // Interpolation lineaire
		}
		else {
			// Slerp
			float theta = acos(glm::dot(q1, q2));
			float sinTheta = sin(theta);
			// Avoid 0 division
			if (sinTheta > 0.00001) {
				q = (sin(theta * (1 - m_time)) * q1 + sin(m_time * theta) * q2) / sinTheta;
			} else {
				q = q1; 
			}
		}
		// Transformation en mat4
		m = glm::mat4_cast(q);
	}


	m_mainShader->setUniformValue(m_mainShader_matrix, m);
	m_mainShader->setUniformValue(m_mainShader_matrixNormal, glm::inverseTranspose(glm::mat3(m)));
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_vertices.size());
	

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

void MainWindow::updateGeometry()
{
	// Clear CPU memory
	m_vertices.clear();
	m_normals.clear();

	// Base positions
	const float PI = 3.14159265358979323846f;
	const float offset = PI * 2.f / (float)COUNT;
	const float RTOP = 0.5f;
	const float RBOT = 0.8f;
	
	for (unsigned int i = 0; i < COUNT; i++) {
		// Calcul des positions
		glm::vec3 p1bot = glm::vec3(
			RBOT * std::cos(offset * i), 
			-0.7, 
			RBOT *  std::sin(offset * i)
		);
		glm::vec3 p1top = glm::vec3(
			RTOP * std::cos(offset * i),
			0.7,
			RTOP * std::sin(offset * i)
		);
		glm::vec3 p2bot = glm::vec3(
			RBOT *  std::cos(offset * (i + 1)), 
			-0.7, 
			RBOT *  std::sin(offset * (i + 1))
		);
		glm::vec3 p2top = glm::vec3(
			RTOP * std::cos(offset * (i + 1)),
			0.7,
			RTOP * std::sin(offset * (i + 1))
		);

		// Triangle 1
		m_vertices.push_back(p1top);
		m_vertices.push_back(p1bot);
		m_vertices.push_back(p2bot);
		// Triangle 2
		m_vertices.push_back(p2top);
		m_vertices.push_back(p1top);
		m_vertices.push_back(p2bot);

		
		// Calcul des normales a chaque sommets
		// Note: p1top et p1bot ont les meme normales
		glm::vec3 n1 = glm::normalize(p1bot - glm::vec3(0, -0.7, 0));
		glm::vec3 n2 = glm::normalize(p2bot - glm::vec3(0, -0.7, 0));
		// Triangle 1
		m_normals.push_back(n1);
		m_normals.push_back(n1);
		m_normals.push_back(n2);
		// Triangle 2
		m_normals.push_back(n2);
		m_normals.push_back(n1);
		m_normals.push_back(n2);
		
	}
	
	// Add data on the GPU (position)
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Position]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * m_vertices.size(),
		m_vertices.data(),
		GL_STATIC_DRAW);
	// Add data on the GPU (normal)
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[Normal]);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(glm::vec3) * m_normals.size(),
		m_normals.data(),
		GL_STATIC_DRAW);

	// Optional: Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}