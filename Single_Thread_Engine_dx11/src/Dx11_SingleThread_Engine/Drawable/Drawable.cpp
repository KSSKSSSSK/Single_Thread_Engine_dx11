#include "Drawable.h"
#include "../Bindable/IndexBuffer.h"

void Drawable::Draw(Graphics& _gfx) const noexcept
{
	for (auto& b : binds)
	{
		b->Bind(_gfx);
	}
	for (auto& b : GetStaticBinds())
	{
		b->Bind(_gfx);
	}

	_gfx.DrawIndexed(pIndexBuffer->GetCount());
}
void Drawable::AddBind(std::unique_ptr<Bindable> _bind) noexcept
{
	binds.push_back(std::move(_bind));
}
void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> _ibuf) noexcept
{
	pIndexBuffer = _ibuf.get();
	binds.push_back(std::move(_ibuf));
}