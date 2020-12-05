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
	DWORD					dwImmediate_;		// ���۸� ������, ���� ������ ����
	DWORD					dwElements_;

	DIDEVICEOBJECTDATA		didod_[SAMPLE_BUFFER_SIZE];		// ���۸� ������ ȹ��
	BYTE					KeyState_[KEYSTATECOUNT];		// ���� ������ ȹ��
	BYTE					KeyBeforeState_[KEYSTATECOUNT];	// ���� ������ ���� Button_up�� ����.  ������ ���� ���� ���� �����ϴ¹���
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
	������ �޼����� �������� �ʰ� �ϵ���� ����̹��� ���� ����� ���ȭ�� �Է� �������� ���� ����.
	��׶��忡���� �׼��� ���� ��氡��. force feedback�� ��� Ÿ���� �Է� ��ġ�� �����Ѵ�.

*/