#pragma once
#include "Drawable.h"

template<class T>
class DrawableBase : public Drawable
{
private:
	static std::vector<std::unique_ptr<Bindable>> staticBinds;
private:
	const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept
	{
		return staticBinds;
	}
public:
	bool IsStaticInitialized() const noexcept
	{
		return !staticBinds.empty();
	}
	void AddStaticBind(std::unique_ptr<Bindable> _bind) noexcept
	{
		staticBinds.push_back(std::move(_bind));
	}
	void AddStaticIndexBuffer(std::unique_ptr<IndexBuffer> _ibuf) noexcept
	{
		pIndexBuffer = _ibuf.get();
		staticBinds.push_back(std::move(_ibuf));
	}
};

template<class T>
std::vector<std::unique_ptr<Bindable>> DrawableBase<T>::staticBinds;



