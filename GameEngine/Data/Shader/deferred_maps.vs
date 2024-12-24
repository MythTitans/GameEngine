#version 330

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;
layout (location = 2) in vec3 vertNormal;

out vec2 texCoords;
out vec3 position;
out vec3 normal;

uniform mat4 modelViewProjection;
uniform mat4 modelInverseTranspose;

void main()
{
    texCoords = vertTexCoords;
    normal = normalize( modelInverseTranspose * vec4( vertNormal, 0.0 ) ).xyz;

    gl_Position = modelViewProjection * vec4( vertPosition, 1.0 );
}