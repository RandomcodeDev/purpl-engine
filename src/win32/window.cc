#include <purpl/win32/window.h>
#include "resource.h"
using namespace purpl;

WNDCLASSEXW wndclass = {};

const wchar_t wndclass_name[] = L"Purpl Engine Window Class";

const HINSTANCE instance = GetModuleHandle(NULL);

LRESULT P_EXPORT CALLBACK purpl::window::wndproc(HWND wnd, UINT msg,
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

	return DefWindowProcW(wnd, msg, wparam, lparam);
}

P_EXPORT purpl::window::window(int width, int height, bool keep_console,
			       const wchar_t *title, ...)
{
	va_list args;

	wndclass.cbSize = sizeof(WNDCLASSEXW);
	wndclass.lpfnWndProc = wndproc;
	wndclass.hInstance = instance;
	wndclass.hIcon = (HICON)LoadImageW(instance, L"purpl.ico", IMAGE_ICON, 512, 512, LR_LOADFROMFILE);
	wndclass.hCursor = LoadCursorW(0, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndclass.lpszClassName = wndclass_name;

	RegisterClassExW(&wndclass);

	this->width = width;
	this->height = height;

	this->win_queue = {};

	this->should_close = false;

	va_start(args, title);

	memset(this->title, WINDOW_TEXT_MAX, sizeof(wchar_t));
	wcsncpy(this->title, fmt_text_va(title, &args), WINDOW_TEXT_MAX);

	va_end(args);

	this->handle = CreateWindowExW(0, wndclass_name, this->title,
				       WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
				       CW_USEDEFAULT, this->width, this->height,
				       NULL, NULL, instance, NULL);
	if (!this->handle) {
		MessageBoxW(NULL, L"Failed to create window", L"Error",
			    MB_OK | MB_ICONERROR);
		return;
	}

	if (!keep_console)
		FreeConsole();

	ShowWindow(this->handle, SW_NORMAL);
}

void P_EXPORT purpl::window::update(int width, int height, const wchar_t *title,
				    ...)
{
	RECT winrect;

	if (width)
		SendMessageW(this->handle, WM_SIZE, NULL,
			     P_CONCAT(this->height, width, int, long));
	if (height)
		SendMessageW(this->handle, WM_SIZE, NULL,
			     P_CONCAT(height, this->width, int, long));
	if (title)
		SetWindowTextW(this->handle, title);

	GetClientRect(this->handle, &winrect);
	this->width = winrect.right;
	this->height = winrect.bottom;

	GetWindowTextW(this->handle, this->title, WINDOW_TEXT_MAX - 1);

	if (!GetMessageW(&win_queue, NULL, NULL, NULL)) {
		should_close = true;
		return;
	}

	TranslateMessage(&win_queue);
	DispatchMessageW(&win_queue);
}

/* purpl::window::~window()
{
	This isn't needed yet
} */
