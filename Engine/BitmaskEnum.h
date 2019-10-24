//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <type_traits>

template<typename E>
struct EnableBitmaskOperators
{
	static const bool enable = false;
};


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E>::type
operator|(E lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E>::type
operator&(E lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E>::type
operator^(E lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	return static_cast<E>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E&>::type
operator|=(E& lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	lhs = static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
	return lhs;
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E&>::type
operator&=(E& lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	lhs = static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
	return lhs;
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, E&>::type
operator^=(E& lhs, E rhs)
{
	using underlying = std::underlying_type_t<E>;
	lhs = static_cast<E>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
	return lhs;
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, bool>::type
operator==(E lhs, int rhs)
{
	return static_cast<int>(lhs) == rhs;
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, bool>::type
operator==(int lhs, E rhs)
{
	return lhs == static_cast<int>(rhs);
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, bool>::type
operator!=(E lhs, int rhs)
{
	return static_cast<int>(lhs) != rhs;
}


template <typename E>
typename std::enable_if<EnableBitmaskOperators<E>::enable, bool>::type
operator!=(int lhs, E rhs)
{
	return lhs != static_cast<int>(rhs);
}