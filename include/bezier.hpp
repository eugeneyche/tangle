#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>

void GenBezierCurve(
        const std::vector<glm::vec2>& control_points, 
        std::vector<glm::vec2>& r_path,
        std::vector<GLfloat>& r_alpha);
