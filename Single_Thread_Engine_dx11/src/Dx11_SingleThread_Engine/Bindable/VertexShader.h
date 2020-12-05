#pragma once
#include "Bindable.h"

class VertexShader : public Bindable
{
protected:
	std::string		path_;
	std::wstring	w_path_;
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob_;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_;
public:
	// static std::shared_ptr<VertexShader> Resolve(Graphics& gfx, const std::string& path);
	static std::string GenerateUID(const std::string& path);

	void Bind(Graphics& gfx) noexcept override;
public:
	ID3DBlob*			GetBytecode()	const noexcept;
	const std::string	GetUID()		const noexcept override;
public:
	VertexShader(Graphics& gfx, const std::string& path);
};