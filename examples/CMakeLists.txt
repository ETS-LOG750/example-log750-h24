include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)
set(SHARED_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ShaderProgram.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/ShaderProgram.h
)


# Seance 01: Introduction
# - imGUI example
add_subdirectory(01_imGUIExample)
add_subdirectory(01_imGUIDemo)
# - draw 2 triangles (simplest example)
add_subdirectory(01_Triangles)