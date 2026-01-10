layout (location = 0) in vec3 vertPosition;
layout (location = 4) in uvec4 vertBones;
layout (location = 5) in vec4 vertWeights;

layout(std140, binding = 0) uniform SkinningDataBlock
{
    mat4 boneMatrices[ 128 ];
};

uniform mat4 modelViewProjection;
uniform bool useSkinning;

void main()
{
    if( useSkinning )
    {
        vec4 transformedPosition = vec4( 0.0, 0.0, 0.0, 0.0 );
        for( int i = 0; i < 4; ++i )
            transformedPosition += vertWeights[ i ] * boneMatrices[ vertBones[ i ] ] * vec4( vertPosition, 1.0 );

        gl_Position = modelViewProjection * transformedPosition;
    }
    else
    {
        gl_Position = modelViewProjection * vec4( vertPosition, 1.0 );
    }
}