#version 330

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;
layout (location = 2) in vec3 vertNormal;

out vec2 texCoords;
out vec3 position;
out vec3 normal;

uniform mat4 viewProjection;

void main()
{
    texCoords = vertTexCoords;
    position = vertPosition;
    normal = vertNormal;

    gl_Position = viewProjection * vec4( vertPosition, 1 );
}