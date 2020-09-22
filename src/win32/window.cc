#include <purpl/win32/window.h>
using namespace purpl;

WNDCLASSEXA wndclass = {};

const char wndclass_name[] = "Purpl Engine Window Class";

const HINSTANCE instance = GetModuleHandleA(NULL);

LRESULT P_EXPORT CALLBACK purpl::win32_window::wndproc(HWND wnd, UINT msg,
						 WPARAM wparam, LPARAM lparam)
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

P_EXPORT purpl::win32_window::win32_window(int width, int height, bool keep_console,
			       const char *title, ...)
{
	va_list args;

	wndclass.cbSize = sizeof(WNDCLASSEXA);
	wndclass.lpfnWndProc = wndproc;
	wndclass.hInstance = instance;
	wndclass.hIcon = (HICON)LoadImageA(instance, "purpl.ico", IMAGE_ICON, 512, 512, LR_LOADFROMFILE);
	wndclass.hCursor = LoadCursorA(0, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszClassName = wndclass_name;

	RegisterClassExA(&wndclass);

	this->width = width;
	this->height = height;

	this->win_queue = {};

	this->should_close = false;

	va_start(args, title);
	this->title = fmt_text_va(title, &args);
	va_end(args);

	this->handle = CreateWindowExA(0, wndclass_name, this->title,
				       WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
				       CW_USEDEFAULT, this->width, this->height,
				       NULL, NULL, instance, NULL);
	if (!this->handle) {
		MessageBoxA(NULL, "Failed to create window", "Error",
			    MB_OK | MB_ICONERROR);
		return;
	}

	if (!keep_console)
		FreeConsole();

	ShowWindow(this->handle, SW_NORMAL);
}

void P_EXPORT purpl::win32_window::update(int width, int height, const char *title,
				    ...)
{
	RECT winrect;

	if (width)
		SendMessageA(this->handle, WM_SIZE, NULL,
			     P_CONCAT(this->height, width, int, long));
	if (height)
		SendMessageA(this->handle, WM_SIZE, NULL,
			     P_CONCAT(height, this->width, int, long));
	if (title)
		SetWindowTextA(this->handle, title);

	GetClientRect(this->handle, &winrect);
	this->width = winrect.right;
	this->height = winrect.bottom;

	GetWindowTextA(this->handle, this->title, P_WIN32_WINDOW_TEXT_MAX - 1);

	if (!GetMessageA(&win_queue, NULL, NULL, NULL)) {
		should_close = true;
		return;
	}

	TranslateMessage(&win_queue);
	DispatchMessageA(&win_queue);
}

/* purpl::window::~window()
{
	This isn't needed yet
} */
