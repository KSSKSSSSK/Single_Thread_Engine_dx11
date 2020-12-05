#pragma once
#include "Bindable.h"

class InputLayout : public Bindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
public:
	void Bind(Graphics& _gfx) noexcept override;
public:
	InputLayout(Graphics& _gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& _layout, ID3DBlob* _pVSBytecode);
};