#include "purpl/inst.h"

P_EXPORT purpl::engine_inst::engine_inst(app_info *info, window *wnd,
					 bool write_hello)
{
	this->is_active = false;

	this->info = info;

#ifdef P_USE_VULKAN_GFX
	char vert_path[260];
	char frag_path[260];

	strcpy(vert_path, this->info->get_res_path());
	strcpy(frag_path, this->info->get_res_path());

	this->gfx = new gfx_inst(wnd, strcat(vert_path, "shaders/triangle.vert.spv"), strcat(frag_path, "shaders/triangle.frag.spv"));
#endif
	this->wnd = wnd;

	if (!this->gfx->init_success || this->wnd->should_close)
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

	if (this->wnd->should_close)
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
