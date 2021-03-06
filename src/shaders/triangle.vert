#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_colour;

layout(location = 0) out vec4 out_colour;

void main()
{
	gl_Position = in_pos;
	out_colour = in_colour;
}
