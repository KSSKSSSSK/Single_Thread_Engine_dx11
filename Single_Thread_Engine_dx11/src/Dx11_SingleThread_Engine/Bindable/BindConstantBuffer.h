#pragma once
#include "Bindable.h"

template<typename T>
class BindConstantBuffer : public Bindable
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
public:
	void Update(Graphics& _gfx, const T& _consts)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		GetContext(_gfx)->Map(pConstantBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );

		memcpy(msr.pData, &_consts, sizeof(_consts));
		GetContext(_gfx)->Unmap(pConstantBuffer.Get(), 0u);
	}
public:
	BindConstantBuffer(Graphics& _gfx, const T& _consts)
	{
		D3D11_BUFFER_DESC cbd = {};
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(_consts);
		cbd.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &_consts;

		GetDevice(_gfx)->CreateBuffer(&cbd, &csd, pConstantBuffer.GetAddressOf());
	}
	BindConstantBuffer(Graphics& _gfx)
	{
		D3D11_BUFFER_DESC cbd = {};
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(T);
		cbd.StructureByteStride = 0u;

		GetDevice(_gfx)->CreateBuffer(&cbd, nullptr, pConstantBuffer.GetAddressOf());
	}
};


template<typename T>
class BindVertexConstantBuffer : public BindConstantBuffer<T>
{
	using BindConstantBuffer<T>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using BindConstantBuffer<T>::BindConstantBuffer;
	void Bind(Graphics& _gfx) noexcept override
	{
		GetContext(_gfx)->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
	}
};

template<typename T>
class BindPixelConstantBuffer : public BindConstantBuffer<T>
{
	using BindConstantBuffer<T>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using BindConstantBuffer<T>::BindConstantBuffer;
	void Bind(Graphics& _gfx) noexcept override
	{
		GetContext(_gfx)->PSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
	}
};