#include "Topology.h"

Topology::Topology(Graphics& _gfx, D3D11_PRIMITIVE_TOPOLOGY _type)
	: type(_type)
{

}

void Topology::Bind(Graphics& _gfx) noexcept
{
	GetContext(_gfx)->IASetPrimitiveTopology(type);
}
