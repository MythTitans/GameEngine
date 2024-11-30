#version 330

const int MAX_DIRECTIONAL_LIGHTS = 4;

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertTexCoords;
layout (location = 2) in vec3 vertNormal;
layout (location = 3) in vec3 vertBinormal;
layout (location = 4) in vec3 vertTangent;

out vec3 position;
out vec2 texCoords;
out vec3 normal;
out vec3 binormal;
out vec3 tangent;
out vec4 directionalLightSpacePos[4];

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 directionalLightTransform[4];
uniform int directionalLightCount;

void main()
{
    position = (model * vec4(vertPosition, 1)).xyz;
    texCoords = vertTexCoords;

    mat3 modelInverseTranspose = mat3(transpose(inverse(model)));
    normal = modelInverseTranspose * vertNormal;
    binormal = mat3(model) * vertBinormal;
    tangent = mat3(model) * vertTangent;

    for (int i = 0; i < directionalLightCount; ++i)
    {
        directionalLightSpacePos[i] = directionalLightTransform[i] * model * vec4(vertPosition, 1);
    }

    gl_Position = projection * view * model * vec4(vertPosition, 1);
}