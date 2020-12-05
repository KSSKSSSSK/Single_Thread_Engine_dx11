#include "Bindable.h"

ID3D11Device* Bindable::GetDevice(Graphics& _gfx) noexcept
{
	return _gfx.GetDevice();
}
ID3D11DeviceContext* Bindable::GetContext(Graphics& _gfx) noexcept
{
	return _gfx.GetContext();
}