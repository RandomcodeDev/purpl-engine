#include "purpl/x11/window.h"
using namespace purpl;

bool should_close = true;
Window handle;

/* Get the default screen */
int screen = DefaultScreen(XOpenDisplay(NULL));

/* This function is specifically for checking if the window still exists */
int check_window_continued_existence(Display *displau, XErrorEvent *error_event)
{
	if (error_event->error_code == BadWindow &&
	    error_event->resourceid == handle)
		should_close = true;
}

P_EXPORT purpl::x11_window::x11_window(int width, int height, const char *title,
				       ...)
{
	va_list args;
	char *tmp;

	/* Similar to Vulkan init, we set this to false only when initialization succeeds */
	this->should_close = should_close;

	/* Format our title to get the final string */
	va_start(args, title);
	tmp = fmt_text_va(title, &args);
	va_end(args);

	/* Copy the title into the member */
	strcpy(this->title, tmp);

	/* Free the memory from fmt_text_va */
	free(tmp);

	/* Initialize threading so X11 doesn't freak out when we pass our window over to Vulkan */ 
	XInitThreads();

	/* Open the display */
	this->display = XOpenDisplay(NULL);
	if (!this->display) {
		fprintf(stderr, "Error opening display\n");
		return;
	}

	/* Create our window */
	this->handle = XCreateSimpleWindow(this->display,
					   RootWindow(this->display, screen),
					   10, 10, width, height, 1,
					   BlackPixel(this->display, screen),
					   WhitePixel(this->display, screen));
	if (!this->handle) {
		fprintf(stderr, "Failed to create window\n");
		return;
	}

	handle = this->handle;

	/* Ensure we have a decent chance of knowing the window is dead */
	XSetErrorHandler(check_window_continued_existence);

	/* Set the window's title */
	XStoreName(this->display, this->handle, this->title);

	/* Choose the type of events we want to recieve */
	XSelectInput(this->display, this->handle, ExposureMask | KeyPressMask);

	/* Map our window to the screen */
	XMapWindow(this->display, this->handle);

	should_close = false;
	this->should_close = should_close;
}

void P_EXPORT purpl::x11_window::update(int width, int height,
					const char *title, ...)
{
	va_list args;
	char *tmp;
	XTextProperty text_property;
	XWindowAttributes window_attrs;

	/* Set the width/height/title of the window */
	if (width)
		XResizeWindow(this->display, this->handle, width, this->height);
	if (height)
		XResizeWindow(this->display, this->handle, this->width, height);
	if (title) {
		/* Format our title string */
		va_start(args, title);
		tmp = fmt_text_va(title, &args);
		va_end(args);

		if (strcmp(tmp, "") ==
		    0) { /* This means that fmt_text_va got an invalid string */
			XStoreName(this->display, this->handle, this->title);
		} else {
			strcpy(this->title, tmp);
			XStoreName(this->display, this->handle, this->title);
			free(tmp);
		}
	}

	/* Retrieve the various information about the window that we need */
	XGetWindowAttributes(this->display, this->handle, &window_attrs);
	this->width = window_attrs.width;
	this->height = window_attrs.height;
	XGetWMName(this->display, this->handle, &text_property);

	strcpy(this->title, (char *)text_property.value);

	/* Process events */
	XNextEvent(this->display, &this->win_queue);
	switch (this->win_queue.type) {
	default:
		break;
	}

	this->should_close = should_close;
	handle = this->handle;
}

P_EXPORT purpl::x11_window::~x11_window(void)
{
	this->should_close = true;
	XCloseDisplay(this->display);
}
