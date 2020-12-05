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
	// dx11 dll ����
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
	BufferDesc		: �� ���� ��带 ��Ÿ���� DXGI_MODE_DESC ����
	Width			: �ػ��� �ʺ�
	Height			: ����� ����
	RefreshRate		: ����� (Hz����)�� ��Ÿ���� DXGI_RATIONAL����
	Numerator	: ������� ���ڰ� (���� 60)
	Denominator	: ������� �и� (���� 1)
	Format			: ����� ��� �ȼ� ������ ��Ÿ���� DXGI_FORMAT ����. (���� DXGI_FORMAT_R8G8B8A8_UNORM))
	SanlineOrdering : �ֻ缱 ������ ����� ����
	Scaling			: ũ�� ���� ���.
	SampleDesc		: ��Ƽ ���ø� �Ű� ������ ��Ÿ���� DXGI_SAMPLE_DESC ����
				(1�� �̻��� �ȼ����� ����, �����Ͽ� ���ο� �ȼ����� ���. �ε巴�� ��︮�� �ȼ����, ��Ƽ�ٸ��ƽ�)
	Count		: �ȼ� ������ ��Ƽ ���ø��� �� (�ȼ��� ���ø��� �ȼ��� ���� 1�̸� ��Ƽ ���ø� x)
	Quality		: �̹����� ǰ�� ������ ��ȯ�ȴ�. ǰ���� �������� ������ ���ϵȴ�.
				(��ȿ���� 0 ~ CheckMultisampleQualityLevels�Լ��� ��ȯ�� -1���� ��� ����)
	BufferUsage		: �� ���� ǥ�� ���� �� CPU �׼��� �ɼ��� ��Ÿ���� DXGI_USAGE ���� �� ���.
				�� ���۴� ���̴��� �Է��ϰų� ����Ÿ�� ������� ����� �� �ִ�.
	BufferCount		: ���� ü���� ���� ����.
	OutputWindow	: ��� â�� HWND �ڵ�. NULL�� �� �� ����.
	Windowed		: ������ ��� = True, Ǯ��ũ�� ��� = false
	SwapEffect		: �ø��� ���� ���� Ÿ�� ������ ������ ó���ϱ� ���� DXGI_SWAP_EFFECT ���� ��
					(�� ������ ������ ��� ���� ���� ����)
	Falgs			: ����ü�ε��� �ɼ��� ��Ÿ���� DXGI_SWAP_CHAIN_FLAG ������. */

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
	// �߰�
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
	//ALT + EnterŰ�� ��ȣ ��Ȳ�� Ȱ��ȭ �� ��Ȱ��ȭ �� �� �ִ�.
	// Ű ��ȯ ����� Ȱ��ȭ�� �����̶�� �⺻������ ���õǾ� �־� ������ �ڵ� �߰� �ʿ� x.
	/*		pGIFactory_.Get()->MakeWindowAssociation(hWnd_, flags);
	
		DXGI_MWA_NO_WINDOW_CHANGES �޽��� ť�� ����͸� ���� �ʰ� �ȴ�.
		DXGI_MWA_NO_ALT_ENTER Alt + Enter �۾��� �������� �ʵ��� �Ѵ�
		DXGI_MWA_NO_PRINT_SCREEN PrintScreenŰ�� �������� �ʵ��� �Ѵ�.	*/

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
	// ���� �����Ǿ� �ִ� ���� ����(�� ����)�� ������ ��� �Լ�.
	// ����̽� ������ DXGI_SWAP_CHAIN_DESC�� ����ü ������ ���� �� �ִ�.
	// ũ�⻩��� �״�� ���.
	pSwap_->GetDesc(&CurrentSD);
	pSwap_->ResizeBuffers(CurrentSD.BufferCount,	// ����ü���� ���� ��
		_width, _height,				// �� ������ ���ο� ��, ���� 0������ â�� Ŭ���̾�Ʈ ���� ���	
		CurrentSD.BufferDesc.Format,	// �� ������ ���ο� ����
		0);								// ���� ü���� ����� ��Ÿ���� �÷��� DXGI_SWAP_CHAIN_FLAG

	// ����� ������� ũ�⸦ ��´�.
	pSwap_->GetDesc(&AfterSD);
	width_ = AfterSD.BufferDesc.Width;
	height_ = AfterSD.BufferDesc.Height;


	if (FAILED(hr = SetRenderTargetView()))
	{
		return hr;
	}

	// ����Ʈ�� �����ϰ� �����Ѵ�.
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
		// ������ ũ�� �� �ػ�( ȭ���� ) ����
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