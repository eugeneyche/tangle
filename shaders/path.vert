#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in float alpha;

uniform mat4 world_view;

smooth out float v_alpha;

void main()
{
    gl_Position = world_view * vec4(position, 0, 1);
    v_alpha = alpha;
}
