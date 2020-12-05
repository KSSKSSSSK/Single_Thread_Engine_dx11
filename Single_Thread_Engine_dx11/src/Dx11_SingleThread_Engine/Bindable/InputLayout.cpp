#include "InputLayout.h"


InputLayout::InputLayout(Graphics& _gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& _layout, ID3DBlob* _pVSBytecode)
{
	HRESULT hr;

	hr = GetDevice(_gfx)->CreateInputLayout(
		_layout.data(), (UINT)std::size(_layout),
		_pVSBytecode->GetBufferPointer(),
		_pVSBytecode->GetBufferSize(),
		pInputLayout.GetAddressOf()
	);
}

void InputLayout::Bind(Graphics& _gfx) noexcept
{
	GetContext(_gfx)->IASetInputLayout(pInputLayout.Get());
}
