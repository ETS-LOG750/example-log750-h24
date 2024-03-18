#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>
#include <tuple>

#include "ShaderProgram.h"

typedef std::tuple<glm::vec3, glm::vec3, glm::vec3> vec3x3;
typedef std::tuple<glm::vec2, glm::vec2, glm::vec2> vec2x3;

class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);
private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();

	// Rendering scene (OpenGL)
	void RenderScene();
	// Rendering interface ImGUI
	void RenderImgui();

	void updateCameraEye();

	bool loadTexture(const std::string& path, unsigned int& textureID,
		GLint uvMode, // UV/ST warp mode
		GLint minMode,  // Minfication
		GLint magMode); // Magnification
	
	glm::vec3 computeTangentFace(vec3x3 pos, vec2x3 uvs) const;

private:
	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 720;

	float m_longitude = 0.0f;
	float m_latitude = 0.0f;
	float m_distance = 8.0f;
	bool m_activateARM = true;
	bool m_activateNormalMap = true;

	float m_light_theta = 0.0f;
	float m_light_phi = 0.0f;

	glm::vec3 m_eye, m_at, m_up;
	glm::mat4 m_proj;

	unsigned int m_diffTexID = -1 ;
	unsigned int m_normalTexID = -1;
	unsigned int m_ARMTexID = -1;

	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { ArrayBuffer, NumBuffers };

	// GLFW Window
	GLFWwindow* m_window = nullptr;

	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct { 
		GLuint mvMatrix;
		GLuint projMatrix;
		GLuint normalMatrix;
		GLuint activateARM;
		GLuint activateNormalMap;
		GLuint lightDirection; 
	} m_uniforms;

};
