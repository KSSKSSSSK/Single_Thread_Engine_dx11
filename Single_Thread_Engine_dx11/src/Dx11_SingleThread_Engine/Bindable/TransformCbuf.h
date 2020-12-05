#pragma once
#include "Bindable.h"
#include "../Drawable/Drawable.h"
#include "../Bindable/BindConstantBuffer.h"

class TransformCbuf : public Bindable
{
	/*
protected:
	struct Transforms
	{
		DirectX::XMMATRIX model_;
		DirectX::XMMATRIX modelView_;
		DirectX::XMMATRIX modelViewProj_;
	};
protected:
	Transforms GetTransforms(Graphics& _gfx) noexcept;
	void UpdateBindImpl(Graphics& _gfx, const Transforms& _tf) noexcept;
public:
	void Bind(Graphics& _gfx) noexcept override;
	void InitializeParentReference(const Drawable& _parent) noexcept;//  override;
	// std::unique_ptr<CloningBindable> Clone() const noexcept override;
public:
	TransformCbuf(Graphics& _gfx, UINT _slot = 0u);
private:
	static std::unique_ptr<BindVertexConstantBuffer<Transforms>> pVcbuf;
	const Drawable* pParent = nullptr;
	*/
public:
	TransformCbuf(Graphics& _gfx, const Drawable& _parent);
	void Bind(Graphics& _gfx) noexcept override;
private:
	BindVertexConstantBuffer<DirectX::XMMATRIX> vcbuf_;
	const Drawable& parent_;
};