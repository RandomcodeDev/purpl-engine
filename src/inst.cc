#include "purpl/inst.h"

P_EXPORT purpl::engine_inst::engine_inst(app_info *info, window *wnd,
					 bool write_hello)
{
	this->is_active = false;

	this->info = info;
	this->gfx = new gfx_inst(wnd);
	this->wnd = wnd;

	if (!this->gfx->init_success)
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
}

P_EXPORT purpl::engine_inst::~engine_inst(void)
{
	this->is_active = false;

	delete this->gfx;
	delete this->info;
	delete this->wnd;
}
