layout (location = 0) in vec3 vertPosition;

uniform mat4 modelViewProjection;

void main()
{
    gl_Position = modelViewProjection * vec4( vertPosition, 1.0 );
}