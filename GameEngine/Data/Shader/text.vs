layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec2 vertUV;

out vec2 uv;

uniform vec2 position;
uniform vec2 size;

void main()
{
    uv = vertUV;
    gl_Position = vec4( vertPosition.xy * size + position, 0, 1 );
}