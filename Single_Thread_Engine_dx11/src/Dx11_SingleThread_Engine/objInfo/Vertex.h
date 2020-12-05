#pragma once
#include <vector>
#include <assert.h>
#include "../Graphics/Graphics.h"

#define DVTX_ELEMENT_AI_EXTRACTOR(member) \
static SysType Extract( const aiMesh& mesh,size_t i ) noexcept \
 {return *reinterpret_cast<const SysType*>(&mesh.member[i]);}

class aiMesh;

class VertexLayout
{
public:
	enum ElementType
	{
#define X(el) el,
		LAYOUT_ELEMENT_TYPES
#undef X
	};
public:
	size_t Size() const noexcept;
	size_t GetElementCount() const noexcept;
	/*

	template<ElementType> struct Map;
	template<> struct Map<Position2D>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P2";
		DVTX_ELEMENT_AI_EXTRACTOR(mVertices)
	};
	template<> struct Map<Position3D>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Position";
		static constexpr const char* code = "P3";
		DVTX_ELEMENT_AI_EXTRACTOR(mVertices)
	};
	template<> struct Map<Texture2D>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "Texcoord";
		static constexpr const char* code = "T2";
		DVTX_ELEMENT_AI_EXTRACTOR(mTextureCoords[0])
	};
	template<> struct Map<Normal>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Normal";
		static constexpr const char* code = "N";
		DVTX_ELEMENT_AI_EXTRACTOR(mNormals)
	};
	template<> struct Map<Tangent>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Tangent";
		static constexpr const char* code = "Nt";
		DVTX_ELEMENT_AI_EXTRACTOR(mTangents)
	};
	template<> struct Map<Bitangent>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Bitangent";
		static constexpr const char* code = "Nb";
		DVTX_ELEMENT_AI_EXTRACTOR(mBitangents)
	};
	template<> struct Map<Float3Color>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C3";
		DVTX_ELEMENT_AI_EXTRACTOR(mColors[0])
	};
	template<> struct Map<Float4Color>
	{
		using SysType = DirectX::XMFLOAT4;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C4";
		DVTX_ELEMENT_AI_EXTRACTOR(mColors[0])
	};
	template<> struct Map<BGRAColor>
	{
		using SysType = ::BGRAColor;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static constexpr const char* semantic = "Color";
		static constexpr const char* code = "C8";
		DVTX_ELEMENT_AI_EXTRACTOR(mColors[0])
	};
	template<> struct Map<Count>
	{
		using SysType = long double;
		static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
		static constexpr const char* semantic = "!INVALID!";
		static constexpr const char* code = "!INV!";
		DVTX_ELEMENT_AI_EXTRACTOR(mFaces)
	};

	template<template<VertexLayout::ElementType> class F, typename... Args>
	static constexpr auto Bridge(VertexLayout::ElementType type, Args&&... args) noxnd
	{
		switch (type)
		{
#define X(el) case VertexLayout::el: return F<VertexLayout::el>::Exec( std::forward<Args>( args )... );
			LAYOUT_ELEMENT_TYPES
#undef X
		}
		assert("Invalid element type" && false);
		return F<VertexLayout::Count>::Exec(std::forward<Args>(args)...);
	}


public:
	template<ElementType Type>
	const Element& Resolve() const noxnd
	{
		for (auto& e : elements)
		{
			if (e.GetType() == Type)
			{
				return e;
			}
		}
		assert("Could not resolve element type" && false);
		return elements.front();
	}
	const Element& ResolveByIndex(size_t i) const noxnd;
	VertexLayout& Append(ElementType type) noxnd;
	size_t Size() const noxnd;
	size_t GetElementCount() const noexcept;
	std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const noxnd;
	std::string GetCode() const noxnd;
	bool Has(ElementType type) const noexcept;

public:
	size_t Size() const noexcept;
	*/

	class Element
	{
	private:
		VertexLayout::ElementType type_;
		size_t offset_;
	public:
		size_t GetOffsetAfter() const noexcept;
		size_t GetOffset() const;
		size_t Size() const noexcept;
		static constexpr size_t SizeOf(VertexLayout::ElementType _type) noexcept;
		VertexLayout::ElementType GetType() const noexcept;
		D3D11_INPUT_ELEMENT_DESC GetDesc() const noexcept;
		const char* GetCode() const noexcept;
	public:
		Element(VertexLayout::ElementType _type, size_t _offset);
	};
private:
	std::vector<Element> elements;
};

////////////////////////////////////////////////////////////////////////

class Vertex
{
	friend class VertexBuffer;
private:
	char* pData_;
	const VertexLayout& layout_;
public:
	Vertex(char* _pData, const VertexLayout& _layout) noexcept;
};

class ConstVertex
{
private:
	Vertex vertex_;
public:
	ConstVertex(const Vertex& v) noexcept;
	// template<VertexLayout::ElementType Type>
	// const auto& Attr() const noexcept
	// {
	// 	return const_cast<Vertex&>(vertex).Attr<Type>();
	// }
};

////////////////////////////////////////////////////////////////////////

class VertexBuffer
{
private:
	std::vector<char> buffer_;
	VertexLayout layout_;
public:
	template<typename...Params>
	void EmplaceBack(Params&&... _params) noexcept
	{
		assert(sizeof...(_params) == layout_.GetElementCount() && "Param count doesn't match number of vertex elements");
		buffer_.resize(buffer_.size() + layout_.Size());
		Back().SetAttributeByIndex(0u, std::forward<Params>(_params)...);
	}
public:
	const char* GetData() const noexcept;
	const VertexLayout& GetLayout() const noexcept;
	void Resize(size_t _newSize) noexcept;
	size_t Size() const noexcept;
	size_t SizeBytes() const noexcept;
public:
	Vertex Back() noexcept;
	Vertex Front() noexcept;
	Vertex operator[](size_t _idx) noexcept;
	ConstVertex Back() const noexcept;
	ConstVertex Front() const noexcept;
	ConstVertex operator[](size_t _idx) const noexcept;
public:
	VertexBuffer(VertexLayout _layout, size_t _size = 0u) noexcept;
	VertexBuffer(VertexLayout _layout, const aiMesh& _mesh);
};