#include "PixelShader.h"

PixelShader::PixelShader(Graphics& _gfx, const std::string& _path)
	:path_(_path)
{
	w_path_.assign(path_.begin(), path_.end());
	// INFOMAN(gfx);

	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	D3DReadFileToBlob(w_path_.c_str(), &pBlob);
	GetDevice(_gfx)->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, pPixelShader_.GetAddressOf());
}

void PixelShader::Bind(Graphics& _gfx) noexcept
{
	// INFOMAN_NOHR(gfx);
	GetContext(_gfx)->PSSetShader(pPixelShader_.Get(), nullptr, 0u);
}

std::string PixelShader::GenerateUID(const std::string& path)
{
	using namespace std::string_literals;
	return typeid(PixelShader).name() + "#"s + path;
}
const std::string PixelShader::GetUID() const noexcept
{
	return GenerateUID(path_);
}
