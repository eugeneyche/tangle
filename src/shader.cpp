#include "shader.hpp"
#include "error.hpp"
#include <fstream>

GLuint LoadShader(GLenum type, const std::string& path)
{
    std::ifstream file (path);
    if (!file.is_open())
        FatalError("Failed to open shader '" + path + "'.");
    GLint file_size = 0;
    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<GLchar> src_buffer (file_size);
    file.read(src_buffer.data(), file_size);
    file.close();
    const GLchar* src_data = src_buffer.data();
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src_data, &file_size);
    glCompileShader(shader);
    GLint is_compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (!is_compiled)
    {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log_buffer (log_length);
        glGetShaderInfoLog(shader, log_length, &log_length, log_buffer.data());
        std::string log (log_buffer.data(), static_cast<size_t>(log_length));
        DumpLog(log);
        FatalError("Failed to compile shader '" + path + "'.");
    }
    return shader;
}

GLuint LoadProgram(std::vector<GLuint> shaders)
{
    GLuint program = glCreateProgram();
    for (GLuint shader : shaders)
    {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);
    GLint is_linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &is_linked);
    if (!is_linked)
    {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<GLchar> log_buffer (log_length);
        glGetProgramInfoLog(program, log_length, &log_length, log_buffer.data());
        std::string log (log_buffer.data(), static_cast<size_t>(log_length));
        DumpLog(log);
        FatalError("Failed to link program.");
    }
    for (GLuint shader : shaders)
    {
        glDetachShader(program, shader);
    }
    return program;
}
