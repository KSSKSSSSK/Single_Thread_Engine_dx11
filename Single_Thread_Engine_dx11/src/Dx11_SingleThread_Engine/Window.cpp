#include "Window.h"
#include "common/Input.h"
#include "WindowUtil/WIndowsMessageMap.h"
#include <sstream>

Window* g_pWindow = nullptr;

Window::WindowClass::WindowClass() noexcept 
	: hInst(GetModuleHandle(nullptr))
{
}
Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

void Window::WindowClass::Init()
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hIconSm = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = L"123";
	wc.lpszClassName = GetName();

	RegisterClassEx(&wc);
}

const wchar_t* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}
HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return Get().hInst;
}

Window::WindowClass& Window::WindowClass::Get()
{
	static Window::WindowClass singleton;

	return singleton;
}

/////////////////////////////////////////////////////////////////////////

Window::Window(int16_t _width, int16_t _height, const std::wstring _name) noexcept
	:	width_(_width),
		height_(_height),
		name(_name),
		hWnd(0)
{
	g_pWindow = this;
}
Window::~Window()
{
	DestroyWindow(hWnd);
}

void Window::ActiveWindow()
{
	Window::WindowClass::Get().Init();

	// calculate window size based on desired client region size
	RECT wr;
	wr.left = 100;
	wr.right = width_ + wr.left;
	wr.top = 100;
	wr.bottom = height_ + wr.top;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	// create window & get hWnd
	hWnd = CreateWindow(
		WindowClass::GetName(), L"SingleThread_dx",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 640,
		nullptr, nullptr, WindowClass::GetInstance(), this);

	if (hWnd == nullptr) {

	}

	// Save window properties
	WindowStyle_ = GetWindowLong(hWnd, GWL_STYLE);
	GetWindowRect(hWnd, &WindowBounds_);
	GetClientRect(hWnd, &WindowClient_);

	// Show WIndow
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	pGfx_ = std::make_unique<Graphics>(hWnd, width_, height_);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (msg == WM_NCCREATE) {
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);


	}
	// use create parameter passed in from CreateWindow() to store window class pointer
	// if (msg == WM_NCCREATE) {
	// 	// extract ptr to window class from creation data
	// 	const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
	// 	Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
	// 	
	// 	// set WinAPI managed user data to store ptr to window class
	// 	SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
	// 	// set message proc to normal (non-setup) handler now that setup is finished
	// 	SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(Window::HandleMsgThunk));
	// 	// forward message to window instance handler
	// 	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	// }
	// if we get a message before the WM_NCCREATE message, handle with default handler
	return g_pWindow->HandleMsg(hWnd, msg, wParam, lParam);
}


LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	
	// forward message to window class handler
	return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	static WindowsMessageMap mm;
	OutputDebugString(mm(msg, lParam, wParam).c_str());

	switch (msg) {
		/*	백그라운드 모드에서 윈도우 밖에 마우스가 위치하게 된다면 자동적으로 마우스는 릴리즈.
			포커스가 돌아왔을 경우 장치를 다시 얻어와야됨.	*/
		case WM_ACTIVATE: {
			if (WA_INACTIVE == wParam)
				Active_ = false;
			else
				Active_ = true;

			// DXInput가 독점모드일 경우에는 Set exclusive mode access to the mouse based on active state
			Input::GetInstance().SetAcquire(Active_);
			return 0;
		} 
		case WM_KEYDOWN: {
			switch (wParam) {
			case '0': {
				ChangeFullScreenMode();
			} break;
			}
		} break;
		case WM_CLOSE: {
			PostQuitMessage(0);
			return 0;
		} break;
		case WM_SIZE: {
			if (wParam != SIZE_MINIMIZED && pGfx_.get()) {
				UINT width = LOWORD(lParam);
				UINT height = HIWORD(lParam);
				ResizeDevice(width, height);
			}
		} break;
		default: {
			return DefWindowProc(hWnd, msg, wParam, lParam);
		} break;
	}

	return 0;
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// return empty optional when not quitting app
	return {};
}


HRESULT Window::ResizeDevice(UINT _width, UINT _height)
{
	HRESULT hr;
	hr = pGfx_.get()->ResizeDevice(_width, _height);

	return hr;
}
bool Window::ChangeFullScreenMode()
{
	if (pGfx_.get()->ChangeFullScreenMode_Swapchain()) {
		ShowWindow(hWnd, SW_SHOW);
	}

	return true;
}


int16_t	Window::GetWidth()
{
	return width_;
}
int16_t	Window::GetHeight()
{
	return height_;
}

Graphics& Window::Get_gfx()
{
	return *pGfx_;
}
HWND Window::Get_Handle()
{
	return hWnd;
}
HINSTANCE Window::Get_Instance()
{
	return Window::WindowClass::GetInstance();
}
////////////////////////////////////////////////////////////

std::wstring Window::Exception::TranslateErrorCode(HRESULT hr) noexcept
{
	wchar_t* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return L"Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::wstring errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}
