layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;
layout (location = 2) in vec3 vertNormal;
layout (location = 3) in vec3 vertTangent;
layout (location = 4) in uvec4 vertBones;
layout (location = 5) in vec4 vertWeights;

out vec2 texCoord;
out vec3 position;
out vec3 normal;
out vec3 tangent;
out vec3 bitangent;

uniform mat4 modelViewProjection;
uniform mat4 model;
uniform mat4 modelInverseTranspose;
uniform mat4 boneMatrices[ 128 ];
uniform bool useSkinning;

void main()
{
    texCoord = vertTexCoords;

    vec4 transformedPosition = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec3 transformedNormal = vec3( 0.0, 0.0, 0.0 );
    vec3 transformedTangent = vec3( 0.0, 0.0, 0.0 );

    if( useSkinning )
    {
        for( int i = 0; i < 4; ++i )
        {
            transformedPosition += vertWeights[ i ] * boneMatrices[ vertBones[ i ] ] * vec4( vertPosition, 1.0 );
            // TODO #eric maybe add support for non-uniform scaling ?
            transformedNormal += vertWeights[ i ] * mat3( boneMatrices[ vertBones[ i ] ] ) * vertNormal;
            transformedTangent += vertWeights[ i ] * mat3( boneMatrices[ vertBones[ i ] ] ) * vertTangent;
        }

        transformedNormal = normalize( transformedNormal );
        transformedTangent = normalize( transformedTangent );
    }
    else
    {
        transformedPosition = vec4( vertPosition, 1.0 );
        transformedNormal = vertNormal;
        transformedTangent = vertTangent;
    }

    position = ( model * transformedPosition ).xyz;

    normal = ( modelInverseTranspose * vec4( transformedNormal, 0.0 ) ).xyz;
    tangent = ( modelInverseTranspose * vec4( transformedTangent, 0.0 ) ).xyz;
    bitangent = cross( normal, tangent );

    gl_Position = modelViewProjection * transformedPosition;
}