#version 330

layout (location = 0) in vec3 vertPosition;

uniform mat4 viewProjection;
uniform vec3 position;
uniform float radius;

void main()
{
    gl_Position = viewProjection * vec4( radius * vertPosition + position, 1.0 );
}