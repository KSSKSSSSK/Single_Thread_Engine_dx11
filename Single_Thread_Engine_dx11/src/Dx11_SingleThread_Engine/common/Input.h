#pragma once
#include <dinput.h>

#define SAMPLE_BUFFER_SIZE	16 
#define KEYSTATECOUNT		256

class Input
{
public:
	HWND					hWnd_;
	LPDIRECTINPUT8			pDi_;
	LPDIRECTINPUTDEVICE8	pDidevKey_;
	LPDIRECTINPUTDEVICE8	pDidevMouse_;	
	DWORD					dwImmediate_;		// 버퍼링 데이터, 직접 데이터 여부
	DWORD					dwElements_;

	DIDEVICEOBJECTDATA		didod_[SAMPLE_BUFFER_SIZE];		// 버퍼링 데이터 획득
	BYTE					KeyState_[KEYSTATECOUNT];		// 직접 데이터 획득
	BYTE					KeyBeforeState_[KEYSTATECOUNT];	// 직접 데이터 모드는 Button_up이 없다.  때문에 이전 상태 값을 저장하는버퍼
	DIMOUSESTATE			MouseState_;
	DIMOUSESTATE			MouseBeforeState_;
public:
	static Input& GetInstance()
	{
		static Input theSingleInstance;

		return theSingleInstance;
	}
public:
	bool Init();
	bool Frame();
	bool Render();
	bool Release();
	bool ResetDevice();
public:
	bool InitDirectInput(HINSTANCE _hInst, HWND _hWnd, bool _keyboard, bool _mouse);

	bool KeyProcess();
	bool MouseProcess();
	void PostProcess();

	bool IsKeyDown(DWORD dwKey);
	bool IsKeyUP(DWORD dwKey);

	void DeviceAcquire();
	void DeviceUnacquire();
	void SetAcquire(bool _Active);
public:
	Input();
	~Input();
};

/*
	윈도우 메세지에 의존하지 않고 하드웨어 드라이버와 직접 통신해 고속화된 입력 데이터의 접근 가능.
	백그라운드에서도 액세스 권한 취득가능. force feedback등 모든 타입의 입력 장치를 지원한다.

*/