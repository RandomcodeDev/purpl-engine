#include "purpl/x11/window.h"
using namespace purpl;

Display *display;

P_EXPORT purpl::x11_window::x11_window(int width, int height, char *title, ...)
{
	/* Open our display */ 
	display = XOpenDisplay(NULL);
}
