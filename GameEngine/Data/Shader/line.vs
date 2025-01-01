#version 330

layout (location = 0) in vec3 vertPosition;

uniform mat4 viewProjection;

void main()
{
    gl_Position = viewProjection * vec4( vertPosition, 1.0 );
}