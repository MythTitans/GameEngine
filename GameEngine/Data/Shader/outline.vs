layout (location = 0) in vec3 vertPosition;
layout (location = 2) in vec3 vertNormal;
layout (location = 4) in uvec4 vertBones;
layout (location = 5) in vec4 vertWeights;

layout(std140, binding = 0) uniform SkinningDataBlock
{
    mat4 boneMatrices[ 128 ];
};

uniform mat4 model;
uniform mat4 modelViewProjection;
uniform vec3 cameraPosition;
uniform float displacement;

void main()
{
    vec4 transformedPosition = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec3 transformedNormal = vec3( 0.0, 0.0, 0.0 );
    for( int i = 0; i < 4; ++i )
    {
        transformedPosition += vertWeights[ i ] * boneMatrices[ vertBones[ i ] ] * vec4( vertPosition, 1.0 );
        // TODO #eric maybe add support for non-uniform scaling ?
        transformedNormal += vertWeights[ i ] * mat3( boneMatrices[ vertBones[ i ] ] ) * vertNormal;
    }

    transformedNormal = normalize( transformedNormal );

    float factor = length( (model * transformedPosition).xyz - cameraPosition );
    gl_Position = modelViewProjection * vec4( transformedPosition.xyz + factor * displacement * normalize( transformedNormal ), 1.0 );
}