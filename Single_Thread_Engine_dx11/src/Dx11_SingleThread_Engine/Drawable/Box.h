#pragma once
#include "DrawableBase.h"

class Box : public DrawableBase<Box>
{
private:
	// positional
	float r_;
	float roll_;
	float pitch_;
	float yaw_;
	float theta_;
	float phi_;
	float chi_;

	// speed (delta/s)
	float droll_;
	float dpitch_;
	float dyaw_;
	float dtheta_;
	float dphi_;
	float dchi_;
public:
	void Update(float _dt) noexcept override;
public:
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
public:
	Box(Graphics& _gfx, std::mt19937& _rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist);
};