#pragma once
#include "Bindable.h"

class IndexBuffer : public Bindable
{
private:
	UINT count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
public:
	virtual void Bind(Graphics& _gfx) noexcept;
public:
	UINT GetCount() const;
public:
	IndexBuffer(Graphics& _gfx, const std::vector<unsigned short>& _indices);
};