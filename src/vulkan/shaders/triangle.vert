#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 frag_color;

vec4 verts[3] = vec4[](vec4(0.0, -0.5, 0.0, 1.0), vec4(0.5, 0.5, 0.0, 1.0),
		       vec4(-0.5, 0.5, 0.0, 1.0));

vec4 colors[3] = vec4[](vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0),
			vec4(0.0, 0.0, 1.0, 1.0));

void main()
{
	gl_Position = verts[gl_VertexIndex];
	frag_color = colors[gl_VertexIndex];
}
