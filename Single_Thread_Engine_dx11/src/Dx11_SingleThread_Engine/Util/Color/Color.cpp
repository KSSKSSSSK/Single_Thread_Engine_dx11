#include "Color.h"

Color::Color()
	: red_(0), green_(0), blue_(0), alpha_(0), color_{ red_, green_, blue_, alpha_ }
{
}

Color::Color(float _red, float _green, float _blue, float _alpha)
	: red_(_red), green_(_green), blue_(_blue), alpha_(_alpha), color_{	red_, green_, blue_, alpha_ }
{
}
