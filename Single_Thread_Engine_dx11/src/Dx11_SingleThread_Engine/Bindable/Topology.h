#pragma once
#include "Bindable.h"

class Topology : public Bindable
{
protected:
	D3D11_PRIMITIVE_TOPOLOGY type;
public:
	void Bind(Graphics& _gfx) noexcept override;
public:
	Topology(Graphics& _gfx, D3D11_PRIMITIVE_TOPOLOGY _type);
};