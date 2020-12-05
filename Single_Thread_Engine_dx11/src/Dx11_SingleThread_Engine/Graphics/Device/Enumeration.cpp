#include "Enumeration.h"

EnumOutput::~EnumOutput()
{
}
void EnumOutput::Set(UINT _Index, IDXGIOutput* _pOutput)
{
	Output_ = _Index;
	pOutput_ = _pOutput;
	pOutput_->GetDesc(&Desc_);
}

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

EnumDevice::~EnumDevice()
{
}

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

EnumAdapter::~EnumAdapter()
{
    EnumDevice* pDeviceInfo;
    for (int i = 0; i < DeviceInfoList_.size(); i++)
    {
        pDeviceInfo = DeviceInfoList_[i];
        pSAFE_DEL(pDeviceInfo);
    }
    DeviceInfoList_.clear();

    EnumOutput* pOutputInfo;
    for (int i = 0; i < OutputInfoList_.size(); i++)
    {
        pOutputInfo = OutputInfoList_[i];
        pSAFE_DEL(pOutputInfo);
    }
    OutputInfoList_.clear();

    dxSAFE_RELEASE(pAdapter_);
}

void EnumAdapter::Set(UINT _Index, IDXGIAdapter* _pAdapter)
{
    AdapterOrdinal_ = _Index;
    pAdapter_ = _pAdapter;
    pAdapter_->GetDesc(&AdapterDesc_);
}

/*--------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------*/

Enumeration::Enumeration()
{

}
Enumeration::~Enumeration()
{

}

HRESULT Enumeration::Enumerate(IDXGIFactory* _pFactory)
{
    HRESULT hr;
    if (_pFactory == nullptr)      
        return E_FAIL;

    ClearAdapterInfoList();

    for (int index = 0; ; ++index) {
        IDXGIAdapter* pAdapter = nullptr;

        // 컴퓨터에 장착된 어댑터(그래픽카드)객체 생성
        // 어댑터 객체가 여러개면 여러개 생성.
        hr = _pFactory->EnumAdapters(index, &pAdapter);
        if (FAILED(hr)) 
            break;

        EnumAdapter* pAdapterInfo = 0;
        pSAFE_NEW(pAdapterInfo, EnumAdapter);

        if (!pAdapterInfo)
            return E_OUTOFMEMORY;

        pAdapterInfo->Set(index, pAdapter);
        
        hr = EnumerateDevices(pAdapterInfo);
        if (FAILED(hr)) {
            delete pAdapterInfo;
            continue;
        }

        hr = EnumerateOutputs(pAdapterInfo);
        if (FAILED(hr) || pAdapterInfo->OutputInfoList_.size() <= 0) {
            delete pAdapterInfo;
            continue;
        }

        AdapterInfoList_.push_back(pAdapterInfo);
    }

    return S_OK;
}
HRESULT Enumeration::EnumerateDevices(EnumAdapter* _pAdapterInfo)
{
    HRESULT hr;
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    const UINT devTypeArrayCount = sizeof(driverTypes) / sizeof(driverTypes[0]);

    for (UINT iDeviceType = 0; iDeviceType < devTypeArrayCount; iDeviceType++) {
        EnumDevice* pDeviceInfo = 0;
        pSAFE_NEW(pDeviceInfo, EnumDevice);
        if (pDeviceInfo == nullptr)
            return E_OUTOFMEMORY;

        pDeviceInfo->AdapterOrdinal_ = _pAdapterInfo->AdapterOrdinal_;
        pDeviceInfo->DeviceType_ = driverTypes[iDeviceType];

        ID3D11Device* pd3dDevice = nullptr;
        ID3D11DeviceContext* pImmediateContext = nullptr;
        IDXGIAdapter* pAdapter = nullptr;

        D3D_DRIVER_TYPE ddt = driverTypes[iDeviceType];
        if (driverTypes[iDeviceType] == D3D_DRIVER_TYPE_HARDWARE) {
            pAdapter = _pAdapterInfo->pAdapter_;
            ddt = D3D_DRIVER_TYPE_UNKNOWN;
        }


        D3D_FEATURE_LEVEL getFeatureLevel;
        hr = D3D11CreateDevice(pAdapter, ddt, (HMODULE)0, NULL,
            featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &pd3dDevice, &getFeatureLevel, &pImmediateContext);
        if (FAILED(hr)) {
            delete pDeviceInfo;
            continue;
        }

        D3D11_FEATURE_DATA_D3D10_X_HARDWARE_OPTIONS ho;
        // 그래픽 구성 요소 장치 인터페이스를 지원하는지 확인.
        pd3dDevice->CheckFeatureSupport(D3D11_FEATURE_D3D10_X_HARDWARE_OPTIONS, &ho, sizeof(ho));

        dxSAFE_RELEASE(pd3dDevice);
        dxSAFE_RELEASE(pImmediateContext);

        _pAdapterInfo->DeviceInfoList_.push_back(pDeviceInfo);
    }

    return S_OK;
}

HRESULT Enumeration::EnumerateOutputs(EnumAdapter* pAdapterInfo)
{
    HRESULT hr;
    IDXGIOutput* pOutput;

    for (int iOutput = 0; ; ++iOutput) {
        pOutput = nullptr;
        // 디스플레이 서브시스템을 열거.
        hr = pAdapterInfo->pAdapter_->EnumOutputs(iOutput, &pOutput);
        if (DXGI_ERROR_NOT_FOUND == hr) {
            return S_OK;
        } else if (FAILED(hr)) {
            return hr;	//Something bad happened.
        } else {
            //Success!
            EnumOutput* pOutputInfo = new EnumOutput;
            if (!pOutputInfo) {
                dxSAFE_RELEASE(pOutput);
                return E_OUTOFMEMORY;
            }
            pOutputInfo->Set(iOutput, pOutput);

            EnumerateDisplayModes(pOutputInfo);
            if (pOutputInfo->DisplayModeList_.size() <= 0) {
                // If this output has no valid display mode, do not save it.
                delete pOutputInfo;
                continue;
            }

            pAdapterInfo->OutputInfoList_.push_back(pOutputInfo);
        }
    }
}
HRESULT Enumeration::EnumerateDisplayModes(EnumOutput* pOutputInfo)
{
    HRESULT hr = S_OK;
    DXGI_FORMAT allowedAdapterFormatArray[] =
    {
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,     //This is DXUT's preferred mode
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R16G16B16A16_FLOAT,
        DXGI_FORMAT_R10G10B10A2_UNORM
    };
    int allowedAdapterFormatArrayCount = sizeof(allowedAdapterFormatArray) / sizeof(allowedAdapterFormatArray[0]);

    for (int f = 0; f < allowedAdapterFormatArrayCount; ++f)
    {
        UINT NumModes = 512;
        hr = pOutputInfo->pOutput_->GetDisplayModeList(allowedAdapterFormatArray[f],
            DXGI_ENUM_MODES_SCALING,
            &NumModes,
            0);

        DXGI_MODE_DESC* pDesc = new DXGI_MODE_DESC[NumModes];
        assert(pDesc);
        if (!pDesc)
            return E_OUTOFMEMORY;

        hr = pOutputInfo->pOutput_->GetDisplayModeList(allowedAdapterFormatArray[f],
            DXGI_ENUM_MODES_SCALING,
            &NumModes,
            pDesc);

        if (DXGI_ERROR_NOT_FOUND == hr) {
            pSAFE_DELETE_ARRAY(pDesc);
            NumModes = 0;
            break;
        }

        if (SUCCEEDED(hr)) {
            for (UINT m = 0; m < NumModes; m++) {
                if (pDesc[m].Width >= 800 && pDesc[m].Height >= 600) {
                    pOutputInfo->DisplayModeList_.push_back(pDesc[m]);
                }
            }
        }
        pSAFE_DELETE_ARRAY(pDesc);
    }
    return hr;
}
void Enumeration::ClearAdapterInfoList()
{
    EnumAdapter* pAdapterInfo;
    for (int i = 0; i < AdapterInfoList_.size(); i++) {
        pAdapterInfo = AdapterInfoList_[i];
        pSAFE_DEL(pAdapterInfo);
    }

    AdapterInfoList_.clear();
}

