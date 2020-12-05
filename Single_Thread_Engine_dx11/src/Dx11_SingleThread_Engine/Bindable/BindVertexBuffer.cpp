#include "BindVertexBuffer.h"

BindVertexBuffer::BindVertexBuffer(Graphics& _gfx, const VertexBuffer& _vbuf)
	: BindVertexBuffer(_gfx, "?", _vbuf)
{

}
BindVertexBuffer::BindVertexBuffer(Graphics& _gfx, const std::string& _tag, const VertexBuffer& _vbuf)
	: stride_((UINT)_vbuf.GetLayout().Size()),
	tag_(_tag),
	layout_(_vbuf.GetLayout())
{
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = UINT(_vbuf.SizeBytes());
	bd.StructureByteStride = stride_;
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = _vbuf.GetData();
	
	GetDevice(_gfx)->CreateBuffer(&bd, &sd, pVertexBuffer_.GetAddressOf());
}

std::string BindVertexBuffer::GenerateUID_(const std::string& _tag)
{
	using namespace std::string_literals;
	return typeid(VertexBuffer).name() + "#"s + _tag;
}

void BindVertexBuffer::Bind(Graphics& _gfx) noexcept
{
	const UINT offset = 0u;
	// INFOMAN_NOHR(_gfx);
	GetContext(_gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer_.GetAddressOf(), &stride_, &offset);
}
const std::string BindVertexBuffer::GetUID()	const noexcept
{
	return GenerateUID(tag_);
}
const VertexLayout& BindVertexBuffer::GetLayout() const noexcept
{
	return layout_;
}


/*
std::shared_ptr<BindVertexBuffer> BindVertexBuffer::Resolve(Graphics& _gfx, const std::string& _tag, const VertexBuffer& _vBuf)
{
	assert(_tag != "?");
	return Codex::Resolve<VertexBuffer>(_gfx, _tag, _vbuf);
}
*/