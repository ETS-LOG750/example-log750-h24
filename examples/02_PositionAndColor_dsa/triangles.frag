#version 460 core

// Couleur uniforme (si donnee)
layout(location = 0) uniform vec4 uColor;

// Entree de la couleur interpolee (vertex shader)
in vec4 ifColor;

// Tampon de couleur (image de sortie)
layout (location = 0) out vec4 fColor;

void
main()
{
    if (length(ifColor.rgb) > 0) {
        // Si on a une couleur non noire interpolee
        // on utilise cette couleur. 
        // Pour rappel, si l'information des sommets etait absente
        // la couleur par defaut est: (0, 0, 0, 1)
        fColor = ifColor;
    }
    else
    {
        // Pas de couleur associee aux sommets
        // Ici on montre un usage de alpha
        if(uColor.a != 0) {
            // Si on a alpha non nul: utilisation de la couleur donnee
            fColor = uColor;
        }
        else
        {
            // sinon utilisation d'une couleur par defaut (blue)
            fColor = vec4(0.0, 0.0, 1.0, 1.0);
        }
    }
}