#pragma once
#include "../Graphics/Graphics.h"

class Bindable
{
protected:
	static ID3D11Device*		GetDevice(Graphics& _gfx)		noexcept;
	static ID3D11DeviceContext*	GetContext(Graphics& _gfx)		noexcept;
	// static DxgiInfoManager&		GetInfoManager(Graphics& _gfx)	noexcept;
public:
	virtual void Bind(Graphics& _gfx) noexcept = 0;
public:
	virtual const std::string GetUID() const noexcept
	{
		assert(false);
		return "";
	}
public:
	virtual ~Bindable() = default;
};