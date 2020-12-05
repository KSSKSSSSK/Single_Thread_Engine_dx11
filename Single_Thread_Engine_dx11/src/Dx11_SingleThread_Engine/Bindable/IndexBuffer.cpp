#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Graphics& _gfx, const std::vector<unsigned short>& _indices)
	: count((UINT)_indices.size())
{
	// INFOMAN(_gfx);

	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = UINT(count * sizeof(unsigned short));
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = _indices.data();

	GetDevice(_gfx)->CreateBuffer(&ibd, &isd, pIndexBuffer.GetAddressOf());
}

void IndexBuffer::Bind(Graphics& _gfx) noexcept
{
	GetContext(_gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
}


UINT IndexBuffer::GetCount() const
{
	return count;
}