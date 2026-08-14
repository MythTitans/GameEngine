layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba8, binding = 0) uniform image2D testTexture;
layout(std430, binding = 0) buffer Test
{
    vec3 test;
};

void main()
{
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(testTexture);

    vec2 uv = vec2(gl_LocalInvocationID.xy) / vec2(8);

    imageStore(testTexture, pixel, vec4(uv * test.rg, 0.0, 1.0));
}