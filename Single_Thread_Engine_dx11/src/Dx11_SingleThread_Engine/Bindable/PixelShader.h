#pragma once
#include "Bindable.h"

class PixelShader : public Bindable
{
protected:
	std::string	 path_;
	std::wstring	w_path_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_;
public:
	void Bind(Graphics& gfx) noexcept override;
public:
	// static std::shared_ptr<PixelShader> Resolve(Graphics& gfx, const std::string& path);
	static	std::string GenerateUID(const std::string& path);
	const	std::string GetUID() const noexcept override;
public:
	PixelShader(Graphics& gfx, const std::string& path);
};