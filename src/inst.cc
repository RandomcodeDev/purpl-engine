#include "purpl/inst.h"
using namespace purpl;

char purpl::vert_path[260];
char purpl::frag_path[260];

P_EXPORT purpl::engine_inst::engine_inst(app_info *info, window *wnd,
					 bool write_hello,
					 const char *vert_shader_name,
					 const char *frag_shader_name)
{
	this->is_active = false;

	this->info = info;

	/* Shaders */
	strcpy(vert_path, this->info->get_res_path());
	strcpy(frag_path, this->info->get_res_path());

	strcat(vert_path, "shaders/");
	strcat(frag_path, "shaders/");

	strcat(vert_path, vert_shader_name);
	strcat(frag_path, frag_shader_name);

#ifdef P_USE_VULKAN_GFX
	strcat(vert_path, ".spv");
	strcat(frag_path, ".spv");
#endif

	this->gfx = new gfx_inst(wnd);
	this->wnd = wnd;

	if (!this->gfx->is_active || this->wnd->should_close)
		return;

	if (write_hello)
		this->info->log->write(this->info->logindex, INFO, P_FILENAME,
				       __LINE__, "Instance created");

	this->is_active = true;
}

P_EXPORT void purpl::engine_inst::update(int width, int height,
					 const char *title, ...)
{
	va_list args;

	/* Call the window's update function, more to come later but that's it for now */
	va_start(args, title);
	this->wnd->update(width, height, fmt_text_va(title, &args));
	va_end(args);

	this->gfx->update(this->wnd);

	if (this->wnd->should_close || !this->gfx->is_active)
		this->is_active = false;
	else
		this->is_active = true;
}

P_EXPORT purpl::engine_inst::~engine_inst(void)
{
	this->is_active = false;

	delete this->gfx;
	delete this->info;
	delete this->wnd;
}
