#include "Graphics.h"


typedef HRESULT(WINAPI* LPCREATEDXGIFACTORY)(REFIID, void**);
typedef HRESULT(WINAPI* LPD3D11CREATEDEVICE)(__in_opt IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	__in_ecount_opt(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	__out_opt ID3D11Device** ppDevice,
	__out_opt D3D_FEATURE_LEVEL* pFeatureLevel,
	__out_opt ID3D11DeviceContext** ppImmediateContext);


static HMODULE				s_hModDXGI	= NULL;
static HMODULE				s_hModD3D11 = NULL;

static LPCREATEDXGIFACTORY	s_DynamicCreateDXGIFactory = NULL;
static LPD3D11CREATEDEVICE	s_DynamicD3D11CreateDevice = NULL;

static bool EnsureD3D11APIs(void)
{
	if (s_hModD3D11 != NULL || s_hModDXGI != NULL)
		return true;

	// This may fail if Direct3D 10 isn't installed
	s_hModD3D11 = LoadLibrary(L"d3d11.dll");
	if (s_hModD3D11 != NULL) {
		s_DynamicD3D11CreateDevice = (LPD3D11CREATEDEVICE)GetProcAddress(s_hModD3D11, "D3D11CreateDevice");
	}

	s_hModDXGI = LoadLibrary(L"dxgi.dll");
	if (s_hModDXGI) {
		s_DynamicCreateDXGIFactory = (LPCREATEDXGIFACTORY)GetProcAddress(s_hModDXGI, "CreateDXGIFactory");
	}

	return (s_hModDXGI != NULL) && (s_hModD3D11 != NULL);
}


Graphics::Graphics(HWND _hWnd, int16_t _width, int16_t _hight)
	: Enumeration_(),
	clear_color_(0, 0, 0, 0),
	width_(_width),
	height_(_hight)
{
	// dx11 dll 여부
	if (!EnsureD3D11APIs()) {
		WCHAR strBuffer[512];
		wcscpy_s(strBuffer, ARRAYSIZE(strBuffer),
			L"This application requires a Direct3D 11 class\ndevice (hardware or reference rasterizer) running on Windows Vista Or Window7 (or later).");
		MessageBox(0, strBuffer, L"Could not initialize Direct3D 11", MB_OK);
		return;
	}

	HRESULT hr = E_FAIL;

	//////////////////////////////////////////////////

	// Device, SwapChain
	if (FAILED(hr = CreateDevice())) {

	}
	// Dxgi
	if (FAILED(hr = CreateGIFactory())) {

	}

	if (FAILED(hr = CreateSwapChain(_hWnd, _width, _hight))) {

	}

	//if (FAILED(hr = CreateDeviceAndSwapchain(_hWnd, _width, _hight))) {
	//
	//}



	// rendertarget
	if (FAILED(hr = SetRenderTargetView())) {

	}


	// create depth stensil state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	pDevice_->CreateDepthStencilState(&dsDesc, pDSState.GetAddressOf());

	pContext_->OMSetDepthStencilState(pDSState.Get(), 1u);

	// create depth stensil texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = swapchain_desc_.BufferDesc.Width;
	descDepth.Height = swapchain_desc_.BufferDesc.Height;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	pDevice_->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;

	pDevice_->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, pDSV_.GetAddressOf());



	SetViewport();
}
Graphics::~Graphics()
{
}


void Graphics::DrawIndexed(UINT _count) noexcept
{
	pContext_->DrawIndexed(_count, 0u, 0u);
}

void Graphics::EndFrame()
{
	pSwap_->Present(1u, 0u);
}

void Graphics::ClearBuffer(float _red, float _green, float _blue) noexcept
{
	pContext_->ClearRenderTargetView(pTarget_.Get(), clear_color_.get_color());
	// pContext_->ClearDepthStencilView(pDSV_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection_;
}
void Graphics::SetProjection(DirectX::FXMMATRIX _proj) noexcept
{
	projection_ = _proj;
}

HRESULT Graphics::CreateRenderTargetView()
{
	HRESULT hr;

	// gain access to texture subresourece in swap chain (back buffer)
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;
	Microsoft::WRL::ComPtr<IDXGISurface1>	pBackBuffer_write;

	hr = pSwap_->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()));
	if (FAILED(hr))
		return hr;

	hr = pSwap_->GetBuffer(0, __uuidof(IDXGISurface1), reinterpret_cast<void**>(pBackBuffer_write.GetAddressOf()));
	if (FAILED(hr))
		return hr;


	hr = pDevice_->CreateRenderTargetView(pBackBuffer.Get(), nullptr, pTarget_.GetAddressOf());
	if (FAILED(hr))
		return hr;

	return hr;
}
HRESULT Graphics::SetRenderTargetView()
{
	HRESULT hr = S_OK;

	if (FAILED(hr = CreateRenderTargetView())) {
		return hr;
	}

	// bind depth stensil view to OM
	pContext_->OMSetRenderTargets(1u, pTarget_.GetAddressOf(), pDSV_.Get());

	return hr;
}

HRESULT Graphics::SetViewport()
{
	HRESULT hr = S_OK;
	DXGI_SWAP_CHAIN_DESC Desc;
	pSwap_->GetDesc(&Desc);

	viewport_desc_.Width = Desc.BufferDesc.Width;
	viewport_desc_.Height = Desc.BufferDesc.Height;
	viewport_desc_.MinDepth = 0.0f;
	viewport_desc_.MaxDepth = 1.0f;
	viewport_desc_.TopLeftX = 0.0f;
	viewport_desc_.TopLeftY = 0.0f;

	pContext_->RSSetViewports(1u, &viewport_desc_);

	return hr;
}

////////////////////////////////////////////////////
HRESULT Graphics::CreateDevice()
{
	HRESULT hr;
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driver_type_ = driverTypes[driverTypeIndex];
		if (SUCCEEDED(hr = D3D11CreateDevice(NULL, driver_type_, NULL, createDeviceFlags,
			featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, pDevice_.GetAddressOf(), &feature_level_, pContext_.GetAddressOf()))) {

			if (FAILED(hr) || feature_level_ < D3D_FEATURE_LEVEL_11_0) {
				if (pContext_.Get()) pContext_->Release();
				if (pDevice_.Get()) pDevice_->Release();
				continue;
			}

			break;
		}
	}
	if (FAILED(hr))       return hr;
}
HRESULT	Graphics::CreateSwapChain(HWND _hWnd, UINT _width, UINT _height, BOOL _IsFullScreen)
{
	/* Swapchain
	DXGI_SWAP_CHAIN_DESC
	BufferDesc		: 백 버퍼 모드를 나타내는 DXGI_MODE_DESC 구조
	Width			: 해상도의 너비
	Height			: 행상도의 높이
	RefreshRate		: 재생빈도 (Hz단위)를 나타내는 DXGI_RATIONAL구조
	Numerator	: 재생빈도의 분자값 (보통 60)
	Denominator	: 재생빈도의 분모값 (보통 1)
	Format			: 백버퍼 출력 픽셀 포맷을 나타내는 DXGI_FORMAT 구조. (보통 DXGI_FORMAT_R8G8B8A8_UNORM))
	SanlineOrdering : 주사선 렌더링 모드의 형식
	Scaling			: 크기 조정 모드.
	SampleDesc		: 멀티 샘플링 매개 변수를 나타내는 DXGI_SAMPLE_DESC 구조
				(1개 이상의 픽셀들을 조합, 연산하여 새로운 픽셀값을 계산. 부드럽고 어울리는 픽셀계산, 안티앨리아싱)
	Count		: 픽셀 단위의 멀티 샘플링의 수 (픽셀당 샘플링할 픽셀의 개수 1이면 멀티 샘플링 x)
	Quality		: 이미지의 품질 수준이 반환된다. 품질이 높을수록 성능이 저하된다.
				(유효범위 0 ~ CheckMultisampleQualityLevels함수의 반환값 -1까지 사용 가능)
	BufferUsage		: 백 버퍼 표면 사용법 및 CPU 액세스 옵션을 나타내는 DXGI_USAGE 열거 형 멤버.
				백 버퍼는 쉐이더를 입력하거나 렌더타겟 출력으로 사용할 수 있다.
	BufferCount		: 스왑 체인의 버퍼 개수.
	OutputWindow	: 출력 창에 HWND 핸들. NULL이 될 수 없다.
	Windowed		: 윈도우 모드 = True, 풀스크린 모드 = false
	SwapEffect		: 플립핑 이후 렌더 타겟 버퍼의 내용을 처리하기 위한 DXGI_SWAP_EFFECT 열거 형
					(백 버퍼의 내용을 어떻게 보관 할지 결정)
	Falgs			: 스왑체인동작 옵션을 나타내는 DXGI_SWAP_CHAIN_FLAG 열거형. */

	HRESULT hr = S_OK;
	SetFullScreenFlag(_IsFullScreen);
	if (pGIFactory_.Get() == nullptr) return S_FALSE;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _width;
	sd.BufferDesc.Height = _height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = !IsFullScreenMode_;
	// 추가
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	if (FAILED(hr = pGIFactory_->CreateSwapChain(pDevice_.Get(), &sd, pSwap_.GetAddressOf()))) {
		return hr;
	}

	return hr;
}

HRESULT Graphics::CreateDeviceAndSwapchain(HWND _hWnd, UINT _width, UINT _height)
{
	HRESULT hr;

	hWnd_ = _hWnd;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_UNKNOWN,
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
	};
	UINT numFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);


	swapchain_desc_.BufferDesc.Width = width_;
	swapchain_desc_.BufferDesc.Height = height_;
	swapchain_desc_.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapchain_desc_.BufferDesc.RefreshRate.Numerator = 60;
	swapchain_desc_.BufferDesc.RefreshRate.Denominator = 1;
	swapchain_desc_.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchain_desc_.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchain_desc_.SampleDesc.Count = 1;
	swapchain_desc_.SampleDesc.Quality = 0;
	swapchain_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc_.BufferCount = 1;
	swapchain_desc_.OutputWindow = _hWnd;
	swapchain_desc_.Windowed = TRUE;
	swapchain_desc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchain_desc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		driver_type_ = driverTypes[driverTypeIndex];

		if (SUCCEEDED(hr = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			&feature_level_, 0, D3D11_SDK_VERSION, &swapchain_desc_,
			pSwap_.GetAddressOf(), pDevice_.GetAddressOf(), &feature_level_, pContext_.GetAddressOf()))) {
			if (FAILED(hr) || feature_level_ < D3D_FEATURE_LEVEL_11_0) {
				if (pContext_)
					pContext_->Release();

				if (pDevice_)
					pDevice_->Release();

				continue;
			}
			break;
		}
	}

	return hr;
}
HRESULT Graphics::CreateGIFactory()
{
	HRESULT hr = E_FAIL;

	if (FAILED(hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)pGIFactory_.GetAddressOf()))) {
		//DXTRACE_ERR_MSGBOX( DXGetErrorDescription(hr), hr );				
		//MessageBox( m_hWnd, DXGetErrorDescription(hr), DXGetErrorString(hr), MB_OK );			
	}
	//ALT + Enter키로 상호 전황을 활성화 및 비활성화 할 수 있다.
	// 키 전환 기능이 활성화가 목적이라면 기본값으로 세팅되어 있어 별도의 코드 추가 필요 x.
	/*		pGIFactory_.Get()->MakeWindowAssociation(hWnd_, flags);
	
		DXGI_MWA_NO_WINDOW_CHANGES 메시지 큐를 모니터링 하지 않게 된다.
		DXGI_MWA_NO_ALT_ENTER Alt + Enter 작업에 응답하지 않도록 한다
		DXGI_MWA_NO_PRINT_SCREEN PrintScreen키가 응답하지 않도록 한다.	*/

	if (FAILED(Enumeration_.Enumerate(pGIFactory_.Get()))) {
		return hr;
	}

	return hr;
}

HRESULT Graphics::ResizeDevice(UINT _width, UINT _height)
{
	if (pDevice_.Get() == nullptr)
		return true;

	HRESULT hr;

	pContext_->OMSetRenderTargets(0, nullptr, nullptr);
	if (pTarget_.Get())
		pTarget_.Reset();

	DXGI_SWAP_CHAIN_DESC CurrentSD, AfterSD;
	// 현재 설정되어 있는 스왑 버퍼(백 버퍼)의 정보를 얻는 함수.
	// 디바이스 생성시 DXGI_SWAP_CHAIN_DESC의 구조체 정보를 얻을 수 있다.
	// 크기빼고는 그대로 사용.
	pSwap_->GetDesc(&CurrentSD);
	pSwap_->ResizeBuffers(CurrentSD.BufferCount,	// 스왑체인의 버퍼 수
		_width, _height,				// 백 버퍼의 새로운 폭, 높이 0지정시 창의 클라이언트 영역 사용	
		CurrentSD.BufferDesc.Format,	// 백 버퍼의 새로운 형식
		0);								// 스왑 체인의 기능을 나타내는 플래그 DXGI_SWAP_CHAIN_FLAG

	// 변경된 백버퍼의 크기를 얻는다.
	pSwap_->GetDesc(&AfterSD);
	width_ = AfterSD.BufferDesc.Width;
	height_ = AfterSD.BufferDesc.Height;


	if (FAILED(hr = SetRenderTargetView()))
	{
		return hr;
	}

	// 뷰포트를 세팅하고 적용한다.
	if (FAILED(hr = SetViewport()))
	{
		return hr;
	}

	return hr;
}

void Graphics::ChangeResolution(int16_t width, int16_t height)
{
	if (pSwap_.Get())
	{
		DXGI_MODE_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.RefreshRate.Denominator = 0;
		desc.RefreshRate.Numerator = 0;
		desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		// 윈도우 크기 및 해상도( 화면모드 ) 변경
		pSwap_->ResizeTarget(&desc);
	}
}

BOOL Graphics::GetFullScreenFlag()
{
	return IsFullScreenMode_;
}
void Graphics::SetFullScreenFlag(BOOL bFlag)
{
	IsFullScreenMode_ = bFlag;
}

bool Graphics::ChangeFullScreenMode_Swapchain()
{
	if (pSwap_.Get()) {
		BOOL IsScreenMode = FALSE;
		pSwap_->GetFullscreenState(&IsScreenMode, NULL);
		pSwap_->SetFullscreenState(!IsScreenMode, NULL);
		SetFullScreenFlag(IsScreenMode);
	}

	return IsFullScreenMode_;
}