#include "Timer.h"

Timer::Timer()
	: Started_(false),
	FramePerSecond_(0),  
	FPSElapse_(0),
	SecPerFrame_(0.0f),
	EventTime_(0.0f)
{
	// �ý����� ���ļ� �������� ��� �´�. �ð��� ǥ���� ��� ���ؼ� ���.
	// ���ļ� �� �ȹٲ�. (CPU Hz ��� ����)
	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency_);
}
Timer::~Timer()
{

}

bool Timer::Init()
{
	// �� ���� �ʱ�ȭ
	QueryPerformanceCounter(&Frame_);

	// ���ػ� Ÿ�̸� �������� �Ǵ�
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
	// 1�������� �ð� üũ 
	// ���� �ð��� ã�´�. 
	QueryPerformanceCounter(&Current_);
	SecPerFrame_ = static_cast<float>(Current_.QuadPart - Frame_.QuadPart) / static_cast<float>(Frequency_.QuadPart);

	// 1�ʴ� ������ üũ 	
	// (Current_.LowPart - FPS_.LowPart) / Frequency_.LowPart) 1�ʴ� ������
	if (((Current_.LowPart - FPS_.LowPart) / Frequency_.LowPart) >= 1) {
		FramePerSecond_ = FPSElapse_;
		FPSElapse_ = 0;
		FPS_ = Current_;
	}

	// ������ �� üũ
	FPSElapse_++;

	// �����Ӵ� �� ���� ����
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