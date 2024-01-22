#version 330

// Input
in vec3 frag_color;
in float hp;  // Received HP value from vertex shader

// Output
layout(location = 0) out vec4 out_color;

float modify_color_based_on_hp(float hp)
{
    float color;
    // Modify color based on HP
    if (hp > 2.0)
    {
        color = 1.0;
    }
    else if (hp > 1.0)
    {
        color = 0.75;
    }
    else if (hp > 0.5)
    {
        color = 0.5;
    }
    else
    {
        color = 0.25;
    }
    return color;
}

void main()
{
    vec3 modified_color = frag_color * modify_color_based_on_hp(hp);
    // Write pixel out color
    out_color = vec4(modified_color, 1);
}
