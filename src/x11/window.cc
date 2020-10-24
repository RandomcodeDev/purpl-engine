#include "purpl/x11/window.h"
using namespace purpl;

/* Get the default screen */
int screen = DefaultScreen(XOpenDisplay(NULL));

P_EXPORT purpl::x11_window::x11_window(int width, int height, const char *title, ...)
{
	va_list args;
	char *tmp;

	/* Similar to Vulkan init, we set this to false only when initialization succeeds */
	this->should_close = true;

	/* Format our title to get the final string */
	va_start(args, title);
	tmp = fmt_text_va(title, &args);
	va_end(args);

	/* Copy the title into the member */
	strcpy(this->title, tmp);

	/* Free the memory from fmt_text_va */
	free(tmp);

	/* Open the display */
	this->display = XOpenDisplay(NULL);
	if (!this->display) {
		fprintf(stderr, "Error opening display\n");
		return;
	}

	/* Create our window */
	this->handle = XCreateSimpleWindow(this->display, RootWindow(this->display, screen), 10, 10, width, height, 1, BlackPixel(this->display, screen), WhitePixel(this->display, screen));
	if (!this->handle) {
		fprintf(stderr, "Failed to create window\n");
		return;
	}

	/* Set the window's title */
	XStoreName(this->display, this->handle, this->title);

	/* Choose the type of events we want to recieve */
	XSelectInput(this->display, this->handle, ExposureMask | KeyPressMask);

	/* Map our window to the screen */
	XMapWindow(this->display, this->handle);
}

void P_EXPORT purpl::x11_window::update(int width, int height, const char *title, ...)
{
	va_list args;
	char *tmp;
	XTextProperty text_property;

	/* Set the width/height/title of the window */
	XResizeWindow(this->display, this->handle, (width) ? width : this->width, (height) ? width : this->width);
	if (title) {
		/* Format our title string */
		va_start(args, title);
		tmp = fmt_text_va(title, &args);
		va_end(args);

		if (strcmp(tmp, "") == 0) { /* This means that fmt_text_va got an invalid string */
			XStoreName(this->display, this->handle, this->title);
		} else {
			strcpy(this->title, tmp);
			XStoreName(this->display, this->handle, this->title);
			free(tmp);
		}
	}

	/* Retrieve the various information about the window that we need */
	XGetGeometry(this->display, this->handle, NULL, NULL, NULL, this->width, this->height, NULL, NULL);
	XGetWMName(this->display, this->handle, &text_property);

	strcpy(this->title, (char *)text_property.value);

	/* Process events */
	XNextEvent(this->display, &this->win_queue);
	switch (this->win_queue.type) {
	default:
		break;
	}
}

P_EXPORT purpl::x11_window::~x11_window(void)
{
	XCloseDisplay(this->display);
}
