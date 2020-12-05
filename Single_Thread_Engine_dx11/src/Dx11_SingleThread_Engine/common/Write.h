#pragma once
#include <vector>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
// #include <wincodec.h>
#include <wrl.h>
#include <string>

/* Direct2D�� �ϵ���� ���Ӹ�� ���. 2D, ��Ʈ��, �ؽ�Ʈ ������ ���ɰ� ǰ���� ����.
*/
class Write
{
private:
	HWND hWnd_;
	Microsoft::WRL::ComPtr<ID2D1RenderTarget>		pRT_;
	Microsoft::WRL::ComPtr<ID2D1Factory>			pD2DFactory_;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	pBlackBrush_;
	Microsoft::WRL::ComPtr<IDWriteFactory>			pDWriteFactory_;
	Microsoft::WRL::ComPtr<IDWriteTextFormat>		pTextFormat_;
	Microsoft::WRL::ComPtr<IDWriteTextLayout>		pTextLayout_;
	
	float	fontSize_;
	float	DPIScaleX_;
	float	DPIScaleY_;
	FLOAT	dpiX_;
	FLOAT	dpiY_;

	DWRITE_TEXT_ALIGNMENT		horizon_alignment_;
	DWRITE_PARAGRAPH_ALIGNMENT	vertical_alignment_;

	std::wstring		wszFontFamily_;
	std::wstring		wszText_;
	DWRITE_FONT_WEIGHT	fontWeight_;
	DWRITE_FONT_STYLE	fontStyle_;
	UINT32				TextLength_;
	BOOL				fontUnderline_;

	std::vector<std::wstring>	wszFonFamilys_;
public:
	// �ʱ�ȭ
	bool Set(HWND hWnd, int iWidth, int iHeight, IDXGISurface1* m_pSurface);
	bool Init();
	// ������
	bool			Begin();
	HRESULT			DrawText(RECT rc,const TCHAR* pText, D2D1::ColorF Color = D2D1::ColorF(1, 0, 0, 1));
	HRESULT			DrawText(D2D1_POINT_2F origin, D2D1::ColorF Color = D2D1::ColorF(1, 0, 0, 1));
	bool			End();
	// ��ü �Ҹ�
	bool			Release();
	// ��ü ���� �� �Ҹ�
	HRESULT			CreateDeviceIndependentResources();
	HRESULT			CreateDeviceResources(IDXGISurface1* m_pSurface);
	void			DiscardDeviceIndependentResources();
	void			DiscardDeviceResources();
public:
	void			GetLocalFontFamilys();
public:
	HRESULT			SetAlignment(DWRITE_TEXT_ALIGNMENT _horizon, DWRITE_PARAGRAPH_ALIGNMENT _vertical);
	HRESULT			SetText(D2D1_POINT_2F pos, const wchar_t* text, D2D1::ColorF Color);
	HRESULT			SetFont(wchar_t* fontFamily);
	HRESULT			SetFontSize(float size);
	HRESULT			SetBold(bool bold);
	HRESULT			SetItalic(bool italic);
	HRESULT			SetUnderline(bool underline);

	// ȭ�� ������ ����
	void			OnResize(UINT width, UINT height, IDXGISurface1* pSurface);
public:
	static Write& GetInstane()
	{
		static Write singleton;

		return singleton;
	}
public:
	// Microsoft::WRL::ComPtr<ID2D1RenderTarget>		pRT_;
	// Microsoft::WRL::ComPtr<ID2D1Factory>				pD2DFactory_;
	// Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		pBlackBrush_;
	// Microsoft::WRL::ComPtr<IDWriteFactory>			pDWriteFactory_;
	// Microsoft::WRL::ComPtr<IDWriteTextFormat>		pTextFormat_;
	// Microsoft::WRL::ComPtr<IDWriteTextLayout>		pTextLayout_;
	ID2D1RenderTarget* GetRenderTarget()	{ return pRT_.Get(); }
	IDWriteTextFormat*	GetTextFormat()		{ return pTextLayout_.Get();	}
public:
	Write();
	~Write();
};