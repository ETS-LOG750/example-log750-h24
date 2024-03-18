#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <memory>

#include "ShaderProgram.h"

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

	// Update camera position (eye)
	void updateCameraEye();

	// Load texture
	bool loadTexture(const std::string& path, unsigned int& textureID,
		GLint uvMode, // UV/ST warp mode
		GLint minMode,  // Minfication
		GLint magMode); // Magnification

private:
	// settings
	const unsigned int SCR_WIDTH = 900;
	const unsigned int SCR_HEIGHT = 720;

	// Basic camera control
	float m_longitude = 0.0f;
	float m_latitude = 0.0f;
	float m_distance = 3.0f;

	glm::vec3 m_eye, m_at, m_up;
	glm::mat4 m_proj;

	// Geometry (Position, normal, UV)
	enum VAO_IDs { Triangles, NumVAOs };
	enum Buffer_IDs { ArrayBuffer, NumBuffers };

	GLuint m_VAOs[NumVAOs];
	GLuint m_buffers[NumBuffers];

	// Textures
	unsigned int m_textureDiffuseID = -1;
	unsigned int m_textureARMID = -1;
	int m_mode = 0;
	bool m_activateARM = true;

	// GLFW Window
	GLFWwindow* m_window = nullptr;

	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct {
		GLint mvMatrix;
		GLint projMatrix;
		GLint normalMatrix;
		// Textures
		GLint textureDiffuse;
		GLint textureARM;
		GLint activateARM;
	} m_uniforms;
};
