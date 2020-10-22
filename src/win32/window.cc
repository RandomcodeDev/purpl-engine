#include "purpl/win32/window.h"
using namespace purpl;

WNDCLASSEXA wndclass = {};

const char wndclass_name[] = "Purpl Engine Window Class";

const HINSTANCE instance = GetModuleHandleA(NULL);

LRESULT P_EXPORT CALLBACK purpl::win32_window::wndproc(HWND wnd, UINT msg,
						       WPARAM wparam,
						       LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		PAINTSTRUCT paint;
		HDC hdc = BeginPaint(wnd, &paint);

		//FillRect(hdc, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(wnd, &paint);
	}

	return DefWindowProcA(wnd, msg, wparam, lparam);
}

P_EXPORT purpl::win32_window::win32_window(int width, int height, const char *title,
					   ...)
{
	va_list args;

	/* Set up our window class */
	wndclass.cbSize = sizeof(WNDCLASSEXA);
	wndclass.lpfnWndProc = wndproc;
	wndclass.hInstance = instance;
	wndclass.hIcon = (HICON)LoadImageA(instance, "purpl.ico", IMAGE_ICON,
					   512, 512, LR_LOADFROMFILE);
	wndclass.hCursor = LoadCursorA(0, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszClassName = wndclass_name;

	/* Register the window class */
	RegisterClassExA(&wndclass);

	/* Copy the width and height parameters into the appropriate members */
	this->width = width;
	this->height = height;

	/* Initialize the message queue */
	this->win_queue = {};

	/* Make sure the window doesn't just close */
	this->should_close = false;

	/* Format our title to get the final string */
	va_start(args, title);
	this->title = fmt_text_va(title, &args);
	va_end(args);

	/* Create a window */
	this->handle = CreateWindowExA(0, wndclass_name, this->title,
				       WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
				       CW_USEDEFAULT, this->width, this->height,
				       NULL, NULL, instance, NULL);
	if (!this->handle) {
		MessageBoxA(NULL, "Failed to create window", "Error",
			    MB_OK | MB_ICONERROR);
		return;
	}

#ifdef NDEBUG
	FreeConsole(); /* This seems like the best way of handling this, but who knows? */
#endif

	/* Show our window */
	ShowWindow(this->handle, SW_NORMAL);
}

void P_EXPORT purpl::win32_window::update(int width, int height,
					  const char *title, ...)
{
	RECT winrect;
	va_list args;

	char *tmp;

	/* 
	 * Handle resizing/title change. lparam gets height in
	 * the higher half and width in the lower, so we use P_CONCAT.
	 */
	SendMessageA(this->handle, WM_SIZE, NULL,
			     P_CONCAT((height) ? height : this->height, (width) ? width : this->width, int, long));
	if (title) {
		/* Format our title string */
		va_start(args, title);
		tmp = fmt_text_va(title, &args);
		va_end(args);

		if (strcmp(tmp, "") == 0) {
			SetWindowTextA(this->handle, this->title);
		} else {
			this->title = tmp;
			SetWindowTextA(this->handle, tmp);
		}
	}

	/* Now we calculate the width and height of the client area of our window */
	GetClientRect(this->handle, &winrect);
	this->width = winrect.right - winrect.left;
	this->height = winrect.bottom - winrect.top;

	/* And retrieve the title */
	GetWindowTextA(this->handle, this->title, P_WIN32_WINDOW_TEXT_MAX - 1);

	/* Now check if we need to close the window (if we got a WM_QUIT) */
	if (!GetMessageA(&win_queue, NULL, NULL, NULL)) {
		should_close = true;
		return;
	}

	/* And finally, update the message queue */
	TranslateMessage(&win_queue);
	DispatchMessageA(&win_queue);
}

/* purpl::window::~window()
{
	This isn't needed yet
} */
