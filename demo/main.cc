#include <stdlib.h>
#include <time.h>

#include <purpl/purpl.h>
using namespace purpl;

int main(int argc, char *argv[])
{
	srand((uint)time(NULL));

	int logindex = 0;

	engine_inst inst = engine_inst(new app_info(), new gfx_inst(), new window(1024, 600, true, "A random number: %d", rand() % 10));

	inst.info->log->write(logindex, INFO, P_FILENAME, __LINE__, "A random number: %d", rand() % 10);



	while (!inst.wnd->should_close) {
		inst.wnd->update(NULL, NULL, NULL);
	}

	return 0;
}
