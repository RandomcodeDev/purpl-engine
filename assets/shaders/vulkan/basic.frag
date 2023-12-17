#version 460

layout (location = 0) in vec3 FragmentPosition;
layout (location = 1) in vec4 FragmentColour;
layout (location = 2) in vec2 FragmentTextureCoordinate;
layout (location = 3) in vec3 FragmentNormal;
layout (location = 4) in vec3 FragmentLightPosition;

layout (location = 0) out vec4 OutputColour;

layout (binding = 2) uniform sampler2D Sampler;

void
main()
{
    const vec3 LightColour = vec3(1.0, 1.0, 1.0);
    const float LightIntensity = 1.0f;
    const float AmbientLightStrength = 0.1;
    const vec3 AmbientLight = AmbientLightStrength * LightColour;

    vec3 Normal = normalize(FragmentNormal);
    vec3 LightDirection = normalize(FragmentLightPosition - FragmentPosition);
    float DiffuseImpact = max(dot(Normal, LightDirection), 0.0) * LightIntensity;
    vec3 Diffuse = DiffuseImpact * LightColour;

    OutputColour = vec4(AmbientLight + Diffuse, 1.0) *
        vec4(FragmentColour.rgb * texture(Sampler, FragmentTextureCoordinate).rgb, 1.0);
}
