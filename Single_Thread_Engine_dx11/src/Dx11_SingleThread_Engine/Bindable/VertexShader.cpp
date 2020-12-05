#include "VertexShader.h"

VertexShader::VertexShader(Graphics& _gfx, const std::string& _path)
	:path_(_path)
{
	w_path_.assign(path_.begin(), path_.end());
// 	INFOMAN(gfx);

	D3DReadFileToBlob(w_path_.c_str(), pBytecodeBlob_.GetAddressOf());

	GetDevice(_gfx)->CreateVertexShader(
		pBytecodeBlob_->GetBufferPointer(),
		pBytecodeBlob_->GetBufferSize(),
		nullptr,
		pVertexShader_.GetAddressOf()
	);
}

void VertexShader::Bind(Graphics& _gfx) noexcept
{
	//INFOMAN_NOHR(gfx);
	GetContext(_gfx)->VSSetShader(pVertexShader_.Get(), nullptr, 0u);
}


std::string VertexShader::GenerateUID(const std::string& _path)
{
	using namespace std::string_literals;
	return typeid(VertexShader).name() + "#"s + _path;
}
ID3DBlob* VertexShader::GetBytecode() const noexcept
{
	return pBytecodeBlob_.Get();
}
const std::string VertexShader::GetUID() const noexcept
{
	return GenerateUID(path_);
}