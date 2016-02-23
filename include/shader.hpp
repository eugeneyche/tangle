#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

GLuint LoadShader(GLenum type, const std::string& path);
GLuint LoadProgram(const std::vector<GLuint> shaders);
