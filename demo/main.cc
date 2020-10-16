#include <stdlib.h>
#include <time.h>

#include <purpl/purpl.h>
using namespace purpl;

int main(int argc, char *argv[])
{
	srand((uint)time(NULL));

	int logindex = 0;

	engine_inst inst = engine_inst(new app_info(), NULL, new window(1024, 600, "A random number: %d", rand() % 10));

	inst.info->log->write(logindex, INFO, P_FILENAME, __LINE__, "A random number: %d", rand() % 10);

	while (inst.is_active) {
		inst.update();
	}

	return 0;
}
