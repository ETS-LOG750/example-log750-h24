#pragma once

#include <glm/gtc/matrix_transform.hpp>

inline glm::mat4 question_transform(float v) {
	// TODO: Calculer la transformation anim�e hierachique
	// Rappel, vous pouvez utiliser:
	// - glm::translate
	// - glm::scale
	// - glm::rotate (attention a l'axe
	glm::mat4 t(1.0);
	// ex: une translation
	t = glm::translate(t, glm::vec3(v, 0.0, 0.0));
	return t;
}