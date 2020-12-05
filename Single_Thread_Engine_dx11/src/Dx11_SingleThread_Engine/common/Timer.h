#pragma once 
#include <Windows.h>

class Timer
{
private:
	LARGE_INTEGER	Frequency_;			// 초당 주파수 
	LARGE_INTEGER	Current_;			// 현재 시간 
	LARGE_INTEGER	Frame_;				// 이전 프레임의 시간 체크 
	float			SecPerFrame_;		// 프레임 경과 시간 

	LARGE_INTEGER	FPS_;				// FPS 체크 타이머		
	int				FramePerSecond_;	// 초당 프레임	
	int				FPSElapse_;			// 초당 경과된 프레임 


	LARGE_INTEGER	Start_;				// 이벤트 타이머 시작 시간
	LARGE_INTEGER	Elapse_;			// 이베트 타이머 경과 시간
	bool			Started_;			// 이벤트 타이머 작동여부
	float			EventTime_;			// 이벤트 타이머 정지시 경과 시간 저장
public:
	bool	Init();
	bool	Frame();

	// 스톱워치 느낌
	// 경과한 시간에 대한 지원 함수들
	void	Reset();
	void	Start();
	void	Stop();
	bool	IsStarted() const;
	float	GetElapsedTime();

	// FPS
	int		GetFPS();
	float	GetSPF();
public:
	Timer();
	~Timer();
};