#version 460 core

// Entree du nuanceurs de sommets
layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vColor; // Cette information peut etre desactiver pendant le rendu

// Sortie: couleur sera interpolee et passee dans le nuanceur de fragment
out vec4 ifColor;

void main()
{
     gl_Position = vPosition;
     ifColor = vColor;
}

