#pragma once
#include "Window.h"
#include "Drawable/Box.h"
#include "Bindable/Bindable.h"
#include "common/Write.h"
#include "common/Timer.h"
#include "common/ScreenDebug.h"

class App
{
private:
	Window		wnd_;
	Timer		Timer_;
	std::vector<std::unique_ptr<Box>> boxes;
private:
	bool DoInit();
	bool DoFrame();
	bool DoRender();
	bool DoRelease();
public:
	int Go();
public:
	Timer*	GetTimer()	{ return &Timer_; }
	Window* GetWindow() { return &wnd_; }
public:
	App();
};

