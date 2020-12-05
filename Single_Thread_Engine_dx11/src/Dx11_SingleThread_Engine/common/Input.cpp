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

	// DirectInput 객체 생성
	if (FAILED(hr = DirectInput8Create(
		_hInst,					// DirectInput 객체를 생성하는 응용프로그램 or Dll의 인스턴스 핸들
		DIRECTINPUT_VERSION,	// DirectInput의 버전 번호. 보통 DIRECTINPUT_VERSION 사용
		IID_IDirectInput8,		// 인터페이스의 고유한 식별자
		(void**)&pDi_,			// 호출 성공시 반환받을 DirectInput 객체 주소
		NULL)))					// IUnknown 인터페이스의 주소. 일반적으로 NULL사용
	{
		return false;
	}

	if (_keyboard) {
		// 키보드 device 생성
		if (FAILED(pDi_->CreateDevice(
			GUID_SysKeyboard,	// 입력 장치의 인스턴스 GUID에 대한 참조 값
			&pDidevKey_,		// 반환 device 주소
			NULL)))				// 보통 null
			return false;

		// 데이터 형식 지정
		if (FAILED(pDidevKey_->SetDataFormat(&c_dfDIKeyboard)))
			return false;

		// 장치 동작 설정. 이 장치 인스턴스가 같은 장치의 다른 인스턴스 및 시스템이외의 부분과 데이터를 교환하는 방법.지정
		if (FAILED(hr = pDidevKey_->SetCooperativeLevel(hWnd_, 
			DISCL_NONEXCLUSIVE | DISCL_FOREGROUND | DISCL_NOWINKEY))) {	// 협조 레벨
			while (pDidevKey_->Acquire() == DIERR_INPUTLOST);
			if (FAILED(pDidevKey_->SetCooperativeLevel(hWnd_, DISCL_EXCLUSIVE | DISCL_BACKGROUND | DISCL_NOWINKEY)))
				return false;
		}
		/* 버퍼 데이터 / 직접 데이터
			버퍼링 데이터	: 응용 프로그램에서 데이터를 얻는 시점까지의 입력데이터를 저장하고 있는 방식.
							  준비된 버퍼의 크기가 넘어가면 이후 데이터는 저장되지 않는다.
							  현재 키 데이터의 상태보다 이벤트를 중요시하는 경우 선택 하는 것이 좋다.
							  ex) 마우스의 움직임이나 버튼의 클릭에 반응하는 응용프로그램 등... 양쪽 모두의 종류 데이터 사용 가능.
			직접 데이터		: 현재 상태의 snapshot. 장치의 현재 상태만을 필요로 하는 응용프로그램으로 사용하는 것이 좋다.	*/
		if (!dwImmediate_) {
			DIPROPDWORD dipdw;

			dipdw.diph.dwSize = sizeof(DIPROPDWORD);
			dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dipdw.diph.dwObj = 0;
			dipdw.diph.dwHow = DIPH_DEVICE;
			dipdw.dwData = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

			// 버퍼 데이터를 얻어오기 위해 버퍼 사이즈 설정.장치를 최초로 얻어오기 전에 실시해야 한다. 디폴트는 0
			// 바이트 단위 아니고 장치의 타입에 대응한 데이터의 항목 단위로 카운트 됨.
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


/* 매 프레임 호출시 현재 키보드 데이터 값을 조사하여 버퍼에 저장. 
	이후 여러 처리를 통해 키들의 데이터 상태 값을 조사할 수 있다.
*/
bool Input::KeyProcess()
{
	HRESULT hr;

	if (dwImmediate_) {
		ZeroMemory(KeyState_, sizeof(BYTE) * KEYSTATECOUNT);
		if (!pDidevKey_) return false;

		// 디바이스를 취득하는 함수. 
		if (FAILED(pDidevKey_->GetDeviceState(
			KEYSTATECOUNT,	// 파라미터내의 버퍼 사이즈
			KeyState_		// 장치의 현재 상태를 받는 구조체 주소. SetDataFormat 함수를 호출하기 전에 데이터 형식을 설정.
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

	WINAPI DirectInput8Create 함수가 성공했을 경우는 DI_OK 를 돌려주며 
	실패했을 경우는 다음과 같은 에러 값들이 리턴 된다.

	DIERR_BETADIRECTINPUTVERSION	DirectInput의 지원되지 않는 버전용으로 생성.
	DIERR_INVALIDPARAM				잘못된 파라미터의 사용
	DIERR_OLDDIRECTINPUTVERSION		새로운 버전의 DirectInput 를 필요.
	DIERR_OUTOFMEMORY				충분한 메모리를 할당할 수가 없다.

	////////////////////////////////////////////////////////////////////////////

	ipDidevKey_->SetDataFormat(&c_dfDIKeyboard)

	유일한 인자 값 LPCDIDATAFORMAT lpdf; 에는 장치의 종류에
	따라(c_dfDIKeyboard /c_dfDIMouse /c_dfDIMouse2 / c_dfDIJoystick /c_dfDIJoystick2) 사용하며 반환 값은 다음과 같다.

	DIERR_ACQUIRED				장치가 취득하기 위하여 실행할 수 없다.
	DIERR_INVALIDPARAM			잘못된 파라미터가 사용되었다.
	DIERR_NOTINITIALIZED		개체는 초기화되지 않다.


	////////////////////////////////////////////////////////////////////////////

	SetCooperativeLevel flags

	DISCL_BACKGROUND	백그라운드 액세스 권한이 허가되면 할당된 윈도우가 활성화 
						윈도우가 아닌 경우에서도 언제라도 장치를 취득할 수 있다. 
						응용프로그램이 활성화된 여부에 상관없이 입력장치의 사용이 가능하다.
	
	DISCL_FOREGROUND	Foreground 액세스가 부여되면 할당된 윈도우가 백그라운드로 이동할 때 
						장치는 자동적으로 릴리즈 되며 응용프로그램이 활성화 중일 경우에 입력장치들 사용할 수 있다.

	DISCL_EXCLUSIVE		배타적 액세스 권한이 주어지면 장치가 취득되고 있는 동안 
						장치의 다른 인스턴스는 그 장치의 베타 액세스 권한을 취득할 수 없다.

	DISCL_NONEXCLUSIVE	응용프로그램은 비 배타적 액세스 권한을 요구한다. 
						장치에 대한 액세스 권한은 같은 장치에 액세스 하고 있는 다른응용프로그램을 방해하지 않는다.

	DISCL_NOWINKEY		Microsoft Windows® 키를 무효로 한다. 
						이 플래그를 설정 하면 사용자는 잘못해 응용프로그램을 종료하는 것이 없어진다..

	////////////////////////////////////////////////////////////////////////////

	GetDeviceState
	
	대응하는 장치 상태의 구조체

	c_dfDIMouse			DIMOUSESTATE
	c_dfDIMouse2		DIMOUSESTATE2
	c_dfDIKeyboard		256 바이트의 배열
	c_dfDIJoystick		DIJOYSTATE
	c_dfDIJoystick2		DIJOYSTATE2
*/