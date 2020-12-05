#include "Vertex.h"




// VertexLayout
////////////////////////////////////////////////////////////////////////

size_t VertexLayout::Size() const noexcept
{
	return elements.empty() ? 0u : elements.back().GetOffsetAfter();
}
size_t VertexLayout::GetElementCount() const noexcept
{
	return elements.size();
}


VertexLayout::Element::Element(ElementType _type, size_t _offset)
	:
	type_(_type),
	offset_(_offset)
{

}
size_t VertexLayout::Element::GetOffsetAfter() const noexcept
{
	return offset_ + Size();
}
size_t VertexLayout::Element::GetOffset() const
{
	return offset_;
}
size_t VertexLayout::Element::Size() const noexcept
{
	return SizeOf(type_);
}
constexpr size_t VertexLayout::Element::SizeOf(ElementType _type) noexcept
{
	return 0;
}
VertexLayout::ElementType VertexLayout::Element::GetType() const noexcept
{
	return type_;
}
D3D11_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noexcept
{
	return D3D11_INPUT_ELEMENT_DESC();
}
const char* VertexLayout::Element::GetCode() const noexcept
{
	return nullptr;
}

// Vertex
////////////////////////////////////////////////////////////////////////

Vertex::Vertex(char* _pData, const VertexLayout& _layout) noexcept
	: layout_(_layout)
{

}

ConstVertex::ConstVertex(const Vertex& _vertex) noexcept
	: vertex_(_vertex)
{}

// VertexBuffer
////////////////////////////////////////////////////////////////////////


VertexBuffer::VertexBuffer(VertexLayout _layout, size_t _size) noexcept
	: layout_(std::move(_layout))
{
	Resize(_size);
}
VertexBuffer::VertexBuffer(VertexLayout _layout, const aiMesh& _mesh)
	: layout_(std::move(_layout))
{
	// Resize(mesh.mNumVertices);
	// 
	// for (size_t i = 0, end = layout_.GetElementCount(); i < end; i++)
	// {
	// 	VertexLayout::Bridge<AttributeAiMeshFill>(layout.ResolveByIndex(i).GetType(), this, mesh);
	// }
}

void VertexBuffer::Resize(size_t _newSize) noexcept
{
	const auto size = Size();

	if (size < _newSize)
	{
		buffer_.resize(buffer_.size() + layout_.Size() * (_newSize - size));
	}
}
size_t VertexBuffer::Size() const noexcept
{
	return buffer_.size() / layout_.Size();
}
size_t VertexBuffer::SizeBytes() const noexcept
{
	return buffer_.size();
}
Vertex VertexBuffer::Back() noexcept
{
	assert(buffer_.size() != 0u);
	return Vertex{ buffer_.data() + buffer_.size() - layout_.Size(), layout_ };
}
Vertex VertexBuffer::Front() noexcept
{
	assert(buffer_.size() != 0u);
	return Vertex{ buffer_.data(), layout_ };
}
Vertex VertexBuffer::operator[](size_t _idx) noexcept
{
	assert(_idx < Size());
	return Vertex{ buffer_.data() + layout_.Size() * _idx, layout_ };
}

ConstVertex VertexBuffer::Back() const noexcept
{
	return const_cast<VertexBuffer*>(this)->Back();
}
ConstVertex VertexBuffer::Front() const noexcept
{
	return const_cast<VertexBuffer*>(this)->Front();
}
ConstVertex VertexBuffer::operator[](size_t i) const noexcept
{
	return const_cast<VertexBuffer&>(*this)[i];
}

const char* VertexBuffer::GetData() const noexcept
{
	return buffer_.data();
}
const VertexLayout& VertexBuffer::GetLayout() const noexcept
{
	return layout_;
}