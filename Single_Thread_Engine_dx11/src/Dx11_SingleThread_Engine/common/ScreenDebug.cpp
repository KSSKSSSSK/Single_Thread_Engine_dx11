#include "ScreenDebug.h"
#include "../common/Timer.h"
#include "../App.h"
#include "../Window.h"

ScreenDebug::ScreenDebug()
{
	memset(pTImerBuffer_, 0, sizeof(TCHAR) * 256);
}
ScreenDebug::~ScreenDebug()
{

}

bool ScreenDebug::Init(App* _App)
{
	pApp_ = _App;
	pAppTimer_ = pApp_->GetTimer();

	IDXGISurface1* pBackBuffer = NULL;
	HRESULT hr = pApp_->GetWindow()->Get_gfx().GetSwap()->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	Write::GetInstane().Set(pApp_->GetWindow()->Get_Handle(), pApp_->GetWindow()->GetWidth(), pApp_->GetWindow()->GetHeight(), pBackBuffer);
	if (pBackBuffer)	pBackBuffer->Release();

	return true;
}
bool ScreenDebug::Frame()
{
	memset(pTImerBuffer_, 0, sizeof(TCHAR) * 256);

	_stprintf_s(pTImerBuffer_, _T("FPS:%d SecPerFrame:%10.5f Elapsed : %10.2f"), pAppTimer_->GetFPS(), pAppTimer_->GetSPF(), pAppTimer_->GetElapsedTime());
	
	return true;
}
bool ScreenDebug::Render()
{
	Write::GetInstane().Begin();
		Write::GetInstane().GetTextFormat()->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		Write::GetInstane().GetTextFormat()->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		RECT rc1 = { 0,0, pApp_->GetWindow()->GetWidth(), pApp_->GetWindow()->GetHeight() };
		Write::GetInstane().DrawText(rc1, pTImerBuffer_, D2D1::ColorF(1, 1, 1, 0.5));

		RECT rc2 = { 400,300, 800, 600 };
		//write_.DrawText(rc2, L"X축 좌측 정렬, Y축 상단 정렬", D2D1::ColorF(1, 1, 1, 0.5));
	Write::GetInstane().End();

	return true;
}
bool ScreenDebug::Release()
{
	Write::GetInstane().Release();

	return true;
}