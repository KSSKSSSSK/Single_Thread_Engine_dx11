#include "Input.h"

#define KEYDOWN(key)	(((KeyState_[key]) & 0x80) ? true : false)
#define KEYUP(key)		(((KeyState_[key]) & 0x80) ? false : true)

Input::Input()
{
	pDi_ = NULL;
	pDidevKey_ = NULL;
	pDidevMouse_ = NULL;
	hWnd_ = NULL;
	dwElements_ = 0;
	dwImmediate_ = TRUE;
}
Input::~Input()
{
	Release();
}

bool Input::Init()
{
	memset(&KeyBeforeState_, 0, sizeof(BYTE) * KEYSTATECOUNT);
	memset(&MouseBeforeState_, 0, sizeof(DIMOUSESTATE));
	dwElements_ = 0;

	return true;
}
bool Input::Frame()
{
	if (!KeyProcess() || !MouseProcess()) {
		return false;
	}
	return true;
}
bool Input::Render()
{
	return true;
}
bool Input::Release()
{
	if (pDi_) {
		if (pDidevKey_) {
			pDidevKey_->Unacquire();
			pDidevKey_->Release();
			pDidevKey_ = NULL;
		}
		if (pDidevMouse_) {
			pDidevMouse_->Unacquire();
			pDidevMouse_->Release();
			pDidevMouse_ = NULL;
		}

		pDi_->Release();
		pDi_ = NULL;
	}

	return true;
}
bool Input::ResetDevice()
{
	Release();
	Init();
	return true;
}

bool Input::InitDirectInput(HINSTANCE _hInst, HWND _hWnd, bool _keyboard, bool _mouse)
{
	HRESULT hr;//GetModuleHandle(NULL)
	hWnd_ = _hWnd;
	if (pDidevKey_ || pDidevMouse_) return true;

	// DirectInput ��ü ����
	if (FAILED(hr = DirectInput8Create(
		_hInst,					// DirectInput ��ü�� �����ϴ� �������α׷� or Dll�� �ν��Ͻ� �ڵ�
		DIRECTINPUT_VERSION,	// DirectInput�� ���� ��ȣ. ���� DIRECTINPUT_VERSION ���
		IID_IDirectInput8,		// �������̽��� ������ �ĺ���
		(void**)&pDi_,			// ȣ�� ������ ��ȯ���� DirectInput ��ü �ּ�
		NULL)))					// IUnknown �������̽��� �ּ�. �Ϲ������� NULL���
	{
		return false;
	}

	if (_keyboard) {
		// Ű���� device ����
		if (FAILED(pDi_->CreateDevice(
			GUID_SysKeyboard,	// �Է� ��ġ�� �ν��Ͻ� GUID�� ���� ���� ��
			&pDidevKey_,		// ��ȯ device �ּ�
			NULL)))				// ���� null
			return false;

		// ������ ���� ����
		if (FAILED(pDidevKey_->SetDataFormat(&c_dfDIKeyboard)))
			return false;

		// ��ġ ���� ����. �� ��ġ �ν��Ͻ��� ���� ��ġ�� �ٸ� �ν��Ͻ� �� �ý����̿��� �κа� �����͸� ��ȯ�ϴ� ���.����
		if (FAILED(hr = pDidevKey_->SetCooperativeLevel(hWnd_, 
			DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY))) {	// ���� ����
			while (pDidevKey_->Acquire() == DIERR_INPUTLOST);
			if (FAILED(pDidevKey_->SetCooperativeLevel(hWnd_, DISCL_EXCLUSIVE | DISCL_BACKGROUND | DISCL_NOWINKEY)))
				return false;
		}
		/* ���� ������ / ���� ������
			���۸� ������	: ���� ���α׷����� �����͸� ��� ���������� �Էµ����͸� �����ϰ� �ִ� ���.
							  �غ�� ������ ũ�Ⱑ �Ѿ�� ���� �����ʹ� ������� �ʴ´�.
							  ���� Ű �������� ���º��� �̺�Ʈ�� �߿���ϴ� ��� ���� �ϴ� ���� ����.
							  ex) ���콺�� �������̳� ��ư�� Ŭ���� �����ϴ� �������α׷� ��... ���� ����� ���� ������ ��� ����.
			���� ������		: ���� ������ snapshot. ��ġ�� ���� ���¸��� �ʿ�� �ϴ� �������α׷����� ����ϴ� ���� ����.	*/
		if (!dwImmediate_) {
			DIPROPDWORD dipdw;

			dipdw.diph.dwSize = sizeof(DIPROPDWORD);
			dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dipdw.diph.dwObj = 0;
			dipdw.diph.dwHow = DIPH_DEVICE;
			dipdw.dwData = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

			// ���� �����͸� ������ ���� ���� ������ ����.��ġ�� ���ʷ� ������ ���� �ǽ��ؾ� �Ѵ�. ����Ʈ�� 0
			// ����Ʈ ���� �ƴϰ� ��ġ�� Ÿ�Կ� ������ �������� �׸� ������ ī��Ʈ ��.
			if (FAILED(hr = pDidevKey_->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
				return false;
		}

		while (pDidevKey_->Acquire() == DIERR_INPUTLOST);
	}

	if (_mouse) {
		if (FAILED(pDi_->CreateDevice(GUID_SysMouse, &pDidevMouse_, NULL)))
			return false;

		if (FAILED(pDidevMouse_->SetDataFormat(&c_dfDIMouse)))
			return false;

		if (FAILED(pDidevMouse_->SetCooperativeLevel(hWnd_, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND))) {
			while (pDidevMouse_->Acquire() == DIERR_INPUTLOST);
			if (FAILED(pDidevMouse_->SetCooperativeLevel(hWnd_, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) 
				return false;

		}
		while (pDidevMouse_->Acquire() == DIERR_INPUTLOST);
	}

	return true;
}


/* �� ������ ȣ��� ���� Ű���� ������ ���� �����Ͽ� ���ۿ� ����. 
	���� ���� ó���� ���� Ű���� ������ ���� ���� ������ �� �ִ�.
*/
bool Input::KeyProcess()
{
	HRESULT hr;

	if (dwImmediate_) {
		ZeroMemory(KeyState_, sizeof(BYTE) * KEYSTATECOUNT);
		if (!pDidevKey_) return false;

		// ����̽��� ����ϴ� �Լ�. 
		if (FAILED(pDidevKey_->GetDeviceState(
			KEYSTATECOUNT,	// �Ķ���ͳ��� ���� ������
			KeyState_		// ��ġ�� ���� ���¸� �޴� ����ü �ּ�. SetDataFormat �Լ��� ȣ���ϱ� ���� ������ ������ ����.
		))) {
			while (pDidevKey_->Acquire() == DIERR_INPUTLOST)
				pDidevKey_->Acquire();
			return true;
		}
	} else {
		if (NULL == pDidevKey_)     return false;
		memset(&didod_, 0, sizeof(DIDEVICEOBJECTDATA) * SAMPLE_BUFFER_SIZE);
		dwElements_ = SAMPLE_BUFFER_SIZE;
		hr = pDidevKey_->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didod_, &dwElements_, 0);

		if (hr != DI_OK) {
			dwElements_ = 0;
			hr = pDidevKey_->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = pDidevKey_->Acquire();
			return true;
		}
	}
	return true;
}
bool Input::MouseProcess()
{
	ZeroMemory(&MouseState_, sizeof(DIMOUSESTATE));
	if (!pDidevMouse_) return false;

	if (FAILED(pDidevMouse_->GetDeviceState(sizeof(DIMOUSESTATE), &MouseState_))) {
		while (pDidevMouse_->Acquire() == DIERR_INPUTLOST)
			pDidevMouse_->Acquire();

		return true;
	}

	return true;
}
void Input::PostProcess()
{
	memcpy(&KeyBeforeState_, &KeyState_, sizeof(BYTE) * KEYSTATECOUNT);
	memcpy(&MouseBeforeState_, &MouseState_, sizeof(DIMOUSESTATE));
}

bool Input::IsKeyDown(DWORD dwKey)
{
	if (dwImmediate_) {
		if (KEYDOWN(dwKey)) 		return true;
	} else {
		for (DWORD i = 0; i < dwElements_; i++) {
			// this will display then scan code of the key
			// plus a 'D' - meaning the key was pressed 
			//   or a 'U' - meaning the key was released
			// DOWN
			if (didod_[i].dwOfs == dwKey && (didod_[i].dwData & 0x80)) {
				return true;
			}
		}
	}

	return false;
}
bool Input::IsKeyUP(DWORD dwKey)
{
	if (dwImmediate_) {
		if (KeyBeforeState_[dwKey] & 0x80) {
			if (KEYUP(dwKey))
				return true;
		}
	} else {
		for (DWORD i = 0; i < dwElements_; i++) {
			// this will display then scan code of the key
			// plus a 'D' - meaning the key was pressed 
			//   or a 'U' - meaning the key was released	
			if (didod_[i].dwOfs == dwKey && !(didod_[i].dwData & 0x80))	{
				return true;
			}
		}
	}

	return false;
}

void Input::DeviceAcquire()
{
	if (pDidevKey_)
		pDidevKey_->Acquire();
	if (pDidevMouse_)
		pDidevMouse_->Acquire();
}
void Input::DeviceUnacquire()
{
	if (pDidevKey_)
		pDidevKey_->Unacquire();
	if (pDidevMouse_)
		pDidevMouse_->Unacquire();
}
void Input::SetAcquire(bool _Active)
{
	if (_Active) DeviceAcquire();
	else		  DeviceUnacquire();
}

/*
	DirectInput8Create

	WINAPI DirectInput8Create �Լ��� �������� ���� DI_OK �� �����ָ� 
	�������� ���� ������ ���� ���� ������ ���� �ȴ�.

	DIERR_BETADIRECTINPUTVERSION	DirectInput�� �������� �ʴ� ���������� ����.
	DIERR_INVALIDPARAM				�߸��� �Ķ������ ���
	DIERR_OLDDIRECTINPUTVERSION		���ο� ������ DirectInput �� �ʿ�.
	DIERR_OUTOFMEMORY				����� �޸𸮸� �Ҵ��� ���� ����.

	////////////////////////////////////////////////////////////////////////////

	ipDidevKey_->SetDataFormat(&c_dfDIKeyboard)

	������ ���� �� LPCDIDATAFORMAT lpdf; ���� ��ġ�� ������
	����(c_dfDIKeyboard /c_dfDIMouse /c_dfDIMouse2 / c_dfDIJoystick /c_dfDIJoystick2) ����ϸ� ��ȯ ���� ������ ����.

	DIERR_ACQUIRED				��ġ�� ����ϱ� ���Ͽ� ������ �� ����.
	DIERR_INVALIDPARAM			�߸��� �Ķ���Ͱ� ���Ǿ���.
	DIERR_NOTINITIALIZED		��ü�� �ʱ�ȭ���� �ʴ�.


	////////////////////////////////////////////////////////////////////////////

	SetCooperativeLevel flags

	DISCL_BACKGROUND	��׶��� �׼��� ������ �㰡�Ǹ� �Ҵ�� �����찡 Ȱ��ȭ 
						�����찡 �ƴ� ��쿡���� ������ ��ġ�� ����� �� �ִ�. 
						�������α׷��� Ȱ��ȭ�� ���ο� ������� �Է���ġ�� ����� �����ϴ�.
	
	DISCL_FOREGROUND	Foreground �׼����� �ο��Ǹ� �Ҵ�� �����찡 ��׶���� �̵��� �� 
						��ġ�� �ڵ������� ������ �Ǹ� �������α׷��� Ȱ��ȭ ���� ��쿡 �Է���ġ�� ����� �� �ִ�.

	DISCL_EXCLUSIVE		��Ÿ�� �׼��� ������ �־����� ��ġ�� ���ǰ� �ִ� ���� 
						��ġ�� �ٸ� �ν��Ͻ��� �� ��ġ�� ��Ÿ �׼��� ������ ����� �� ����.

	DISCL_NONEXCLUSIVE	�������α׷��� �� ��Ÿ�� �׼��� ������ �䱸�Ѵ�. 
						��ġ�� ���� �׼��� ������ ���� ��ġ�� �׼��� �ϰ� �ִ� �ٸ��������α׷��� �������� �ʴ´�.

	DISCL_NOWINKEY		Microsoft Windows�� Ű�� ��ȿ�� �Ѵ�. 
						�� �÷��׸� ���� �ϸ� ����ڴ� �߸��� �������α׷��� �����ϴ� ���� ��������..

	////////////////////////////////////////////////////////////////////////////

	GetDeviceState
	
	�����ϴ� ��ġ ������ ����ü

	c_dfDIMouse			DIMOUSESTATE
	c_dfDIMouse2		DIMOUSESTATE2
	c_dfDIKeyboard		256 ����Ʈ�� �迭
	c_dfDIJoystick		DIJOYSTATE
	c_dfDIJoystick2		DIJOYSTATE2
*/