#version 330 core

smooth in float v_alpha;

uniform bool is_taken;

out vec4 output_color;

void main()
{
    if (is_taken)
    {
        output_color = vec4(1, 0, 0, 1);
    }
    else
    {
        output_color = vec4(0, 0, 0, 1);
    }
}
