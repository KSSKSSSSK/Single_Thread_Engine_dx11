#pragma once 
#include <Windows.h>

class Timer
{
private:
	LARGE_INTEGER	Frequency_;			// �ʴ� ���ļ� 
	LARGE_INTEGER	Current_;			// ���� �ð� 
	LARGE_INTEGER	Frame_;				// ���� �������� �ð� üũ 
	float			SecPerFrame_;		// ������ ��� �ð� 

	LARGE_INTEGER	FPS_;				// FPS üũ Ÿ�̸�		
	int				FramePerSecond_;	// �ʴ� ������	
	int				FPSElapse_;			// �ʴ� ����� ������ 


	LARGE_INTEGER	Start_;				// �̺�Ʈ Ÿ�̸� ���� �ð�
	LARGE_INTEGER	Elapse_;			// �̺�Ʈ Ÿ�̸� ��� �ð�
	bool			Started_;			// �̺�Ʈ Ÿ�̸� �۵�����
	float			EventTime_;			// �̺�Ʈ Ÿ�̸� ������ ��� �ð� ����
public:
	bool	Init();
	bool	Frame();

	// �����ġ ����
	// ����� �ð��� ���� ���� �Լ���
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