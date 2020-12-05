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
    const wchar_t defaultText[] = L"돋움";
    wszFontFamily_ = defaultText;
    return true;
}

HRESULT Write::CreateDeviceIndependentResources()
{
    HRESULT hr;

    /* Direct2D factory 객체 생성
        Direct2D 리소스를 생성, 사용하기 위한 필수 객체. 첫번쨰 인자는 스레드 타입, 두번째는 리턴.
        여러 스레드에서 안전하게 액세스 할 수 있도록 자원의 동기화가 가능한 멀티 스레드가 지원된다.
    */
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, pD2DFactory_.GetAddressOf());
    
    pD2DFactory_->GetDesktopDpi(&dpiX_, &dpiY_);
    DPIScaleX_ = dpiX_ / 96.0f;
    DPIScaleY_ = dpiY_ / 96.0f;

    // 공유 DirectWrite factory 생성
    if (SUCCEEDED(hr))
    {
        /* DirectWrite 팩토리 객체 생성.
            1번째 인자는 객체를 공유하거나 분리하는 방법. 
            일반적으로 공유 팩토리 객체를 사용하며 내부 상태 정보가 
            프로세스 간에 공유되 메모리 사용량 감소될 수 있다.
            분리되면 다른 프로세스에 대한 영향을 받지 않기 때문에 독립적으로 사용 될 수 있다.
            
            2번째 인자는 DirectWrite 팩토리 식별 GUID값 
            3번째 인자는 객체 반환.
        */
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(pDWriteFactory_.GetAddressOf()));
    }

    // 폰트 사이즈, 포맷 설정 디폴트 설정으로 세팅 (사이즈 72)
    if (SUCCEEDED(hr))
    {
        hr = pDWriteFactory_->CreateTextFormat(
            wszFontFamily_.c_str(),             // 폰트 패밀리 이름
            NULL,                               // 글꼴 콜렉션(NULL이면 시스템 폰트 콜렉션 사용 설정)
            fontWeight_,                        // 글꼴 두께
            fontStyle_,                         // 글꼴 스타일
            DWRITE_FONT_STRETCH_NORMAL,         // 글꼴 확장
            fontSize_,                          // 글꼴 크기
            L"ko-kr",                           //L"ko-kr", L"en-us", L"ja-JP" ... (로케일 이름)
            pTextFormat_.GetAddressOf()         // 생성된 객체 반환 포인터
        );
    }
    return hr;
}

/* Direct2D 렌더타겟 생성
    Direct2D 렌더타겟 객체는 디바이스에 종속적인 객체이다.
    Direct3D 백 버퍼의 크기가 수정되거나 갱신 될 경우 소멸 후 다시 생성해야 한다.
    Direct2D 렌더타겟은 DirectWrite가 그려지는 버퍼를 의미. 보통 윈도우 클라 영역 및 Dx11의 백버퍼가 사용됨.
    Direct3D와 연동하여 텍스트를 출력하는 것이 목적이라 함수를 사용해야됨.
*/
HRESULT Write::CreateDeviceResources(IDXGISurface1* pSurface)
{
    HRESULT hr = S_OK;

    /* 스왑체인 객체로 IDXGISurface를 얻고 첫번째 인자로 넘겨야 DXGI 화면 렌더타겟을 생성 가능.
        DXGI의 화면과 일치하는 포맷을 사용해야 하지만, 렌더타겟 생성시. DXGI_FORMAT_UNKOWN 포맷을 지정하면
        자동으로 dxgi 포맷에 맞추어 생성된다 하지만 알파모드는 기본값이 없기 때문에
        포맷에 해당하는 알파모드를 선택하여 사용해야 한다.

        프리 멀티플라이된 알파모드는 RGBA 색상을 만들 떄 색의 알파값은 알파채널에 저장된다.

        ex) 불투명도가 60%의 적색 = (255, 0, 0, 255 * 0.6) = (255, 0, 0, 153)
        스트레이드 알파를 사용시 = (255 * 0.6, 0 * 0.6, 0 * 0.6, 255 * 0.6) = (153, 0, 0, 153)
        처럼 각 색상에 알파값이 곱해져 처리됨.
        D2D1_ALPHA_MODE_IGNORE는 알파 채널 무시하고 불투명 처리.
    */

    /* D2D1_RENDER_TARGET_PROPERTIES 객체
        렌더링 옵션을 설정. 하드웨어 및 소프트웨어 렌더링 여부, DPI(1인치에 출력 가능한 점(픽셀) 갯수)
        렌더타겟 픽셀 형식, 알파모드 선택 가능.

        1번째 인자값 : 하드웨어 렌더링으로 사용할지 여부 지정. 디폴트시 하드웨어 가능하면 하드웨어 아니면 소프트웨어
        Direct2D로 WIC비트맵을 로딩ㅇ하여 사용하는 경우 하드웨어가 지원되지 않는다.

        3, 4번째 인자값은 dpi값 화면이 확대 될 경우 출력된 스트링이 흐려지는 현상을 보정하기 위해 사용.
        5번째 인자값은 D2D1_RENDER_TARGET_USAGE 렌더 타겟의 처리 방법 및 GDI와 호환되는 렌더링 타겟의 생성 여부 지정.
        디폴트는 GDI와 호환이 없고 Direct3D 명령 스트림의 사용으로 렌더 타겟이 생성된다. 실패한 경우 피트맵 리모팅을 사용
        
        마지막 인자값은 D2D1_FEATURE_LEVEL은 하드웨어 랜더링에 필요한 최소한의 Direct3D 기능 수준을 설정
        기본값 (D2D1_FEATURE_LEVEL_DEFAULT)으로 하면 된다.
    */
    D2D1_RENDER_TARGET_PROPERTIES props = 
        D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        static_cast<float>(dpiX_),
        static_cast<float>(dpiY_));
    
    // DXGI 화면에 렌더링하는 렌더 타겟을 생성하며 반드시 Direct3D 디바이스를 생성시에 플래그를 지정해야함.
    hr = pD2DFactory_->CreateDxgiSurfaceRenderTarget(
        pSurface,                                           // 렌더 타겟의 렌더링 대상 IDXGISurface
        &props,                                             // 렌더링 모드
        pRT_.GetAddressOf());                                // 렌더 타겟 객체 반환

    // 텍스트 색상 지정하고 르러쉬 객체 생성. setcolor로 색상 교체 가능.
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

// 폰트 사이즈 설정
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

/* Dxgi 화면 포맷, 알파 모드 매칭 표

            포 맷                              알파 모드
    DXGI_FORMAT_B8G8R8A8_UNORM       D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_B8G8R8A8_UNORM       D2D1_ALPHA_MODE_IGNORE
    DXGI_FORMAT_R8G8B8A8_UNORM       D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_R8G8B8A8_UNORM       D2D1_ALPHA_MODE_IGNORE
    DXGI_FORMAT_A8_UNORM             D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_A8_UNORM             D2D1_ALPHA_MODE_STRAIGHT
    DXGI_FORMAT_UNKNOWN              D2D1_ALPHA_MODE_PREMULTIPLIED
    DXGI_FORMAT_UNKNOWN              D2D1_ALPHA_MODE_IGNORE 
*/


/* IDWriteTextFormat 객체 함수
    
    GetFlowDirection        텍스트 행의 방향을 얻는다.
    GetFontCollection       현재 글꼴 컬렉션을 얻는다.
    GetFontFamilyName       글꼴 패밀리 이름의 복사본을 얻는다.
    GetFontFamilyNameLength 글꼴 패밀리 이름의 길이를 얻는다.
    GetFontSize             글꼴의 높이(DIP)를 얻는다.
    GetFontStretch          텍스트의 글꼴 확장을 얻는다.
    GetFontStyle            텍스트의 글꼴 스타일을 얻는다.
    GetFontWeight           텍스트의 글꼴 두께를 얻는다.
    GetIncrementalTabStop   증분 탭 정지 위치를 얻는다.
    GetLineSpacing          여러 줄의 텍스트 단락에 설정된 간격 조정을 얻는다.
    GetLocaleName           텍스트에서 로케일 이름의 복사본을 얻는다.
    GetLocaleNameLength     텍스트에서 로케일 이름의 길이를 얻는다.
    GetParagraphAlignment   레이아웃 상자의 상단 및 하단을 기준으로 단락 맞춤 옵션을 얻는다..
    GetReadingDirection     단락 내의 텍스트의 현재의 읽기 방향을 얻는다.
    GetTextAlignment        레이아웃 상자의 끝과 끝을 기준으로 텍스트 배치 옵션을 얻는다.
    GetTrimming             레이아웃 상자에서 오버 플로우 텍스트 잘라 내기 옵션을 얻는다.
    GetWordWrapping         자동 줄 바꿈 옵션을 얻는다.
    SetFlowDirection        단락의 텍스트 방향을 설정한다..
    SetIncrementalTabStop   2 개의 인접한 탭 정지 사이의 고정 거리를 설정한다..
    SetLineSpacing          줄 간격을 설정한다..
    SetParagraphAlignment   레이아웃 상자의 상단 및 하단을 기준으로 단락 맞춤 옵션을 설정한다..
    SetReadingDirection     단락의 읽기 방향을 설정한다.
    SetTextAlignment        레이아웃 상자의 끝과 끝을 기준으로 단락 내의 텍스트 배치 옵션을 설정한다..
    SetTrimming             레이아웃 너비에서 오버 플로우 텍스트 잘라 내기 옵션을 설정한다.
    SetWordWrapping         자동 줄 바꿈 옵션을 설정한다..
*/