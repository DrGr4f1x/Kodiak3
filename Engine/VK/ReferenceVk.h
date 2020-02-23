//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

namespace Kodiak
{

template <typename... Ts>
class Reference
{};


template <class U, typename T, typename... Ts>
class ReferenceInternal
{
public:
	static const std::shared_ptr<U>& Get(const Reference<T, Ts...>& ref)
	{
		return ref.m_tail.template Get<U>();
	}

	static void Set(Reference<T, Ts...>& ref, const std::shared_ptr<U>& ptr)
	{
		ref.m_tail.template Set<U>(ptr);
	}
};


template <typename T, typename... Ts>
class ReferenceInternal<T, T, Ts...>
{
public:
	static const std::shared_ptr<T>& Get(const Reference<T, Ts...>& ref)
	{
		return ref.m_head;
	}

	static void Set(Reference<T, Ts...>& ref, const std::shared_ptr<T>& ptr)
	{
		ref.m_head = ptr;
	}
};


template <typename T, typename... Ts>
class Reference<T, Ts...>
{
	template <class U, typename X, typename... Xs>
	friend class ReferenceInternal;

public:
	Reference(const std::shared_ptr<T>& t, std::shared_ptr<Ts>... ts)
		: m_head(t)
		, m_tail(ts...)
	{}

	template <class U>
	const std::shared_ptr<U>& Get() const
	{
		return ReferenceInternal<U, T, Ts...>::Get(*this);
	}

protected:
	template <class U>
	void Set(const std::shared_ptr<U>& ptr)
	{
		ReferenceInternal<U, T, Ts...>::Set(*this, ptr);
	}

private:
	std::shared_ptr<T>	m_head;
	Reference<Ts...>	m_tail;
};

} // namespace Kodiak