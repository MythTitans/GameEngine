layout (location = 0) in vec3 vertPosition;
#ifdef UV
layout (location = 1) in vec2 vertTexCoords;
#endif

#ifdef UV
out vec2 texCoords;
#endif

uniform mat4 modelViewProjection;

void main()
{
#ifdef UV
    texCoords = vertTexCoords;
#endif
    gl_Position = modelViewProjection * vec4( vertPosition, 1.0 );
}