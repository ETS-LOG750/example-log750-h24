# Exemples pour le cours LOG750 (H2024)

Les code d'exemples sont visible dans le dossier `examples`.

Suivre les instructions sur ENA pour la mise en place de l'environment de travail:
- Windows: Visual Studio ou Visual Studio Code
- MacOS ou Linux: Visual Studio Code

**Version OpenGL**: 4.3 ou 4.6

## Cours 01 (OpenGL, imGUI)

- `01_imGUIDemo`: Demo de toutes les fonctionalités de imGUI. Code disponible: https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp 

- `01_imGUIExample`: Demo simple de comment utiliser imGUI pour les laboratoires.

- `01_Triangles` : Application OpenGL (4.3) avec binding qui dessine deux triangles. les concepts couvert sont:
  - Création d'un shader simple (vertex avec une entrée, couleur de sortie fixe)
  - Création d'un VBO (contenant les positions en 2D) et VAO

- `01_Triangles_bindless` : Application **OpenGL (4.6)** identique à `01_Triangles` mais en utilisant bindless/DSA.

