layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;
layout (location = 2) in vec3 vertNormal;
layout (location = 3) in vec3 vertTangent;

out vec2 texCoords;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;

uniform mat4 modelViewProjection;
uniform mat4 modelInverseTranspose;

void main()
{
    texCoords = vertTexCoords;
    normal = ( modelInverseTranspose * vec4( vertNormal, 0.0 ) ).xyz;
    tangent = ( modelInverseTranspose * vec4( vertTangent, 0.0 ) ).xyz;
    bitangent = cross( normal, tangent );

    gl_Position = modelViewProjection * vec4( vertPosition, 1.0 );
}