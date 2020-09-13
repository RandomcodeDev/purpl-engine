#include <purpl/win32/window.h>
using namespace purpl;

WNDCLASSW wndclass = {};

const wchar_t wndclass_name[] = L"Purpl Engine Window Class";

const HINSTANCE instance = GetModuleHandle(NULL);

LRESULT __declspec(dllexport) CALLBACK purpl::window::wndproc(HWND wnd, UINT msg,
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

		FillRect(hdc, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(wnd, &paint);
	}

	return DefWindowProcW(wnd, msg, wparam, lparam);
}

__declspec(dllexport) purpl::window::window(int width, int height,
					    const wchar_t *title, bool keep_console)
{
	wndclass.lpfnWndProc = wndproc;
	wndclass.hInstance = instance;
	wndclass.lpszClassName = wndclass_name;

	RegisterClassW(&wndclass);

	this->width = width;
	this->height = height;

	this->win_queue = {};
	
	this->should_close = false;

	this->handle = CreateWindowExW(0, wndclass_name, title,
				      WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
				      CW_USEDEFAULT, this->width, this->height,
				      NULL, NULL, instance, NULL);
	if (!this->handle) {
		fprintf(stderr, "Failed to open window\n");
		return;
	}

	if (!keep_console)
		FreeConsole();

	ShowWindow(this->handle, SW_NORMAL);

	memset(this->title, WINDOW_TEXT_MAX, sizeof(wchar_t));
	wcsncpy(this->title, title, WINDOW_TEXT_MAX);
}

void __declspec(dllexport) purpl::window::update(int width, int height,
						 const wchar_t *title)
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
