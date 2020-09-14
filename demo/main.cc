#include <stdlib.h>
#include <time.h>

#include <purpl/purpl.h>
using namespace purpl;

int main(int argc, char *argv[])
{
	srand((uint)time(NULL));

	window wnd(1024, 600, false, L"A random number: %d", rand() % 10);

	while (!wnd.should_close) {
		wnd.update(NULL, NULL, NULL);
	}

	return 0;
}
