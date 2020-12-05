#pragma once
#include "../KSWin.h"
#include "../WindowUtil/commin.h"
#include "../Util/Color/Color.h"
#include "../common/define.h"
#include "Device/Enumeration.h"
#include "../common/Write.h"

#include <vector>
#include <cmath>
#include <memory>
#include <random>

class Graphics
{
private:
	Enumeration				Enumeration_;


	D3D_FEATURE_LEVEL		feature_level_;		// D3 특성레벨 속성값
	D3D_DRIVER_TYPE			driver_type_;		// 디아비스 타입 속성값
	DXGI_SWAP_CHAIN_DESC	swapchain_desc_;	// 스왑체인 속성값
	D3D11_VIEWPORT			viewport_desc_;		// 뷰 포트 속성값

	int16_t					width_;
	int16_t					height_;
	Color					clear_color_;
	DirectX::XMMATRIX		projection_;
	RECT					ClientRect_;		// 클라이언트 영역	
	HWND                    hWnd_;
	BOOL					IsFullScreenMode_;	// 풀스크린 디바이스 여부판단

	// 유저모드와 커널모드르를 연결해 주는 단계로써. 
	// kernel Mode Driver 및 System Hardware와 통신하는 것이 목적.
	// 하드웨어 관련 작업이 일로옴 (어댑터 열거, 감마컨트롤, 풀스크린 및 윈도우 전환 작업 관리..)
	Microsoft::WRL::ComPtr<IDXGIFactory>			pGIFactory_;// DXGI 객체

	Microsoft::WRL::ComPtr<ID3D11Device>			pDevice_;	// 디바이스 객체
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>		pContext_;	// 디바이스 컨텍스트 객체
	Microsoft::WRL::ComPtr<IDXGISwapChain>			pSwap_;		// 스왑체인 객체
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	pTarget_;	// 메인 렌더타겟 뷰
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	pDSV_;
public:
	void EndFrame();
	void DrawIndexed(UINT _count) noexcept;
	void ClearBuffer(float _red = 0, float _green = 0 , float _blue = 0) noexcept;

	/////////////////////////////////////////////////
	void	DrawTriangle(float _angle, float _x, float _y)
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(3.0f, 3.0f, 0.0f, 0.0f);
		auto result = DirectX::XMVector3Transform(v, DirectX::XMMatrixScaling(1.5f, 0.0f, 0.0f));
		auto xx = DirectX::XMVectorGetX(result);

		HRESULT hr;

		struct Vertex
		{
			struct
			{
				float x;
				float y;
				float z;
			} pos;
		};

		const Vertex vertices[] =
		{
			{ -1.0f, -1.0f, 0.0f },
			{ -1.0f,  1.0f, 0.0f },
			{  1.0f, -1.0f, 0.0f },
			{  1.0f,  1.0f, 0.0f },
			{ -1.0f, -1.0f,  1.0f },
			{  1.0f, -1.0f,  1.0f },
			{ -1.0f,  1.0f,  1.0f },
			{  1.0f,  1.0f,  1.0f }
		};


		// const Vertex vertices[] =
		// {
		// 	{0.0f, 0.0f, 0.0f},
		// 	{0.0f, 1.0f, 0.0f},
		// 	{1.0f, 0.0f, 0.0f}
		// };

		////////////////////////////////////////////// vb

		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = sizeof(vertices);
		bd.StructureByteStride = sizeof(Vertex);
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0u;

		pDevice_->CreateBuffer(&bd, &sd, pVertexBuffer.GetAddressOf());

		////////////////////////////////////////////// ib

		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;

		const unsigned short indices[] =
		{
			0, 2, 1, 2, 3, 1,
			1, 3, 5, 3, 7, 5,
			2, 6, 3, 3, 6, 7,
			4, 5, 7, 4, 7, 6,
			0, 4, 2, 2, 4, 6,
			0, 1, 4, 1, 5, 4
		};

		D3D11_BUFFER_DESC bid = {};
		bid.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bid.Usage = D3D11_USAGE_DEFAULT;
		bid.CPUAccessFlags = 0u;
		bid.MiscFlags = 0u;
		bid.ByteWidth = sizeof(indices);
		bid.StructureByteStride = sizeof(unsigned short);
		D3D11_SUBRESOURCE_DATA sid = {};
		sid.pSysMem = indices;

		pDevice_->CreateBuffer(&bid, &sid, pIndexBuffer.GetAddressOf());


		////////////////////////////////////////////// csb


		struct ConstantBuffer
		{
			struct
			{
				DirectX::XMMATRIX transform;
			} transformation;
		};

		struct ConstantBuffer2
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			}face_colors[6];
		};

		const ConstantBuffer cb =
		{
			{
				DirectX::XMMatrixTranspose(
					DirectX::XMMatrixRotationZ(0.0f) * //_angle / 0.1f) * 
					DirectX::XMMatrixRotationX(_angle / 180.0f) *
					DirectX::XMMatrixScaling(0.5f, 0.5f, 0.5f) * 
					DirectX::XMMatrixTranslation(_x, _y, 4.0f) * 
					DirectX::XMMatrixPerspectiveFovLH(1.0f, 3.0f / 4.0f,0.5f, 10.0f)
				)
}
		};
		const ConstantBuffer2 cb2 =
		{
			{
				{1.0f, 0.0f, 1.0f},
				{1.0f,0.0f,0.0f},
				{0.0f,1.0f,0.0f},
				{0.0f,0.0f,1.0f},
				{1.0f,1.0f,0.0f},
				{0.0f,1.0f,1.0f},
			}
		};


		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;

		D3D11_BUFFER_DESC cbd = {};
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(cb);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cb;

		pDevice_->CreateBuffer(&cbd, &csd, pConstantBuffer.GetAddressOf());

		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer2;

		D3D11_BUFFER_DESC cbd2 = {};
		cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd2.Usage = D3D11_USAGE_DYNAMIC;
		cbd2.CPUAccessFlags = 0u;
		cbd2.MiscFlags = 0u;
		cbd2.ByteWidth = sizeof(cb2);
		cbd2.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd2 = {};
		csd2.pSysMem = &cb2;

		pDevice_->CreateBuffer(&cbd2, &csd2, pConstantBuffer2.GetAddressOf());

		////////////////////////////////////////////// csb

		D3D11_VIEWPORT vp;
		vp.Width = 800;
		vp.Height = 600;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		Microsoft::WRL::ComPtr<ID3D11VertexShader>	pVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	pPixelShader;

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		D3DReadFileToBlob(L"VertexShader.cso", pBlob.GetAddressOf());

		hr = pDevice_->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());



		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			// 숫자가 비트 단위, 바이트 아님
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			//{ "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		pDevice_->CreateInputLayout(
			ied, (UINT)std::size(ied),
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			pInputLayout.GetAddressOf()
		);

		D3DReadFileToBlob(L"PixelShader.cso", pBlob.GetAddressOf());
		hr = pDevice_->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());

		pContext_->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
		//pContext_->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
		// pContext_->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
		// pContext_->PSSetConstantBuffers(0u, 1u, pConstantBuffer2.GetAddressOf());
		pContext_->VSSetShader(pVertexShader.Get(), nullptr, 0u);
		pContext_->PSSetShader(pPixelShader.Get(), nullptr, 0u);
		pContext_->IASetInputLayout(pInputLayout.Get());
		// bind depth stensil view to OM
		pContext_->OMSetRenderTargets(1u, pTarget_.GetAddressOf(), pDSV_.Get());

		pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		pContext_->RSSetViewports(1u, &vp);
		pContext_->Draw(8, 0);
		//pContext_->DrawIndexed((UINT)sizeof(indices), 0u, 0u);
	}
	HRESULT ResizeDevice(UINT _width, UINT _height);
public:
	HRESULT CreateDevice();
	HRESULT	CreateSwapChain(HWND _hWnd, UINT _width, UINT _height, BOOL _IsFullScreen = false);

	HRESULT CreateDeviceAndSwapchain(HWND _hWnd, UINT _width, UINT _height);
	HRESULT CreateGIFactory();
	HRESULT CreateRenderTargetView();

	HRESULT	SetViewport();
	HRESULT SetRenderTargetView();
	void SetProjection(DirectX::FXMMATRIX  _proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;

	void ChangeResolution(int16_t width, int16_t height);

	BOOL GetFullScreenFlag();
	void SetFullScreenFlag(BOOL bFlag);
	bool ChangeFullScreenMode_Swapchain();
public:
	ID3D11Device*			GetDevice()		{ return pDevice_.Get();	}
	ID3D11DeviceContext*	GetContext()	{ return pContext_.Get();	}
	IDXGISwapChain*			GetSwap()		{ return pSwap_.Get();		}
public:
	Graphics& operator=(const Graphics&) = delete;
	Graphics(const Graphics&) = delete;
public:
	Graphics(HWND _hWnd, int16_t _width, int16_t _hight);
	~Graphics();
};

