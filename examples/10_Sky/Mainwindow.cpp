#include "MainWindow.h"

#include <vector>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

MainWindow::MainWindow() :
    m_camera(m_windowWidth, m_windowHeight,
        glm::vec3(2.0, 2.0, 2.0),
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Sky", NULL, NULL);
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
    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
        MainWindow* w = reinterpret_cast<MainWindow*>(glfwGetWindowUserPointer(window));
        w->CursorPositionCallback(xpos, ypos);
        });

}

void MainWindow::FramebufferSizeCallback(int width, int height) {
    m_windowWidth = width;
    m_windowHeight = height;
    glViewport(0, 0, width, height);
    m_camera.viewportEvents(width, height);
}

void MainWindow::CursorPositionCallback(double xpos, double ypos) {
    if (!m_imGuiActive) {
        int state = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT);
        m_camera.mouseEvents(glm::vec2(xpos, ypos), state == GLFW_PRESS);
    }
}

int MainWindow::InitializeGL()
{
    // build and compile our shader program
    const std::string directory = SHADERS_DIR;
    // --- Sky shader
    m_skydomeShader = std::make_unique<ShaderProgram>();
    bool loadSucess = true;
    loadSucess &= m_skydomeShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "skydome.vert");
    loadSucess &= m_skydomeShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "skydome.frag");
    loadSucess &= m_skydomeShader->link();
    if (!loadSucess) {
        std::cerr << "Error when loading sky shader\n";
        return 4;
    }
    if ((m_vSkyPos = m_skydomeShader->attributeLocation("vPosition")) < 0) {
        std::cerr << "Unable to find shader location for " << "vPosition" << std::endl;
        return 3;
    }
    m_uSky.viewRotMatrix = m_skydomeShader->uniformLocation("viewRotMatrix");
    m_uSky.projMatrix = m_skydomeShader->uniformLocation("projMatrix");

    // --- Normal
    m_mainShader = std::make_unique<ShaderProgram>();
    loadSucess &= m_mainShader->addShaderFromSource(GL_VERTEX_SHADER, directory + "main.vert");
    loadSucess &= m_mainShader->addShaderFromSource(GL_FRAGMENT_SHADER, directory + "main.frag");
    loadSucess &= m_mainShader->link();
    if (!loadSucess) {
        std::cerr << "Error when loading main shader\n";
        return 4;
    }
    if ((m_vMainPos = m_mainShader->attributeLocation("vPosition")) < 0) {
        std::cerr << "Unable to find shader location for " << "vPosition" << std::endl;
        return 3;
    }
    if ((m_vMainNormal = m_mainShader->attributeLocation("vNormal")) < 0) {
        std::cerr << "Unable to find shader location for " << "vNormal" << std::endl;
        return 3;
    }
    m_uMain.mvMatrix = m_mainShader->uniformLocation("mvMatrix");
    m_uMain.projMatrix = m_mainShader->uniformLocation("projMatrix");
    m_uMain.cameraPos = m_mainShader->uniformLocation("cameraPos");
    m_uMain.texSkydome = m_mainShader->uniformLocation("texSkydome");
    m_uMain.useFresnel = m_mainShader->uniformLocation("useFresnel");
    if(m_uMain.texSkydome < 0 | m_uMain.mvMatrix < 0 | m_uMain.projMatrix < 0 | m_uMain.cameraPos < 0 | m_uMain.useFresnel < 0) {
        std::cerr << "Unable to find shader location for " << "mvMatrix, projMatrix, cameraPos, texSkydome, useFresnel" << std::endl;
        return 3;
    }

    std::cout << "Load textures ... \n";
    std::string assets_dir = ASSETS_DIR;
    std::string SkydomePath = assets_dir + "skydome2.png";
    if (!loadTexture(&SkydomePath[0], TextureId, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR)) {
        std::cerr << "Error when loading image skydome2.png\n";
        return 5;
    }

    // Set texture unit (all 0)
    m_skydomeShader->setInt(m_uSky.texSkydome, 0);
    m_mainShader->setInt(m_uMain.texSkydome, 0);

    std::cout << "Load geometry ... \n";
    glGenVertexArrays(NumVAOs, m_VAOs);
    glGenBuffers(NumBuffers, m_buffers);
    initGeometrySphere();

    glEnable(GL_DEPTH_TEST);
    std::cout << "Intialization finished\n";

    // Setup projection matrix (a bit hacky)
    FramebufferSizeCallback(m_windowWidth, m_windowHeight);

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

        ImGui::Begin("Sky");

        if(ImGui::Checkbox("Use Fresnel", &m_useFresnel)) {
            m_mainShader->setInt(m_uMain.useFresnel, m_useFresnel);
        }

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void MainWindow::RenderScene()
{
    // render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(m_VAOs[VAO_Sphere_Sky]);
    glUseProgram(m_skydomeShader->programId());
    glm::mat4 modelMatrix = glm::mat4(1.0);
    glm::mat4 viewMatrix = m_camera.viewMatrix();
    m_skydomeShader->setMat4(m_uSky.projMatrix, m_camera.projectionMatrix());
    m_skydomeShader->setMat3(m_uSky.viewRotMatrix, glm::mat3(viewMatrix));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glDrawElements(GL_TRIANGLES, numTriSphere * 3, GL_UNSIGNED_INT, 0);
    
    glEnable(GL_DEPTH_TEST);
    glm::mat3 normalMat = glm::inverseTranspose(glm::mat3(viewMatrix));
    glBindVertexArray(m_VAOs[VAO_Sphere]);
    glUseProgram(m_mainShader->programId());
    m_mainShader->setMat4(m_uMain.projMatrix, m_camera.projectionMatrix());
    m_mainShader->setMat4(m_uMain.mvMatrix, viewMatrix);
    m_mainShader->setVec3(m_uMain.cameraPos, m_camera.position());
    glDrawElements(GL_TRIANGLES, numTriSphere * 3, GL_UNSIGNED_INT, 0);
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
        if (!m_imGuiActive) {
            m_camera.keybordEvents(m_window, delta_time);
        }

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

void MainWindow::initGeometrySphere()
{
    // Generate a sphere between [-0.5, 0.5]x[-0.5, 0.5]x[-0.5, 0.5]

    //Note: To ease the sphere creation, we use an index(aka elements) buffer.This allows us to create
    //			 each vertex once. Afterward, faces are created by specifying the index of the three vertices
    //			 inside the index buffer. For example, a 2D rectangle could be drawn using the following vertex and
    //       index buffers:
    //
    //	     vertices[4][2] = {{-1,-1},{1,-1},{1,1},{-1,1}};
    //       indices[2*3] = {0, 1, 3, 1, 2, 3};
    //
    //       In this example, the vertex buffer contains 4 vertices, and the index buffer contains two
    //       triangles formed by the vertices (vertices[0], vertices[1], vertices[3]) and (vertices[1],
    //       vertices[2], vertices[3]) respectively.
    //
    //       Also note that indices are stored in a different type of buffer called Element Array Buffer.

    // Create sphere vertices and faces
    std::vector<GLfloat> vertices;
    std::vector<GLfloat> normals;
    std::vector<GLuint> indices;


    // Generate surrounding vertices
    float thetaInc = 2.0f * 3.14159265f / static_cast<float>(numColSphere);
    float phiInc = 3.14159265f / static_cast<float>(numRowSphere + 1); // note the +1 here
    const float radius = 0.9;
    for (int row = 0; row < numRowSphere; ++row)
    {
        // Phi ranges from Pi - Pi * 1 / (numRowSphere+1) to Pi - Pi * (numRowSphere) / (numRowSphere + 1)
        // Which is equivalent to :
        // Pi * numRowSphere/(numRowSphere+1) to Pi * 1/numRowSphere
        // You can think of Phi as sweeping the sphere from the South pole to the North pole
        float phi = 3.14159265f - (static_cast<float>(row + 1) * phiInc);
        for (int col = 0; col < numColSphere; ++col)
        {
            // Theta ranges from 0 to 2*Pi (numColSphere-1)/numColSphere
            // You can think of Theta as circling around the sphere, East to West
            float theta = col * thetaInc;

            // Spherical coordinates 
            glm::vec3 pos = glm::vec3(radius * sin(theta) * sin(phi), radius * cos(phi), radius * cos(theta) * sin(phi));
            vertices.push_back(pos.x);
            vertices.push_back(pos.y);
            vertices.push_back(pos.z);

            normals.push_back(pos.x);
            normals.push_back(pos.y);
            normals.push_back(pos.z);
        }
    }

    // Generate cap vertices
    vertices.push_back(0.0f);
    vertices.push_back(-radius);
    vertices.push_back(0.0f);

    normals.push_back(0.0f);
    normals.push_back(-1.0f);
    normals.push_back(0.0f);

    vertices.push_back(0.0f);
    vertices.push_back(radius);
    vertices.push_back(0.0f);

    normals.push_back(0.0f);
    normals.push_back(1.0f);
    normals.push_back(0.0f);


    // Generate surrounding indices (faces)
    for (int row = 0; row < numRowSphere - 1; ++row)
    {
        unsigned int rowStart = row * numColSphere;
        unsigned int topRowStart = rowStart + numColSphere;

        for (int col = 0; col < numColSphere; ++col)
        {
            // Compute quad vertices
            unsigned int v = rowStart + col;
            unsigned int vi = (col < numColSphere - 1) ? v + 1 : rowStart;
            unsigned int vj = topRowStart + col;
            unsigned int vji = (col < numColSphere - 1) ? vj + 1 : topRowStart;

            // Add to indices
            indices.push_back(v);
            indices.push_back(vi);
            indices.push_back(vj);
            indices.push_back(vi);
            indices.push_back(vji);
            indices.push_back(vj);
        }
    }

    // Generate cap indices (faces)
    for (int col = 0; col < numColSphere; ++col)
    {
        indices.push_back(numColSphere * numRowSphere);
        indices.push_back((col < numColSphere - 1) ? col + 1 : 0);
        indices.push_back(col);

        unsigned int rowStart = (numRowSphere - 1) * numColSphere;
        indices.push_back(numColSphere * numRowSphere + 1);
        indices.push_back(rowStart + col);
        indices.push_back((col < numColSphere - 1) ? (rowStart + col + 1) : rowStart);
    }


    // Copy indices informations
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Sphere_Position]);
    glBufferData(GL_ARRAY_BUFFER, long(sizeof(GLfloat) * vertices.size()), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Sphere_Normal]);
    glBufferData(GL_ARRAY_BUFFER, long(sizeof(GLfloat) * normals.size()), normals.data(), GL_STATIC_DRAW);


    // Fill in indices EBO
    // Note: The current VAO will remember the call to glBindBuffer for a GL_ELEMENT_ARRAY_BUFFER.
    //			 However, we will need to call glDrawElements() instead of glDrawArrays().
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[EBO_Sphere]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // VAO for the skybox
    glBindVertexArray(m_VAOs[VAO_Sphere_Sky]);
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Sphere_Position]);
    glVertexAttribPointer(m_vSkyPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(m_vSkyPos);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[EBO_Sphere]);

    // VAO for the normal shader
    glBindVertexArray(m_VAOs[VAO_Sphere]);
    // - Position
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Sphere_Position]);
    glVertexAttribPointer(m_vMainPos, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(m_vMainPos);
    // - Normals
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VBO_Sphere_Normal]);
    glVertexAttribPointer(m_vMainNormal, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(0));
    glEnableVertexAttribArray(m_vMainNormal);
    // - Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[EBO_Sphere]);
    
    // Cleanup (optional)

    // Do not desactivate EBO when the VAO is still activated
    // as it will desactivate the EBO for this VAO 
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}