#pragma once
#include "../../common/define.h"
#include "../../common/macro.h"
#include <vector>


// output ��ü�� ���÷��� ����� ���� �ɵ�?
class EnumOutput
{
public:
	UINT						Output_;
	IDXGIOutput*				pOutput_;
	/*	 �������̽�
		FindClosestMatchingMode			: ��û�� ��忡�� ���� ����� (�Ǵ� ��û�� ��� ��� �̻�) ��带 �˻��Ѵ�
		GetDesc							: �Ӽ����� ��´�.
		GetDisplayModeList				: �����Ǵ� ���÷��� ��� ����� ��´�.
		GetDisplaySurfaceData			: ���� ǥ�õǰ� �ִ� ȭ���� ���纻�� ��´�..
		GetFrameStatistics				: ������ �����ӿ� ���� ��踦 ��´�.
		GetGammaControl					: ���õ� ���� ����� ��´�.
		GetGammaControlCapabilities		: ���� ����� ��´�.
		ReleaseOwnership				: ����� �����Ѵ�.
		SetDisplaySurface				: ��ġ�� ���÷��� ��带 �����Ѵ�.
		SetGammaControl					: ��� ���� ��Ʈ���� �����Ѵ�.
		TakeOwnership					: ����� �������� ��´�.
		WaitForVBlank					: ������� ���� ���� ȸ�ͼ� �ҰŸ� ��ٸ���.
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

// ����� ���� (�׷���ī��) ��ü
class EnumAdapter
{
public:
	UINT						AdapterOrdinal_;
	DXGI_ADAPTER_DESC			AdapterDesc_;
	/*	WCHAR Description[128];		: ������� ������ ���� ���ڿ�
		UINT VendorId;					: �ϵ���� ���޾�ü�� PCI ID
		UINT DeviceId;					: �ϵ���� ������ġ�� PCI ID
		UINT SubSysId;					: ���� �ý����� PCI ID
		UINT Revision;					: ������� ���� ��ȣ PCI ID
		SIZE_T DedicatedVideoMemory;	: CPU�� �������� �ʴ� ���� ���� �޸𸮷�(����Ʈ ����)
		SIZE_T DedicatedSystemMemory;	: GPU�� �������� �ʴ� ���� �ý��� �޸𸮷�(����Ʈ ����)
		SIZE_T SharedSystemMemory;		: ���� �ý��� �޸𸮷�(����Ʈ ����) ����Ϳ� ����
											���� �� �ִ� �ý��� �޸��� �ִ밪.
		LUID AdapterLuid;				: ����͸� �ĺ��ϴ� ������(DXGI.h�� �ִ� LUID����ü ����)
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
