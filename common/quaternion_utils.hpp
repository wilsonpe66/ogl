#pragma once

#include "gl_base.h"
#include <glm/gtc/quaternion.hpp>

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest);

glm::quat LookAt(glm::vec3 direction, glm::vec3 desiredUp);

glm::quat RotateTowards(glm::quat q1, glm::quat q2, float maxAngle);
