#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <memory>
#include <vector>

#include "ShaderProgram.h"
#include "Camera.h"

inline float random(float min, float max)
{
	return (max - min) * ((float)rand() / (float)RAND_MAX) + min;
}


// A simple particle object.
struct Particle
{
	// Attributes
	glm::vec3 p = glm::vec3(0.0); // position
	float life = 0.0f;			  // time to live
	glm::vec3 v = glm::vec3(0.0); // velocity
	float size = 0.1f;			  // scaling factor
	glm::vec3 c = glm::vec3(0.0); // RGB color
	float padd = 0.0f;			  // padding
};

struct ParticleGeneratorSettings {
	float size = 0.4f;
	float velocityMin = 5.0f;
	float velocityMax = 10.0f;
	float lifeMin = 0.1f;
	float lifeMax = 1.0f;

	void sanitize() {
		size = std::max(size, 0.0001f);
		velocityMin = std::max(velocityMin, 0.0f);
		velocityMax = std::max(velocityMax, velocityMin);
		lifeMin = std::max(lifeMin, 0.0f);
		lifeMax = std::max(lifeMax, lifeMin);
	}

	Particle createNewParticle()
	{
		Particle p;
		p.p = glm::vec3(0, 0, 0);
		const float vx = random(-size, size);
		const float vz = random(-size, size);
		p.v = glm::vec3(vx, random(velocityMin, velocityMax), vz);
		p.life = random(lifeMin, lifeMax);
		const float r = random(0.5, 1.0);
		const float g = random(0.0, 0.5);
		const float b = random(0.0, 0.5);
		p.c = glm::vec3(r, g, b);
		p.size = random(0.1f, 0.25f);
		return p;
	}


};

class MainWindow
{
public:
	MainWindow();

	// Main functions (initialization, run)
	int Initialisation();
	int RenderLoop();

	// Callback to intersept GLFW calls
	void FramebufferSizeCallback(int width, int height);
	void CursorPositionCallback(double xpos, double ypos);

private:
	// Initialize GLFW callbacks
	void InitializeCallback();
	
	// Intiialize OpenGL objects (shaders, ...)
	int InitializeGL();
	void initializeParticles();

	// Rendering scene (OpenGL)
	void RenderScene(float t);
	void RenderImgui();
	void Step(float t);

	glm::mat4 transform(float v) const;

private:
	// Camera
	Camera m_camera;
	bool m_imGuiActive = false;

	// settings
	const unsigned int m_windowWidth = 800;
	const unsigned int m_windowHeight = 800;
	GLFWwindow* m_window = nullptr;

	bool m_animate = true;
	float m_time = 0.0;
	
	// Texture
	GLuint m_textureID;

	// Storage buffer
	enum VAO_IDs { Particules, NumVAOs };
	GLuint m_VAOs[NumVAOs];
	GLuint m_particleBuffer;
	GLuint m_spawnBuffer;
	bool m_particleBufferCreated = true;

	// Compute shader
	std::unique_ptr<ShaderProgram> m_computeShader = nullptr;
	struct {
		GLint dt;
		GLint gravity;
	} m_computeUniforms;
	
	// Particules
	ParticleGeneratorSettings m_settings;
	std::vector<Particle> m_particles;
	bool m_useAdditiveBlending = true;
	int m_numberParticles = 3000;
	float m_speed = 1.0f;
	float m_size = 0.05f;
	float m_transparency = 1.0f;
	bool m_useTexture = false;

	// Shader
	std::unique_ptr<ShaderProgram> m_mainShader = nullptr;
	struct {
		GLint viewMatrix;
		GLint projMatrix;
		GLint globalSize;
		GLint globalTransparency;
		GLint texture;
		GLint useTexture;
		GLint time;
	} m_mainUniforms;
};
