#pragma once
#include "../../common/define.h"
#include "../../common/macro.h"
#include <vector>


// output 객체는 디스플레이 장비라고 보면 될듯?
class EnumOutput
{
public:
	UINT						Output_;
	IDXGIOutput*				pOutput_;
	/*	 인터페이스
		FindClosestMatchingMode			: 요청한 모드에서 가장 가까운 (또는 요청된 모드 기능 이상) 모드를 검색한다
		GetDesc							: 속성값을 얻는다.
		GetDisplayModeList				: 지원되는 디스플레이 모드 목록을 얻는다.
		GetDisplaySurfaceData			: 현재 표시되고 있는 화면의 복사본을 얻는다..
		GetFrameStatistics				: 렌더링 프레임에 대한 통계를 얻는다.
		GetGammaControl					: 세팅된 감마 기능을 얻는다.
		GetGammaControlCapabilities		: 감마 기능을 얻는다.
		ReleaseOwnership				: 출력을 해제한다.
		SetDisplaySurface				: 장치의 디스플레이 모드를 변경한다.
		SetGammaControl					: 출력 감마 컨트롤을 설정한다.
		TakeOwnership					: 출력의 소유권을 얻는다.
		WaitForVBlank					: 모니터의 다음 수직 회귀선 소거를 기다린다.
	*/									
	DXGI_OUTPUT_DESC			Desc_;
	std::vector<DXGI_MODE_DESC>	DisplayModeList_;
public:
	void						Set(UINT _Index, IDXGIOutput* _pOutput);
public:
	~EnumOutput();
};

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

class EnumDevice
{
public:
	UINT						AdapterOrdinal_;
	std::vector<EnumOutput*>	OutputInfoList_;
	D3D_DRIVER_TYPE				DeviceType_;
public:
	~EnumDevice();
};

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

// 어댑터 정보 (그래픽카드) 객체
class EnumAdapter
{
public:
	UINT						AdapterOrdinal_;
	DXGI_ADAPTER_DESC			AdapterDesc_;
	/*	WCHAR Description[128];		: 어댑터의 종류에 대한 문자열
		UINT VendorId;					: 하드웨어 공급업체의 PCI ID
		UINT DeviceId;					: 하드웨어 공급장치의 PCI ID
		UINT SubSysId;					: 서브 시스템의 PCI ID
		UINT Revision;					: 어댑터의 개정 번호 PCI ID
		SIZE_T DedicatedVideoMemory;	: CPU와 공유하지 않는 전용 비디오 메모리량(바이트 단위)
		SIZE_T DedicatedSystemMemory;	: GPU와 공유하지 않는 전용 시스템 메모리량(바이트 단위)
		SIZE_T SharedSystemMemory;		: 고유 시스템 메모리량(바이트 단위) 어댑터에 의해
											사용될 수 있는 시스템 메모리의 최대값.
		LUID AdapterLuid;				: 어댑터를 식별하는 고유값(DXGI.h에 있는 LUID구조체 참조)
	*/
	IDXGIAdapter*				pAdapter_;
	std::vector<EnumOutput*>	OutputInfoList_;
	std::vector<EnumDevice*>	DeviceInfoList_;
public:
	void					Set(UINT iIndex, IDXGIAdapter* pAdapter);
public:
	~EnumAdapter();
};

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

class Enumeration
{
public:
	std::vector<EnumAdapter*>	AdapterInfoList_;
public:
	HRESULT Enumerate(IDXGIFactory* pFactory);
	HRESULT EnumerateDevices(EnumAdapter* pAdapterInfo);
	HRESULT EnumerateDisplayModes(EnumOutput* pOutputInfo);
	HRESULT EnumerateOutputs(EnumAdapter* pAdapterInfo);
	void	ClearAdapterInfoList();
public:
	Enumeration(void);
	virtual ~Enumeration(void);
};
