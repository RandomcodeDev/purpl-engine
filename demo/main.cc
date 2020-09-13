#include <purpl/purpl.h>
using namespace purpl;

int main(int argc, char *argv[])
{
	window wnd(1024, 768, L"asdf", false);

	while (!wnd.should_close) {
		wnd.update(NULL, NULL, NULL);
	}

	return 0;
}
