#include "Write.h"
#include "../common/macro.h"

Write::Write()
{
    hWnd_ = NULL;
    TextLength_ = 0;
    fontWeight_ = DWRITE_FONT_WEIGHT_NORMAL;
    fontStyle_ = DWRITE_FONT_STYLE_NORMAL;
    fontUnderline_ = FALSE;
    fontSize_ = 18.0f;


    horizon_alignment_ = DWRITE_TEXT_ALIGNMENT_CENTER;
    vertical_alignment_ = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

}
Write::~Write()
{
}

bool Write::Set(HWND hWnd, int iWidth, int iHeight, IDXGISurface1* pSurface)
{
    HRESULT hr;
    if (!Init())
    {
        return false;
    }
    hWnd_ = hWnd;

    V(CreateDeviceIndependentResources());
    V(CreateDeviceResources(pSurface));
    GetLocalFontFamilys();

    return true;
}
void Write::GetLocalFontFamilys()
{
    HRESULT hr;

    IDWriteFontCollection* pFontCollection = NULL;

    // Get the system font collection.
    if (SUCCEEDED(hr)) {
        hr = pDWriteFactory_->GetSystemFontCollection(&pFontCollection);
    }

    UINT32 familyCount = 0;

    // Get the number of font families in the collection.
    if (SUCCEEDED(hr)) {
        familyCount = pFontCollection->GetFontFamilyCount();
    }

    for (UINT32 i = 0; i < familyCount; ++i) {
        IDWriteFontFamily* pFontFamily = NULL;
        IDWriteLocalizedStrings* pFamilyNames = NULL;
        UINT32 index = 0;
        BOOL exists = false;

        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

        // Get the font family.
        if (SUCCEEDED(hr)) {
            hr = pFontCollection->GetFontFamily(i, &pFontFamily);
        }
        // Get a list of localized strings for the family name.
        if (SUCCEEDED(hr)) {
            hr = pFontFamily->GetFamilyNames(&pFamilyNames);
        }

        if (SUCCEEDED(hr)) {
            // Get the default locale for this user.
            int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

            // If the default locale is returned, find that locale name, otherwise use "en-us".
            if (defaultLocaleSuccess)
            {
                hr = pFamilyNames->FindLocaleName(localeName, &index, &exists);
            }
            if (SUCCEEDED(hr) && !exists) // if the above find did not find a match, retry with US English
            {
                hr = pFamilyNames->FindLocaleName(L"ko-kr", &index, &exists);
            }
        }

        // If the specified locale doesn't exist, select the first on the list.
        if (!exists)
            index = 0;

        UINT32 length = 0;

        // Get the string length.
        if (SUCCEEDED(hr)) {
            hr = pFamilyNames->GetStringLength(index, &length);
        }

        // Allocate a string big enough to hold the name.
        wchar_t* name = new (std::nothrow) wchar_t[length + 1];
        std::wstring name1;

        if (name == NULL) {
            hr = E_OUTOFMEMORY;
        }

        // Get the family name.
        if (SUCCEEDED(hr)) {
            hr = pFamilyNames->GetString(index, name, length + 1);
            name1 = name;

            if (name) {
                delete name;
                name == nullptr;
            }
            wszFonFamilys_.emplace_back(name1);
        }
    }
}

bool Write::Release()
{
    wszText_.clear();
    wszFontFamily_.clear();
    DiscardDeviceIndependentResources();
    DiscardDeviceResources();
    return true;
}


bool Write::Init()
{
    const wchar_t defaultText[] = L"����";
    wszFontFamily_ = defaultText;
    return true;
}

HRESULT Write::CreateDeviceIndependentResources()
{
    HRESULT hr;

    /* Direct2D factory ��ü ����
        Direct2D ���ҽ��� ����, ����ϱ� ���� �ʼ� ��ü. ù���� ���ڴ� ������ Ÿ��, �ι�°�� ����.
        ���� �����忡�� �����ϰ� �׼��� �� �� �ֵ��� �ڿ��� ����ȭ�� ������ ��Ƽ �����尡 �����ȴ�.
    */
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory_.GetAddressOf());
    
    pD2DFactory_->GetDesktopDpi(&dpiX_, &dpiY_);
    DPIScaleX_ = dpiX_ / 96.0f;
    DPIScaleY_ = dpiY_ / 96.0f;

    // ���� DirectWrite factory ����
    if (SUCCEEDED(hr))
    {
        /* DirectWrite ���丮 ��ü ����.
            1��° ���ڴ� ��ü�� �����ϰų� �и��ϴ� ���. 
            �Ϲ������� ���� ���丮 ��ü�� ����ϸ� ���� ���� ������ 
            ���μ��� ���� ������ �޸� ��뷮 ���ҵ� �� �ִ�.
            �и��Ǹ� �ٸ� ���μ����� ���� ������ ���� �ʱ� ������ ���������� ��� �� �� �ִ�.
            
            2��° ���ڴ� DirectWrite ���丮 �ĺ� GUID�� 
            3��° ���ڴ� ��ü ��ȯ.
        */
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(pDWriteFactory_.GetAddressOf()));
    }

    // ��Ʈ ������, ���� ���� ����Ʈ �������� ���� (������ 72)
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextFormat(
            wszFontFamily_.c_str(),             // ��Ʈ �йи� �̸�
            NULL,                               // �۲� �ݷ���(NULL�̸� �ý��� ��Ʈ �ݷ��� ��� ����)
            fontWeight_,                        // �۲� �β�
            fontStyle_,                         // �۲� ��Ÿ��
            DWRITE_FONT_STRETCH_NORMAL,         // �۲� Ȯ��
            fontSize_,                          // �۲� ũ��
            L"ko-kr",                           //L"ko-kr", L"en-us", L"ja-JP" ... (������ �̸�)
            pTextFormat_.GetAddressOf()         // ������ ��ü ��ȯ ������
        );
    }
    return hr;
}

/* Direct2D ����Ÿ�� ����
    Direct2D ����Ÿ�� ��ü�� ����̽��� �������� ��ü�̴�.
    Direct3D �� ������ ũ�Ⱑ �����ǰų� ���� �� ��� �Ҹ� �� �ٽ� �����ؾ� �Ѵ�.
    Direct2D ����Ÿ���� DirectWrite�� �׷����� ���۸� �ǹ�. ���� ������ Ŭ�� ���� �� Dx11�� ����۰� ����.
    Direct3D�� �����Ͽ� �ؽ�Ʈ�� ����ϴ� ���� �����̶� �Լ��� ����ؾߵ�.
*/
HRESULT Write::CreateDeviceResources(IDXGISurface1* pSurface)
{
    HRESULT hr = S_OK;

    /* ����ü�� ��ü�� IDXGISurface�� ��� ù��° ���ڷ� �Ѱܾ� DXGI ȭ�� ����Ÿ���� ���� ����.
        DXGI�� ȭ��� ��ġ�ϴ� ������ ����ؾ� ������, ����Ÿ�� ������. DXGI_FORMAT_UNKOWN ������ �����ϸ�
        �ڵ����� dxgi ���˿� ���߾� �����ȴ� ������ ���ĸ��� �⺻���� ���� ������
        ���˿� �ش��ϴ� ���ĸ�带 �����Ͽ� ����ؾ� �Ѵ�.

        ���� ��Ƽ�ö��̵� ���ĸ��� RGBA ������ ���� �� ���� ���İ��� ����ä�ο� ����ȴ�.

        ex) �������� 60%�� ���� = (255, 0, 0, 255 * 0.6) = (255, 0, 0, 153)
        ��Ʈ���̵� ���ĸ� ���� = (255 * 0.6, 0 * 0.6, 0 * 0.6, 255 * 0.6) = (153, 0, 0, 153)
        ó�� �� ���� ���İ��� ������ ó����.
        D2D1_ALPHA_MODE_IGNORE�� ���� ä�� �����ϰ� ������ ó��.
    */

    /* D2D1_RENDER_TARGET_PROPERTIES ��ü
        ������ �ɼ��� ����. �ϵ���� �� ����Ʈ���� ������ ����, DPI(1��ġ�� ��� ������ ��(�ȼ�) ����)
        ����Ÿ�� �ȼ� ����, ���ĸ�� ���� ����.

        1��° ���ڰ� : �ϵ���� ���������� ������� ���� ����. ����Ʈ�� �ϵ���� �����ϸ� �ϵ���� �ƴϸ� ����Ʈ����
        Direct2D�� WIC��Ʈ���� �ε����Ͽ� ����ϴ� ��� �ϵ��� �������� �ʴ´�.

        3, 4��° ���ڰ��� dpi�� ȭ���� Ȯ�� �� ��� ��µ� ��Ʈ���� ������� ������ �����ϱ� ���� ���.
        5��° ���ڰ��� D2D1_RENDER_TARGET_USAGE ���� Ÿ���� ó�� ��� �� GDI�� ȣȯ�Ǵ� ������ Ÿ���� ���� ���� ����.
        ����Ʈ�� GDI�� ȣȯ�� ���� Direct3D ��� ��Ʈ���� ������� ���� Ÿ���� �����ȴ�. ������ ��� ��Ʈ�� �������� ���
        
        ������ ���ڰ��� D2D1_FEATURE_LEVEL�� �ϵ���� �������� �ʿ��� �ּ����� Direct3D ��� ������ ����
        �⺻�� (D2D1_FEATURE_LEVEL_DEFAULT)���� �ϸ� �ȴ�.
    */
    D2D1_RENDER_TARGET_PROPERTIES props = 
        D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        static_cast<float>(dpiX_),
        static_cast<float>(dpiY_));
    
    // DXGI ȭ�鿡 �������ϴ� ���� Ÿ���� �����ϸ� �ݵ�� Direct3D ����̽��� �����ÿ� �÷��׸� �����ؾ���.
    hr = pD2DFactory_->CreateDxgiSurfaceRenderTarget(
        pSurface,                                           // ���� Ÿ���� ������ ��� IDXGISurface
        &props,                                             // ������ ���
        pRT_.GetAddressOf());                                // ���� Ÿ�� ��ü ��ȯ

    // �ؽ�Ʈ ���� �����ϰ� ������ ��ü ����. setcolor�� ���� ��ü ����.
    V_RETURN(pRT_->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow),
        pBlackBrush_.GetAddressOf()));

    return S_OK;
}


/*
    TextDialog::SetFont                                            *
*                                                                 *
*  This method sets the font family for the entire range of text  *
*  in the text layout object then redraws the text by using the   *
*  DrawD2DContent method of this class.                            */

HRESULT Write::SetFont(wchar_t* fontFamily)
{
    HRESULT hr;

    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    hr = pTextLayout_->SetFontFamilyName(fontFamily, textRange);

    if (SUCCEEDED(hr))
    {
        wszFontFamily_.clear();
        wszFontFamily_ = fontFamily;
    }
    return hr;
}

/*
    TextDialog::SetFontSize                                        *
*                                                                 *
*  This method sets the font size for the entire range of text    *
*  in the text layout object then redraws the text by using the   *
*  DrawD2DContent method of this class. */

// ��Ʈ ������ ����
HRESULT Write::SetFontSize(float size)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    hr = pTextLayout_->SetFontSize(size, textRange);

    if (SUCCEEDED(hr))
    {
        fontSize_ = size;
    }
    return hr;
}

HRESULT Write::SetAlignment(DWRITE_TEXT_ALIGNMENT _horizon, DWRITE_PARAGRAPH_ALIGNMENT _vertical)
{
    HRESULT hr;

    horizon_alignment_ = _horizon;
    vertical_alignment_ = _vertical;

    hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
     
    return hr;
}
HRESULT Write::SetText(D2D1_POINT_2F size, const wchar_t* text, D2D1::ColorF Color)
{
    HRESULT hr = S_OK;
    // Store the text for later.
    TextLength_ = (UINT32)wcslen(text);
    wszText_.clear();
    wszText_ = text;

    // Release any existing layout object.
     // pSafeRelease(&m_pTextLayout);
    // Create a text layout using the text format.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextLayout(
            wszText_.c_str(),           // The string to be laid out and formatted.
            TextLength_,       // The length of the string.
            pTextFormat_.Get(),       // The text format to apply to the string (contains font information, etc).
            size.x,         // The width of the layout box.
            size.y,        // The height of the layout box.
            pTextLayout_.GetAddressOf()       // The IDWriteTextLayout interface pointer.
        );
    }

    // Declare a typography pointer.
    IDWriteTypography* pTypography = NULL;
    // Create a typography interface object.
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTypography(&pTypography);
    }

    // Set the stylistic set.
    DWRITE_FONT_FEATURE fontFeature = { DWRITE_FONT_FEATURE_TAG_STYLISTIC_SET_7,1 };
    if (SUCCEEDED(hr))
    {
        hr = pTypography->AddFontFeature(fontFeature);
    }

    // Set the typography for the entire string.
    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    if (SUCCEEDED(hr))
    {
        hr = pTextLayout_->SetTypography(pTypography, textRange);
    }

    // Set the underline for the entire string.
    if (SUCCEEDED(hr))
    {
        hr = pTextLayout_->SetUnderline(fontUnderline_, textRange);
    }
 //    SafeRelease(&pTypography);
    return hr;
}
/******************************************************************
*                                                                 *
*  TextDialog::SetBold                                            *
*                                                                 *
*  This method toggles the weight to and from bold for the        *
*  entire range of text in the text layout object then redraws    *
*  the text by using the DrawD2DContent method of this class.      *
*                                                                 *
******************************************************************/

HRESULT Write::SetBold(bool bold)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    if (bold)
    {
        fontWeight_ = DWRITE_FONT_WEIGHT_BOLD;
    }
    else
    {
        fontWeight_ = DWRITE_FONT_WEIGHT_NORMAL;
    }

    // Set the font weight.
    hr = pTextLayout_->SetFontWeight(fontWeight_, textRange);
    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetItalic                                          *
*                                                                 *
*  This method toggles the style to and from italic for the       *
*  entire range of text in the text layout object then redraws    *
*  the text by using the DrawD2DContent method of this class.      *
*                                                                 *
******************************************************************/

HRESULT Write::SetItalic(bool italic)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    if (italic)
    {
        fontStyle_ = DWRITE_FONT_STYLE_ITALIC;
    }
    else
    {
        fontStyle_ = DWRITE_FONT_STYLE_NORMAL;
    }

    // Set the font style.
    hr = pTextLayout_->SetFontStyle(fontStyle_, textRange);
    return hr;
}

/******************************************************************
*                                                                 *
*  TextDialog::SetUnderline                                       *
*                                                                 *
*  This method sets toggles the underline for the entire range of *
*  text in the text layout object then redraws the text by using  *
*  the DrawD2DContent method of this class.                          *
*                                                                 *
******************************************************************/

HRESULT Write::SetUnderline(bool underline)
{
    HRESULT hr;
    DWRITE_TEXT_RANGE textRange = { 0, TextLength_ };

    fontUnderline_ = underline;

    // Set the underline.
    hr = pTextLayout_->SetUnderline(fontUnderline_, textRange);
    return hr;
}
void Write::DiscardDeviceIndependentResources()
{
    if (pRT_) {
        pRT_->Release();
        pRT_ = nullptr;
    }
}
HRESULT Write::DrawText(RECT rc, const TCHAR* pText, D2D1::ColorF Color)
{
    if (pRT_ && pBlackBrush_.Get())
    {
        D2D1_RECT_F layoutRect = D2D1::RectF(
            static_cast<FLOAT>(rc.left) / DPIScaleX_,
            static_cast<FLOAT>(rc.top) / DPIScaleY_,
            static_cast<FLOAT>(rc.right) / DPIScaleX_,
            static_cast<FLOAT>(rc.bottom) / DPIScaleY_);
        pBlackBrush_->SetColor(Color);
        pRT_->DrawText(pText, wcslen(pText), pTextFormat_.Get(), layoutRect, pBlackBrush_.Get());
    }
    return S_OK;
}
HRESULT Write::DrawText(D2D1_POINT_2F pos, D2D1::ColorF Color)
{
    D2D1_POINT_2F origin = D2D1::Point2F(
        static_cast<FLOAT>(pos.x / DPIScaleX_),
        static_cast<FLOAT>(pos.y / DPIScaleY_));
    pBlackBrush_->SetColor(Color);
    pRT_->DrawTextLayout(origin, pTextLayout_.Get(), pBlackBrush_.Get());
    return S_OK;
}
bool Write::Begin()
{
    if (pRT_) {
        pRT_->BeginDraw();
        /*	D2D_MATRIX_3X2_F mat;
            mat._11 = cosf(fTime); mat._12 = -sinf(fTime);
            mat._21 = sinf(fTime); mat._22 = cosf(fTime);
            mat._31 = 400; mat._32=300;*/
        pRT_->SetTransform(D2D1::IdentityMatrix());
        // m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
    }
    return true;
}
bool Write::End()
{
    if (pRT_ && FAILED(pRT_->EndDraw())) {
        return false;
    }
    return true;
}

void Write::DiscardDeviceResources()
{
   // SafeRelease(&pRT_);
   // SafeRelease(&pBlackBrush_);
}

void Write::OnResize(UINT width, UINT height, IDXGISurface1* pSurface)
{
    DiscardDeviceResources();
    CreateDeviceResources(pSurface);
    SetText(D2D1::Point2F(width, height), L"TBasisSample!", D2D1::ColorF(1, 1, 1, 1));
}

/* Dxgi ȭ�� ����, ���� ��� ��Ī ǥ

            �� ��                              ���� ���
    DXGI_FORMAT_B8G8R8A8_UNORM       D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_B8G8R8A8_UNORM       D2D1_ALPHA_MODE_IGNORE
    DXGI_FORMAT_R8G8B8A8_UNORM       D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_R8G8B8A8_UNORM       D2D1_ALPHA_MODE_IGNORE
    DXGI_FORMAT_A8_UNORM             D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_A8_UNORM             D2D1_ALPHA_MODE_STRAIGHT
    DXGI_FORMAT_UNKNOWN              D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_UNKNOWN              D2D1_ALPHA_MODE_IGNORE 
*/


/* IDWriteTextFormat ��ü �Լ�
    
    GetFlowDirection        �ؽ�Ʈ ���� ������ ��´�.
    GetFontCollection       ���� �۲� �÷����� ��´�.
    GetFontFamilyName       �۲� �йи� �̸��� ���纻�� ��´�.
    GetFontFamilyNameLength �۲� �йи� �̸��� ���̸� ��´�.
    GetFontSize             �۲��� ����(DIP)�� ��´�.
    GetFontStretch          �ؽ�Ʈ�� �۲� Ȯ���� ��´�.
    GetFontStyle            �ؽ�Ʈ�� �۲� ��Ÿ���� ��´�.
    GetFontWeight           �ؽ�Ʈ�� �۲� �β��� ��´�.
    GetIncrementalTabStop   ���� �� ���� ��ġ�� ��´�.
    GetLineSpacing          ���� ���� �ؽ�Ʈ �ܶ��� ������ ���� ������ ��´�.
    GetLocaleName           �ؽ�Ʈ���� ������ �̸��� ���纻�� ��´�.
    GetLocaleNameLength     �ؽ�Ʈ���� ������ �̸��� ���̸� ��´�.
    GetParagraphAlignment   ���̾ƿ� ������ ��� �� �ϴ��� �������� �ܶ� ���� �ɼ��� ��´�..
    GetReadingDirection     �ܶ� ���� �ؽ�Ʈ�� ������ �б� ������ ��´�.
    GetTextAlignment        ���̾ƿ� ������ ���� ���� �������� �ؽ�Ʈ ��ġ �ɼ��� ��´�.
    GetTrimming             ���̾ƿ� ���ڿ��� ���� �÷ο� �ؽ�Ʈ �߶� ���� �ɼ��� ��´�.
    GetWordWrapping         �ڵ� �� �ٲ� �ɼ��� ��´�.
    SetFlowDirection        �ܶ��� �ؽ�Ʈ ������ �����Ѵ�..
    SetIncrementalTabStop   2 ���� ������ �� ���� ������ ���� �Ÿ��� �����Ѵ�..
    SetLineSpacing          �� ������ �����Ѵ�..
    SetParagraphAlignment   ���̾ƿ� ������ ��� �� �ϴ��� �������� �ܶ� ���� �ɼ��� �����Ѵ�..
    SetReadingDirection     �ܶ��� �б� ������ �����Ѵ�.
    SetTextAlignment        ���̾ƿ� ������ ���� ���� �������� �ܶ� ���� �ؽ�Ʈ ��ġ �ɼ��� �����Ѵ�..
    SetTrimming             ���̾ƿ� �ʺ񿡼� ���� �÷ο� �ؽ�Ʈ �߶� ���� �ɼ��� �����Ѵ�.
    SetWordWrapping         �ڵ� �� �ٲ� �ɼ��� �����Ѵ�..
*/