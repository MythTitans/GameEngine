#version 330

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertUV;

out vec2 uv;

void main()
{
    uv = vertUV;
    gl_Position = vec4( vertPosition.xy, 0, 1 );
}