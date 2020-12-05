#pragma once
#include "Bindable.h"
#include "../Graphics/Graphics.h"
#include "../objInfo/Vertex.h"

class BindVertexBuffer : public Bindable
{
private:
	static std::string GenerateUID_(const std::string& _tag);
protected:
	UINT stride_;
	std::string tag_;
	VertexLayout layout_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer_;
public:
	template<typename...Ignore>
	static std::string GenerateUID(const std::string& _tag, Ignore&&...ignore)
	{
		return GenerateUID_(_tag);
	}
	// static std::shared_ptr<BindVertexBuffer> Resolve(Graphics& _gfx, const std::string& _tag, const VertexBuffer& _vBuf);

	void Bind(Graphics& _gfx) noexcept override;
public:
	const std::string	GetUID()	const noexcept override;
	const VertexLayout& GetLayout() const noexcept;
public:
	BindVertexBuffer(Graphics& _gfx, const std::string& _tag, const VertexBuffer& _vbuf);
	BindVertexBuffer(Graphics& _gfx, const VertexBuffer& _vbuf);

	template<class T>
	BindVertexBuffer(Graphics& _gfx, const std::vector<T>& vertices)
		: stride_(sizeof(T))
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(T) * vertices.size());
		bd.StructureByteStride = sizeof(T);
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();

		GetDevice(_gfx)->CreateBuffer(&bd, &sd, pVertexBuffer_.GetAddressOf());
	}
};