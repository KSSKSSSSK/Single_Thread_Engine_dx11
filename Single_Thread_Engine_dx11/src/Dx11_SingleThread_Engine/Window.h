#pragma once
#include "KSWin.h"
#include "WindowUtil/KSException.h"
#include "Graphics/Graphics.h"
#include <memory>
#include <optional>

class Window
{
public:
	class Exception : public KSException
	{
		using KSException::KSException;
	public:
		static std::wstring TranslateErrorCode(HRESULT hr) noexcept;
	};
	class HrException : public Exception
	{
	public:
		HrException(int _line, const char* _file, HRESULT _hr) noexcept;
	};
private:
	// singleton manages registeation/cleanup of window class
	class WindowClass
	{
	public:
		void Init();
	public:
		static const wchar_t*	GetName()		noexcept;
		static HINSTANCE		GetInstance()	noexcept;
		static WindowClass&		Get();
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const WindowClass&) = delete;
		WindowClass& operator= (const WindowClass&) = delete;
		static constexpr const wchar_t* wndClassName = L"Dx11 Engine Window";
		HINSTANCE hInst;
	};
private:
	int16_t width_;
	int16_t height_;
	DWORD	WindowStyle_;	// 윈도우 스타일
	RECT	WindowBounds_;   // 윈도우 영역
	RECT	WindowClient_;   // 클라이언트 영역	
	bool	Active_;			// 윈도우의 활성화 여부플 판단한다.
	HWND hWnd;
	std::wstring name;
	std::unique_ptr<Graphics> pGfx_;
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT CALLBACK HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
	void ActiveWindow();

	HRESULT ResizeDevice(UINT _width, UINT _height);
	bool	ChangeFullScreenMode();
public:
	static std::optional<int> ProcessMessages() noexcept;
public:
	Graphics&	Get_gfx();
	HWND		Get_Handle();
	HINSTANCE	Get_Instance();
	int16_t		GetWidth();
	int16_t		GetHeight();
public:
	Window(int16_t _width, int16_t _height, const std::wstring name) noexcept;
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
};

