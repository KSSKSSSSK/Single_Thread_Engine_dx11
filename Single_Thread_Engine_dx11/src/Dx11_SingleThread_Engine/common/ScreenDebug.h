#pragma once
#include <iostream>
#include <string>
#include <tchar.h>
#include "../common/Write.h"

class App;
class Timer;

class ScreenDebug
{
private:
	TCHAR	pTImerBuffer_[256];
	Timer*	pAppTimer_;
	App*	pApp_;
public:
	static ScreenDebug& GetInstance()
	{
		static ScreenDebug Singleton;
		return Singleton;
	}
public:
	bool Init(App* _App);
	bool Frame();
	bool Render();
	bool Release();
public:
	ScreenDebug();
	~ScreenDebug();
};