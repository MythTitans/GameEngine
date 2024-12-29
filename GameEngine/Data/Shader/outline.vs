#version 330

layout (location = 0) in vec3 vertPosition;
layout (location = 2) in vec3 vertNormal;

uniform mat4 modelViewProjection;
uniform vec3 cameraPosition;
uniform float displacement;

void main()
{
    float factor = length( vertPosition - cameraPosition );
    gl_Position = modelViewProjection * vec4( vertPosition + factor * displacement * normalize( vertNormal ), 1.0 );
}