#version 330

layout (location = 0) in vec3 vertPosition;

out vec3 texCoords;

uniform mat4 viewProjection;

void main()
{
    texCoords = vertPosition;
    gl_Position = viewProjection * vec4( vertPosition, 1.0 );
}