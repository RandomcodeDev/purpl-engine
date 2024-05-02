#include "vk.h"

static CONST VkVertexInputAttributeDescription MeshVertexAttributeDescriptions[] = {
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VERTEX, Position)},
    {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VERTEX, Colour)},
    {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VERTEX, TextureCoordinate)},
    {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VERTEX, Normal)},
};

VOID VlkCreatePipelineLayout(VOID)
{
    LogDebug("Creating pipeline layout");

    VkDescriptorSetLayout DescriptorLayouts[] = {VlkData.SceneDescriptorLayout, VlkData.ObjectDescriptorLayout};

    VkPipelineLayoutCreateInfo PipelineLayoutInformation = {0};
    PipelineLayoutInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInformation.setLayoutCount = PURPL_ARRAYSIZE(DescriptorLayouts);
    PipelineLayoutInformation.pSetLayouts = DescriptorLayouts;
    VULKAN_CHECK(vkCreatePipelineLayout(VlkData.Device, &PipelineLayoutInformation, VlkGetAllocationCallbacks(),
                                        &VlkData.PipelineLayout));
    VlkSetObjectName((UINT64)VlkData.PipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Pipeline layout");
}

VOID VlkCreateUniformBuffer(VOID)
{
    LogDebug("Creating uniform buffer");

    VlkAllocateBuffer((sizeof(RENDER_SCENE_UNIFORM) + sizeof(RENDER_OBJECT_UNIFORM)) * VULKAN_FRAME_COUNT,
                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &VlkData.UniformBuffer);
    VlkNameBuffer(&VlkData.UniformBuffer, "Uniform buffer");

    VULKAN_CHECK(vmaMapMemory(VlkData.Allocator, VlkData.UniformBuffer.Allocation, &VlkData.UniformBufferAddress));
}

RENDER_HANDLE VlkLoadShader(_In_z_ PCSTR Name)
{
    LogDebug("Creating pipeline for shader %s", Name);

    UINT64 VertexShaderSize = 0;
    PVOID VertexShader = FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "vulkan/%s.vs.spv", Name), 0, 0,
                                    &VertexShaderSize, 0);

    UINT64 FragmentShaderSize = 0;
    PVOID FragmentShader = FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "vulkan/%s.ps.spv", Name), 0, 0,
                                      &FragmentShaderSize, 0);

    if (!VertexShaderSize || !FragmentShaderSize)
    {
        CmnError("Vulkan shader for %s not found", Name);
    }

    VkShaderModuleCreateInfo VertexCreateInformation = {0};
    VertexCreateInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VertexCreateInformation.pCode = VertexShader;
    VertexCreateInformation.codeSize = VertexShaderSize;

    VkShaderModuleCreateInfo FragmentCreateInformation = {0};
    FragmentCreateInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    FragmentCreateInformation.pCode = FragmentShader;
    FragmentCreateInformation.codeSize = FragmentShaderSize;

    VkShaderModule VertexModule = VK_NULL_HANDLE;
    VULKAN_CHECK(
        vkCreateShaderModule(VlkData.Device, &VertexCreateInformation, VlkGetAllocationCallbacks(), &VertexModule));
    VkShaderModule FragmentModule = VK_NULL_HANDLE;
    VULKAN_CHECK(
        vkCreateShaderModule(VlkData.Device, &FragmentCreateInformation, VlkGetAllocationCallbacks(), &FragmentModule));

    CONST VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
    };

    VkPipelineDynamicStateCreateInfo DynamicState = {0};
    DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.pDynamicStates = DynamicStates;
    DynamicState.dynamicStateCount = PURPL_ARRAYSIZE(DynamicStates);

    VkPipelineShaderStageCreateInfo VertexStageCreateInformation = {0};
    VertexStageCreateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexStageCreateInformation.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexStageCreateInformation.module = VertexModule;
    VertexStageCreateInformation.pName = "VertexMain";

    VkPipelineShaderStageCreateInfo FragmentStageCreateInformation = {0};
    FragmentStageCreateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentStageCreateInformation.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentStageCreateInformation.module = FragmentModule;
    FragmentStageCreateInformation.pName = "PixelMain";

    VkPipelineShaderStageCreateInfo ShaderStages[] = {VertexStageCreateInformation, FragmentStageCreateInformation};

    VkVertexInputBindingDescription VertexBindingDescription = {0};
    VertexBindingDescription.binding = 0;
    VertexBindingDescription.stride = sizeof(VERTEX);
    VertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // TODO: when multiple types of "renderable" are added, make dynamic
    VkVertexInputAttributeDescription *VertexAttributeDescriptions = MeshVertexAttributeDescriptions;
    UINT32 VertexAttributeCount = (UINT32)PURPL_ARRAYSIZE(MeshVertexAttributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {0};
    InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyState.primitiveRestartEnable = FALSE;

    VkRect2D Scissor = {0};
    Scissor.extent.width = RdrGetWidth();
    Scissor.extent.height = RdrGetHeight();

    VkViewport Viewport = {0};
    Viewport.width = RdrGetWidth();
    Viewport.height = RdrGetHeight();
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkPipelineViewportStateCreateInfo ViewportState = {0};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissor;

    VkPipelineVertexInputStateCreateInfo VertexInputInformation = {0};
    VertexInputInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInformation.vertexBindingDescriptionCount = 1;
    VertexInputInformation.pVertexBindingDescriptions = &VertexBindingDescription;
    VertexInputInformation.vertexAttributeDescriptionCount = VertexAttributeCount;
    VertexInputInformation.pVertexAttributeDescriptions = VertexAttributeDescriptions;

    VkPipelineRasterizationStateCreateInfo RasterizationState = {0};
    RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationState.lineWidth = 1.0f;
    RasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
    RasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo MultisampleState = {0};
    MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleState.minSampleShading = 1.0f;

    VkPipelineColorBlendAttachmentState ColourBlendAttachment = {0};
    ColourBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColourBlendAttachment.blendEnable = TRUE;
    ColourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    VkPipelineColorBlendStateCreateInfo ColourBlendState = {0};
    ColourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColourBlendState.attachmentCount = 1;
    ColourBlendState.pAttachments = &ColourBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo DepthStencilState = {0};
    DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // DepthStencilState.flags =
    // VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;
    DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    DepthStencilState.depthWriteEnable = TRUE;
    DepthStencilState.depthTestEnable = TRUE;
    DepthStencilState.depthBoundsTestEnable = FALSE;
    DepthStencilState.stencilTestEnable = FALSE;

    VkGraphicsPipelineCreateInfo PipelineCreateInformation = {0};
    PipelineCreateInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInformation.stageCount = PURPL_ARRAYSIZE(ShaderStages);
    PipelineCreateInformation.pStages = ShaderStages;
    PipelineCreateInformation.pVertexInputState = &VertexInputInformation;
    PipelineCreateInformation.pInputAssemblyState = &InputAssemblyState;
    PipelineCreateInformation.pViewportState = &ViewportState;
    PipelineCreateInformation.pRasterizationState = &RasterizationState;
    PipelineCreateInformation.pMultisampleState = &MultisampleState;
    PipelineCreateInformation.pColorBlendState = &ColourBlendState;
    PipelineCreateInformation.pDynamicState = &DynamicState;
    PipelineCreateInformation.pDepthStencilState = &DepthStencilState;
    PipelineCreateInformation.layout = VlkData.PipelineLayout;
    PipelineCreateInformation.renderPass = VlkData.MainRenderPass;

    VkPipeline Pipeline = VK_NULL_HANDLE;
    VULKAN_CHECK(vkCreateGraphicsPipelines(VlkData.Device, VK_NULL_HANDLE, 1, &PipelineCreateInformation,
                                           VlkGetAllocationCallbacks(), &Pipeline));
    VlkSetObjectName((UINT64)Pipeline, VK_OBJECT_TYPE_PIPELINE, "%s pipeline", Name);

    vkDestroyShaderModule(VlkData.Device, VertexModule, VlkGetAllocationCallbacks());
    vkDestroyShaderModule(VlkData.Device, FragmentModule, VlkGetAllocationCallbacks());

    return (RENDER_HANDLE)Pipeline;
}

VOID VlkDestroyShader(_In_ RENDER_HANDLE Shader)
{
    vkDestroyPipeline(VlkData.Device, (VkPipeline)Shader, VlkGetAllocationCallbacks());
}
