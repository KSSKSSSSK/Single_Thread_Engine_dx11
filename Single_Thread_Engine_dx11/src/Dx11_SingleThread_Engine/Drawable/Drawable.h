#pragma once
#include "../Graphics/Graphics.h"
#include <DirectXMath.h>

class Bindable;
class IndexBuffer;

class Drawable
{
	template<class T>
	friend class DrawableBase;
private:
	const IndexBuffer* pIndexBuffer = nullptr;
	std::vector<std::unique_ptr<Bindable>> binds;
private:
	virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;
public:
	void Draw(Graphics& _gfx) const noexcept;
	virtual void Update(float _dt) noexcept = 0;
	void AddBind(std::unique_ptr<Bindable> _bind) noexcept;
	void AddIndexBuffer(std::unique_ptr<IndexBuffer> _ibuf) noexcept;
public:
	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
public:
	Drawable() = default;
	Drawable(const Drawable&) = delete;
};