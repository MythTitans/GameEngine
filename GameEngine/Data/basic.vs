#version 330

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;

out vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    texCoords = vertTexCoords;

    gl_Position = projection * view * vec4( vertPosition, 1 );
}