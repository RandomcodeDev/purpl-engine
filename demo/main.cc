#include <stdlib.h>
#include <time.h>

#include <purpl/purpl.h>
using namespace purpl;

int main(int argc, char *argv[])
{
	srand((uint)time(NULL));

	int logindex = 0;

	window wnd = window(1024, 600, false, "A random number: %d", rand() % 10); /* Because we want a custom window title, we have to specify all the arguments */
	app_info info;

	info.log->write(logindex, INFO, P_FILENAME, __LINE__, "A random number: %d", rand() % 10);

	while (!wnd.should_close) {
		wnd.update(NULL, NULL, NULL);
	}

	return 0;
}
