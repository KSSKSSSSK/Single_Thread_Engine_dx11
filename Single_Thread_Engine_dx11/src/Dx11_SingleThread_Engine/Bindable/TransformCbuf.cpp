#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(Graphics& _gfx, const Drawable& _parent)
	:
	vcbuf_(_gfx),
	parent_(_parent)
{}

void TransformCbuf::Bind(Graphics& _gfx) noexcept
{
	vcbuf_.Update(_gfx,
		DirectX::XMMatrixTranspose(
			parent_.GetTransformXM() * _gfx.GetProjection()
		));
	vcbuf_.Bind(_gfx);
}

/*
TransformCbuf::TransformCbuf(Graphics& _gfx, UINT _slot)
{
	if (!pVcbuf)
	{
		pVcbuf = std::make_unique<BindVertexConstantBuffer<Transforms>>(_gfx, _slot);
	}
}

TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& _gfx) noexcept
{
	assert(pParent != nullptr);
	const auto model = pParent->GetTransformXM();
	const auto modelView = model * _gfx.GetCamera();
	return {
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(modelView),
		DirectX::XMMatrixTranspose(
			modelView *
			_gfx.GetProjection()
		)
	};
}
void TransformCbuf::UpdateBindImpl(Graphics& _gfx, const Transforms& _tf) noexcept
{
	assert(pParent != nullptr);
	pVcbuf->Update(_gfx, _tf);
	pVcbuf->Bind(_gfx);
}
void TransformCbuf::Bind(Graphics& _gfx) noexcept
{
	UpdateBindImpl(_gfx, GetTransforms(_gfx));
}
void TransformCbuf::InitializeParentReference(const Drawable& _parent) noexcept
{
	pParent = &_parent;
}

*/