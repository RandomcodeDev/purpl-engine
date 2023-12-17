#version 460

layout (binding = 0) uniform Ubo {
    mat4 View;
    mat4 Projection;
} UniformBufferObject;

layout (push_constant) uniform ObjectUbo {
    mat4 Model;
} ObjectUniformBufferObject;

layout (location = 0) in vec3 InputPosition;
layout (location = 1) in vec4 InputColour;
layout (location = 2) in vec2 InputTextureCoordinate;
layout (location = 3) in vec3 InputNormal;

layout (location = 0) out vec3 FragmentPosition;
layout (location = 1) out vec4 FragmentColour;
layout (location = 2) out vec2 FragmentTextureCoordinate;
layout (location = 3) out vec3 FragmentNormal;
layout (location = 4) out vec3 FragmentLightPosition;

void
main()
{
    const vec3 LightPosition = vec3(0.0, 0.0, 8.0);

    mat4 Mvp = UniformBufferObject.Projection * UniformBufferObject.View * ObjectUniformBufferObject.Model;
    gl_Position = Mvp * vec4(InputPosition, 1);
    FragmentPosition = (ObjectUniformBufferObject.Model * vec4(InputPosition, 1)).xyz;
    FragmentColour = InputColour;
    FragmentTextureCoordinate = InputTextureCoordinate;
    FragmentNormal = mat3(transpose(inverse(ObjectUniformBufferObject.Model))) * InputNormal;
    FragmentLightPosition = LightPosition;
}
