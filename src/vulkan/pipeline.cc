#include "purpl/vulkan/pipeline.h"
using namespace purpl;

VkRenderPass purpl::create_render_pass(VkDevice device, VkFormat image_format)
{
	VkRenderPass render_pass;

	VkAttachmentDescription color_attachment = {};
	color_attachment.format = image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = NULL;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = NULL;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;
	render_pass_info.dependencyCount = 1;
	render_pass_info.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass) !=
	    VK_SUCCESS)
		return NULL;

	return render_pass;
}

VkPipeline purpl::create_graphics_pipeline(
	VkDevice device, VkExtent2D viewport_extent, VkRenderPass render_pass,
	const char *vert_shader_path, const char *frag_shader_path,
	VkPipelineLayout *pipeline_layout)
{
	VkPipeline pipeline;
	VkPipelineShaderStageCreateInfo shader_stages[2];
	VkShaderModule vert_shader;
	VkShaderModule frag_shader;
	char *vert_shader_bytecode;
	char *frag_shader_bytecode;
	size_t vert_shader_len;
	size_t frag_shader_len;
	size_t i;

	VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT,
					    VK_DYNAMIC_STATE_LINE_WIDTH };

	/* Check the parameters we've gotten */
	if (!device || !render_pass || !vert_shader_path || !frag_shader_path ||
	    !verts || !vert_count || !pipeline_layout) {
		errno = EINVAL;
		return NULL;
	}

#ifndef NDEBUG
	printf("Using vertex shader \'%s\' and fragment shader \'%s\'\n",
	       vert_shader_path, frag_shader_path);
#endif

	/* Read our shaders */
	vert_shader_bytecode =
		read_file(vert_shader_path, false, &vert_shader_len);
	frag_shader_bytecode =
		read_file(frag_shader_path, false, &frag_shader_len);

	/* Check that we succeeded in reading the shaders */
	if (!vert_shader_bytecode || !frag_shader_bytecode ||
	    !vert_shader_len || !frag_shader_len)
		return NULL;

	/* Give our shaders to Vulkan so we can use them later */
	vert_shader = create_shader_module(device, vert_shader_bytecode,
					   vert_shader_len);
	frag_shader = create_shader_module(device, frag_shader_bytecode,
					   frag_shader_len);

	/* Check the shader modules we just created */
	if (!vert_shader || !frag_shader)
		return NULL;

	/* Free our shader file contents, we don't need them anymore */
	free(vert_shader_bytecode);
	free(frag_shader_bytecode);

	/* Create our vertex shader stage */
	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader;
	vert_shader_stage_info.pName = "main";

	/* Now for the fragment stage */
	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader;
	frag_shader_stage_info.pName = "main";

	/* Now store our creation info structs in an array */
	shader_stages[0] = vert_shader_stage_info;
	shader_stages[1] = frag_shader_stage_info;

	/* Now set up the vertex input */
	VkVertexInputBindingDescription vert_binding_desc =
		get_vert_input_binding_desc();
	VkVertexInputAttributeDescription *vert_attrib_descs =
		get_vert_input_attrib_descs();

	VkPipelineVertexInputStateCreateInfo vert_input_info = {};
	vert_input_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vert_input_info.vertexBindingDescriptionCount = 1;
	vert_input_info.pVertexBindingDescriptions = &vert_binding_desc;
	vert_input_info.vertexAttributeDescriptionCount = 2;
	vert_input_info.pVertexAttributeDescriptions = vert_attrib_descs;

	/* Tell Vulkan we're drawing triangles */
	VkPipelineInputAssemblyStateCreateInfo input_asm_info = {};
	input_asm_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_asm_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_asm_info.primitiveRestartEnable = false;

	/* Now we define our viewport configuration */
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)viewport_extent.width;
	viewport.height = (float)viewport_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	/* Set the scissor rectangle to be the same as the viewport */
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = viewport_extent;

	/* And fill out a viewport state config structure */
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	/* Configure the rasterizer */
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = false;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/* Configure multisampling */
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = false;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = NULL;
	multisampling.alphaToCoverageEnable = false;
	multisampling.alphaToOneEnable = false;

	/* Color blending config */
	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = true;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = false;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	memset(color_blending.blendConstants, 0, 4);

	/* We sure as hell want to be able to resize our viewport */
	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	VkPipelineLayoutCreateInfo layout_create_info = {};
	layout_create_info.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_create_info.setLayoutCount = 0;
	layout_create_info.pSetLayouts = NULL;
	layout_create_info.pushConstantRangeCount = 0;
	layout_create_info.pPushConstantRanges = NULL;

	if (vkCreatePipelineLayout(device, &layout_create_info, NULL,
				   pipeline_layout) != VK_SUCCESS)
		return NULL;

	/* Create our graphics pipeline */
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vert_input_info;
	pipeline_info.pInputAssemblyState = &input_asm_info;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = NULL;
	pipeline_info.pColorBlendState = &color_blending;
	/* pipeline_info.pDynamicState = &dynamic_state; /* Maybe later */
	pipeline_info.layout = *pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = NULL;
	pipeline_info.basePipelineIndex = -1;

	/* Create our graphics pipeline */
	if (vkCreateGraphicsPipelines(device, NULL, 1, &pipeline_info, NULL,
				      &pipeline) != VK_SUCCESS)
		return NULL;

	/* Clean up our shader modules */
	vkDestroyShaderModule(device, vert_shader, NULL);
	vkDestroyShaderModule(device, frag_shader, NULL);

	return pipeline;
}
