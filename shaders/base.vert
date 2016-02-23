#version 330 core

layout(location = 0) in vec2 position;

uniform mat4 world_view;

void main()
{
    gl_Position = world_view * vec4(position, 0, 1);
}
