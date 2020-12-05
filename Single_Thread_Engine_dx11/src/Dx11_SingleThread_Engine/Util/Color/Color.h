#pragma once


class Color
{
private:
	float red_;
	float green_;
	float blue_;
	float alpha_;

	float color_[4];
public:
	inline float get_red()
	{
		return red_;
	}
	inline float get_green()
	{
		return green_;
	}
	inline float get_blue()
	{
		return blue_;
	}
	inline float get_alpha()
	{
		return alpha_;
	}

	inline float* get_color()
	{
		return color_;
	}
public:
	Color();
	Color(float, float, float, float);
};