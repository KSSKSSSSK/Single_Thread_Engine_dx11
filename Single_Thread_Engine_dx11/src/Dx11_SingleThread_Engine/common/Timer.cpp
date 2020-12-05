#include "Timer.h"

Timer::Timer()
	: Started_(false),
	FramePerSecond_(0),  
	FPSElapse_(0),
	SecPerFrame_(0.0f),
	EventTime_(0.0f)
{
	// 시스템의 주파수 변동폭을 얻어 온다. 시간의 표준을 잡기 위해서 사용.
	// 주파수 값 안바뀜. (CPU Hz 라고 생각)
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency_);
}
Timer::~Timer()
{

}

bool Timer::Init()
{
	// 비교 변수 초기화
	QueryPerformanceCounter(&Frame_);

	// 고해상도 타이머 지원여부 판단
	if (Frame_.QuadPart == 0) {
		return false;
	}

	FPS_ = Frame_;
	FramePerSecond_ = 0;
	FPSElapse_ = 0;
	SecPerFrame_ = 0.0f;

	Start();
	return true;
}
bool Timer::Frame()
{
	// 1프레임의 시간 체크 
	// 현재 시간을 찾는다. 
	QueryPerformanceCounter(&Current_);
	SecPerFrame_ = static_cast<float>(Current_.QuadPart - Frame_.QuadPart) / static_cast<float>(Frequency_.QuadPart);

	// 1초당 프레임 체크 	
	// (Current_.LowPart - FPS_.LowPart) / Frequency_.LowPart) 1초당 프레임
	if (((Current_.LowPart - FPS_.LowPart) / Frequency_.LowPart) >= 1) {
		FramePerSecond_ = FPSElapse_;
		FPSElapse_ = 0;
		FPS_ = Current_;
	}

	// 프레임 수 체크
	FPSElapse_++;

	// 프레임당 비교 변수 스왑
	Frame_ = Current_;

	return true;
}


void Timer::Start()
{
	Started_ = true;
	EventTime_ = 0.0f;
	QueryPerformanceCounter((LARGE_INTEGER*)&Start_);
}
void Timer::Stop()
{
	GetElapsedTime();
	Started_ = false;
}
void Timer::Reset()
{
	Started_ = false;
	EventTime_ = 0.0f;
	memset(&Start_, 0, sizeof(LARGE_INTEGER));
	memset(&Elapse_, 0, sizeof(LARGE_INTEGER));
}
float Timer::GetElapsedTime()
{
	if (Started_) {
		QueryPerformanceCounter((LARGE_INTEGER*)&Elapse_);
		EventTime_ = static_cast<float>(Elapse_.LowPart - Start_.LowPart) / static_cast<float>(Frequency_.LowPart);
	}

	return EventTime_;
}

bool Timer::IsStarted() const
{
	return Started_;
}


// FPS
int	Timer::GetFPS()
{
	return FramePerSecond_;
}
float Timer::GetSPF()
{
	return SecPerFrame_;
}