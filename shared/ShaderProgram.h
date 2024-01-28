#pragma once
#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtx/string_cast.hpp>

// Macro for detecting an openGL error.
// Only works if the program is compiled in debug mode.
// 
// Usage:
// GL_CHECK(myfunction); // In case of OpenGL call
// auto v = GL_CHECK(myfunction_withreturn); // In case of function returning values
// GL_CHECK("My checkpoint"); // In case of no function call
// 
// Note calling GL_CHECK(); will result to an error (arg stmt is missing)
inline void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
	GLenum err;
	bool terminate = false;
    while ((err = glGetError()) != GL_NO_ERROR)
	{
		terminate = true;
		// Flush a error message in the error exit
		std::cerr << "OpenGL error " << (int)err << " at " << fname << ":" << line << " for " << stmt << "\n";
		std::cerr.flush();
	}
	if (terminate)
	{
		// Terminate the program. 
		// Can be commented out if needed
		abort();
	}
}
#ifdef _DEBUG
#define GL_CHECK(stmt) stmt;  \
        CheckOpenGLError(#stmt, __FILE__, __LINE__);
#else
#define GL_CHECK(stmt) stmt
#endif

// Other function to check OpenGL error
// From https://learnopengl.com/In-Practice/Debugging
void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam);

// Helper object that simplify the shader loading and interactions
// Can be extended if necessary
class ShaderProgram
{
public:
   // ------------------------------------------------------------------------
   // constructor
   ShaderProgram();
   
   // ------------------------------------------------------------------------
   // attach shader from sources 
   // return true if sucessfull
   bool addShaderFromSource(GLenum type, const std::string& path);
   
   // ------------------------------------------------------------------------
   // link the different shaders to make a full program 
   // return true if sucessfull
   bool link();

   // ------------------------------------------------------------------------
   // get program ID to interact directly with the shader program
   inline GLuint programId() const { return m_ID; }

   // ------------------------------------------------------------------------
   // use shader program
   inline void bind() const { 
       if(!m_linked) {
           // Warn user
           std::cerr << "Shader is not properly linked!\n";
       }
       glUseProgram(m_ID); 
   }
   

   // get id value corresponding to attribute
    // ------------------------------------------------------------------------
   inline GLint attributeLocation(const char* name) const { return glGetAttribLocation(m_ID, name); }
   inline GLint attributeLocation(const std::string name) const { return glGetAttribLocation(m_ID, name.c_str()); }
   inline GLint uniformLocation(const char* name) const { return glGetUniformLocation(m_ID, name); }
   inline GLint uniformLocation(const std::string name) const { return glGetUniformLocation(m_ID, name.c_str()); }

    // utility uniform functions
    // ------------------------------------------------------------------------
    // OLD setUniformValue 
    inline void setUniformValue(GLint location, bool value) const { glProgramUniform1i(m_ID, location, (int)value); }
	inline void setUniformValue(GLint location, int value) const { glProgramUniform1i(m_ID, location, value); }
	inline void setUniformValue(GLint location, float value) const { glProgramUniform1f(m_ID, location, value); }
	inline void setUniformValue(GLint location, const glm::mat4& mat) const { glProgramUniformMatrix4fv(m_ID, location, 1, GL_FALSE, &mat[0][0]); }
	inline void setUniformValue(GLint location, const glm::mat3& mat) const { glProgramUniformMatrix3fv(m_ID, location, 1, GL_FALSE, &mat[0][0]); }
	inline void setUniformValue(GLint location, const glm::vec4& vec) const { glProgramUniform4fv(m_ID, location, 1, &vec[0]); }
	inline void setUniformValue(GLint location, const glm::vec3& vec) const { glProgramUniform3fv(m_ID, location, 1, &vec[0]); }
    // Safer version
    inline void setBool(GLint location, bool value) const { glProgramUniform1i(m_ID, location, (int)value); }
	inline void setInt(GLint location, int value) const { glProgramUniform1i(m_ID, location, value); }
	inline void setFloat(GLint location, float value) const { glProgramUniform1f(m_ID, location, value); }
	inline void setMat4(GLint location, const glm::mat4& mat) const { glProgramUniformMatrix4fv(m_ID, location, 1, GL_FALSE, &mat[0][0]); }
	inline void setMat3(GLint location, const glm::mat3& mat) const { glProgramUniformMatrix3fv(m_ID, location, 1, GL_FALSE, &mat[0][0]); }
	inline void setVec4(GLint location, const glm::vec4& vec) const { glProgramUniform4fv(m_ID, location, 1, &vec[0]); }
	inline void setVec3(GLint location, const glm::vec3& vec) const { glProgramUniform3fv(m_ID, location, 1, &vec[0]); }
    inline void setVec2(GLint location, const glm::vec2& vec) const { glProgramUniform2fv(m_ID, location, 1, &vec[0]); }

private:
    // Shader program id
    GLuint m_ID;
    // Is the shader linked?
    bool m_linked = false;
    // List of the different shaders (can be reused if necessary)
    std::map<std::string, GLuint> m_shaders_ids;
};

inline std::ostream& operator<<(std::ostream& out, const glm::vec2& g)
{
	return out << glm::to_string(g);
}

inline std::ostream& operator<<(std::ostream& out, const glm::vec3& g)
{
	return out << glm::to_string(g);
}

inline std::ostream& operator<<(std::ostream& out, const glm::vec4& g)
{
	return out << glm::to_string(g);
}

inline std::ostream& operator<<(std::ostream& out, const glm::mat3& g)
{
	return out << glm::to_string(g);
}

inline std::ostream& operator<<(std::ostream& out, const glm::mat4& g)
{
	return out << glm::to_string(g);
}

#endif