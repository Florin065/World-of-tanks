#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_color;
layout(location = 3) in vec3 v_normal;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 Color;
uniform float v_hp;

// Output
out vec3 frag_normal;
out vec3 frag_color;
out float hp;

void main()
{
    // Send output to fragment shader
    frag_color    = Color;
    frag_normal   = v_normal;
    hp            = v_hp;
    
    // Deform the tank shape based on HP
    float scale_factor = 3.0;
    if (hp >= 0.0) {
        scale_factor = 3.0 - hp;
    }
    vec3 deformed_position = v_position + vec3(0.1 * sin(scale_factor), -0.05, 0.1 * cos(scale_factor)) * scale_factor;

    // Compute gl_Position
    gl_Position = Projection * View * Model * vec4(deformed_position, 1.0);
}
