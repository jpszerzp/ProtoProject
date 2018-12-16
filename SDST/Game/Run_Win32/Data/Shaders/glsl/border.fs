#version 420 core

in vec4 passColor;
in vec2 passUV;

// Outputs
out vec4 outColor; 

void main()
{
    outColor = vec4(0.04, 0.28, 0.26, 1.0);
}