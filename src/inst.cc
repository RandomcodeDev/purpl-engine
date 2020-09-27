#include <purpl/inst.h>

P_EXPORT purpl::engine_inst::engine_inst(app_info *info, gfx_inst *gfx, window *wnd, bool write_hello)
{
	this->info = info;
	this->gfx = gfx;
	this->wnd = wnd;

	if (write_hello)
		this->info->log->write(this->info->logindex, INFO, P_FILENAME, __LINE__, "Instance created");
}
