#include "MainWindow.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const int NumVertices = 4;

MainWindow::MainWindow() :
    m_at(glm::vec3(0, 0, -1)),
    m_up(glm::vec3(0, 1, 0))
{
}

void MainWindow::FramebufferSizeCallback(int width, int height) {
    m_proj = glm::perspective(45.0f, float(width) / height, 0.01f, 100.0f);
    glViewport(0, 0, width, height);
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
    m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Normal Map", NULL, NULL);
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

glm::vec3 MainWindow::computeTangentFace(vec3x3 pos, vec2x3 uvs) const {
    // Function to compute the tangent from a triangle
    // 1) Compute edges
    glm::vec3 e1 = std::get<1>(pos) - std::get<0>(pos);
    glm::vec3 e2 = std::get<2>(pos) - std::get<0>(pos);
    // 2) compute uvs diff
    glm::vec2 uv1 = std::get<1>(uvs) - std::get<0>(uvs);
    glm::vec2 uv2 = std::get<2>(uvs) - std::get<0>(uvs);

    // Determinant du system de la matrice 2x2. 
    // https://www.chilimath.com/lessons/advanced-algebra/inverse-of-a-2x2-matrix/
    // Evaluate directly the product of inverse matrix and [e1, e2]
    float inv_det = 1.0F / (uv1.x * uv2.y - uv2.x * uv1.y);
    // first line
    glm::vec3 t((uv2.y * e1.x - uv1.y * e2.x) * inv_det,
        (uv2.y * e1.y - uv1.y * e2.y) * inv_det,
        (uv2.y * e1.z - uv1.y * e2.z) * inv_det);
    // If want to compute the bitangent
    // Note that it can be usefull for the headedness
    /*glm::vec3 b((uv1.x * e2.x - uv2.x * e1.x) * inv_det,
        (uv1.x * e2.y - uv2.x * e1.y) * inv_det,
        (uv1.x * e2.z - uv2.x * e1.z) * inv_det);*/

    // Note that normalize might be not necessary
    // if we iterate over all triangle. In this case,
    // the area of the triangle will be backed inside the computed tangent
    return glm::normalize(t);
}

int MainWindow::InitializeGL()
{
    glGenVertexArrays(NumVAOs, m_VAOs);
    glGenBuffers(NumBuffers, m_buffers);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    glm::vec3 Vertices[NumVertices] = {
          glm::vec3(-1, -1, -1),
          glm::vec3( 1, -1, -1),
          glm::vec3(-1,  1, -1),
          glm::vec3( 1,  1, -1)
    };
    glm::vec2 Uvs[NumVertices] = {
        glm::vec2( 0, 0 ),
        glm::vec2( 1, 0 ),
        glm::vec2( 0, 1 ),
        glm::vec2( 1, 1 )
    };
    glm::vec3 Normals[NumVertices] = {
        glm::vec3( 0, 0, 1 ),
        glm::vec3( 0, 0, 1 ),
        glm::vec3( 0, 0, 1 ),
        glm::vec3( 0, 0, 1 )
    };
    glm::vec3 Tangents[NumVertices] = {
      glm::vec3( 1, 0, 0 ),
      glm::vec3( 1, 0, 0 ),
      glm::vec3( 1, 0, 0 ),
      glm::vec3( 1, 0, 0 )
    };

    // Here an example to compute the tangent
    // with class formula
    glm::vec3 t = computeTangentFace(
        {Vertices[0], Vertices[1], Vertices[2]},
        {Uvs[0], Uvs[1], Uvs[2]}
    );
    std::cout << glm::to_string(t) << "\n";

    glBindVertexArray(m_VAOs[Triangles]);

    GLsizeiptr VerticesOffset = 0;
    GLsizeiptr UvsOffset = sizeof(Vertices);
    GLsizeiptr NormalsOffset = UvsOffset + sizeof(Uvs);
    GLsizeiptr TangentsOffset = NormalsOffset +sizeof(Normals);
    GLsizeiptr DataSize = TangentsOffset + sizeof(Tangents);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[ArrayBuffer]);
    glBufferData(GL_ARRAY_BUFFER, DataSize, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, VerticesOffset, sizeof(Vertices), Vertices);
    glBufferSubData(GL_ARRAY_BUFFER, UvsOffset, sizeof(Uvs), Uvs);
    glBufferSubData(GL_ARRAY_BUFFER, NormalsOffset, sizeof(Normals), Normals);
    glBufferSubData(GL_ARRAY_BUFFER, TangentsOffset, sizeof(Tangents), Tangents);

    // build and compile our shader program
    const std::string directory = SHADERS_DIR;
    m_mainShader = std::make_unique<ShaderProgram>();
    bool mainShaderSuccess = true;
    mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "normalmap.vert");
    mainShaderSuccess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "normalmap.frag");
    mainShaderSuccess &= m_mainShader->link();
    if (!mainShaderSuccess) {
        std::cerr << "Error when loading main shader\n";
        return 4;
    }

    m_uniforms.mvMatrix = m_mainShader->uniformLocation("mvMatrix");
    m_uniforms.projMatrix = m_mainShader->uniformLocation("projMatrix");
    m_uniforms.normalMatrix = m_mainShader->uniformLocation("normalMatrix");
    m_uniforms.activateARM = m_mainShader->uniformLocation("activateARM");
    m_uniforms.activateNormalMap = m_mainShader->uniformLocation("activateNormalMap");
    m_uniforms.lightDirection = m_mainShader->uniformLocation("lightDirection");
    if(m_uniforms.mvMatrix == -1 || m_uniforms.projMatrix == -1 || 
       m_uniforms.normalMatrix == -1 || m_uniforms.activateARM == -1 || m_uniforms.activateNormalMap == -1 || m_uniforms.lightDirection == -1) {
        std::cerr << "Unable to find uniform in main shader\n";
        return 4;
    }

    // Setup shader variables
    glUseProgram(m_mainShader->programId());

    int locPos = m_mainShader->attributeLocation("vPosition");
    glVertexAttribPointer(locPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(VerticesOffset));
    glEnableVertexAttribArray(locPos);

    int locNor = m_mainShader->attributeLocation("vNormal");
    glVertexAttribPointer(locNor, 3, GL_FLOAT,GL_TRUE, 0, BUFFER_OFFSET(NormalsOffset));
    glEnableVertexAttribArray(locNor);

    int locTan = m_mainShader->attributeLocation("vTangent");
    glVertexAttribPointer(locTan, 3, GL_FLOAT,GL_TRUE, 0, BUFFER_OFFSET(TangentsOffset));
    glEnableVertexAttribArray(locTan);

    int locUV = m_mainShader->attributeLocation("vUV");
    glVertexAttribPointer(locUV, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(UvsOffset));
    glEnableVertexAttribArray(locUV);

    std::string assets_dir = ASSETS_DIR;
    std::string diffPath = assets_dir + "concrete_debris_diff_1k.jpg";
    std::string normalPath = assets_dir + "concrete_debris_nor_gl_1k.jpg";
    std::string ARMPath = assets_dir + "concrete_debris_arm_1k.jpg";
    std::string dispPath = assets_dir + "concrete_debris_disp_1k.jpg";


    if (!loadTexture(diffPath, m_diffTexID, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)) {
        std::cerr << "Unable to load texture: " << diffPath << std::endl;
        return 4;
    }
    if (!loadTexture(normalPath, m_normalTexID, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)) {
        std::cerr << "Unable to load texture: " << normalPath << std::endl;
        return 4;
    }
    if (!loadTexture(ARMPath, m_ARMTexID, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)) {
        std::cerr << "Unable to load texture: " << ARMPath << std::endl;
        return 4;
    }
    
    // Configure the texture uniform in advances
    {
        GLuint texLoc = m_mainShader->uniformLocation("texColor");
        if(texLoc == -1) {
            std::cerr << "Unable to find uniform texColor" << std::endl;
            return 4;
        }
        m_mainShader->setInt(texLoc, 0);
    }
    {
        GLuint texLoc = m_mainShader->uniformLocation("texNormal");
        if(texLoc == -1) {
            std::cerr << "Unable to find uniform texNormal" << std::endl;
            return 4;
        }
        m_mainShader->setInt(texLoc, 1);
    }
    {
        GLuint texLoc = m_mainShader->uniformLocation("texARM");
        if(texLoc == -1) {
            std::cerr << "Unable to find uniform texARM" << std::endl;
            return 4;
        }
        m_mainShader->setInt(texLoc, 2);
    }

    updateCameraEye();
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

        ImGui::Begin("Normal Map");

        // ImGUI (theta, phi) for controlling the light direction
        ImGui::SliderFloat("Light Theta", &m_light_theta, -180.0f, 180.0f);
        ImGui::SliderFloat("Light Phi", &m_light_phi, -180.0f, 180.0f);

        // Options
        ImGui::Checkbox("Active ARM", &m_activateARM);
        ImGui::Checkbox("Active Normal map", &m_activateNormalMap);

        bool updateCamera = ImGui::SliderFloat("Left Right Slider", &m_longitude, -180.0f, 180.0f);
        updateCamera |= ImGui::SliderFloat("Up Down Slider", &m_latitude, -89.f, 89.f);
        updateCamera |= ImGui::SliderFloat("Forward Backward Slider", &m_distance, 2.f, 14.f);
        if (updateCamera) {
            updateCameraEye();
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void MainWindow::RenderScene()
{
    // render
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(m_VAOs[Triangles]);

    glUseProgram(m_mainShader->programId());

    glm::mat4 LookAt = glm::lookAt(m_eye, m_at, m_up);
    // Note: optimized version of glm::transpose(glm::inverse(...))
    glm::mat3 NormalMat = glm::inverseTranspose(glm::mat3(LookAt));

    m_mainShader->setMat4(m_uniforms.mvMatrix, LookAt);
    m_mainShader->setMat4(m_uniforms.projMatrix, m_proj);
    m_mainShader->setMat3(m_uniforms.normalMatrix, NormalMat);
    m_mainShader->setBool(m_uniforms.activateARM, m_activateARM);
    m_mainShader->setBool(m_uniforms.activateNormalMap, m_activateNormalMap);

    // Compute light direction
    glm::vec3 lightDir = glm::mat3(LookAt) * glm::vec3(
        cos(glm::radians(m_light_theta)) * cos(glm::radians(m_light_phi)),
        sin(glm::radians(m_light_theta)) * cos(glm::radians(m_light_phi)),
        sin(glm::radians(m_light_phi))
    );
    m_mainShader->setVec3(m_uniforms.lightDirection, lightDir);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_diffTexID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalTexID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_ARMTexID);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, NumVertices);
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

void MainWindow::updateCameraEye()
{
    m_eye = glm::vec3(0, 0, m_distance);
    glm::mat4 longitude(1), latitude(1);
    latitude = glm::rotate(latitude,glm::radians(m_latitude),glm::vec3( 1, 0, 0));
    longitude = glm::rotate(longitude, glm::radians(m_longitude), glm::vec3(0, 1, 0));
    m_eye = longitude * latitude * glm::vec4(m_eye,1);
}

bool MainWindow::loadTexture(const std::string& path, unsigned int& textureID, GLint uvMode, GLint minMode, GLint magMode)
{
    glGenTextures(1, &textureID);

    // Ask the library to flip the image horizontally
    // This is necessary as TexImage2D assume "The first element corresponds to the lower left corner of the texture image"
    // whereas stb_image load the image such "the first pixel pointed to is top-left-most in the image"
    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, STBI_rgb_alpha);
    if (data)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        if (minMode == GL_LINEAR_MIPMAP_LINEAR) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uvMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uvMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magMode);

        stbi_image_free(data);

        std::cout << "Texture loaded at path: " << path << std::endl;
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
        return false;
    }

    return true;
}