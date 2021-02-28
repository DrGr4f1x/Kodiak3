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

#include "Graphics\InputLayout.h"
#include "Color.h"

#pragma pack(push, 4)

namespace Kodiak
{

struct EmptyVertex
{
	// Position accessors (dummy)
	XMFLOAT3 GetPosition() const { return XMFLOAT3(); }
	void SetPosition(float x, float y, float z) {}
	void SetPosition(XMFLOAT3) {}
	void SetPosition(const Math::Vector3&) {}

	// Normal accessors (dummy)
	XMFLOAT3 GetNormal() const { return XMFLOAT3(); }
	void SetNormal(float nx, float ny, float nz) {}
	void SetNormal(XMFLOAT3 normal) {}
	void SetNormal(const Math::Vector3& normal) {}

	// Color accessors (dummy)
	XMFLOAT4 GetColor() const { return XMFLOAT4(); }
	void SetColor(float r, float g, float b, float a) {}
	void SetColor(XMFLOAT4 color) {}
	void SetColor(Color color) {}

	// Texcoord accessors (dummy)
	XMFLOAT2 GetTexcoord() const { return XMFLOAT2{}; }
	void SetTexcoord(float u, float v) {}
	void SetTexcoord(XMFLOAT2 texcoord) {}
};


#define IMPL_POSITION \
	XMFLOAT3 GetPosition() const { return position; } \
	void SetPosition(float x, float y, float z) \
	{ \
		position.x = x; \
		position.y = y; \
		position.z = z; \
	}\
	void SetPosition(XMFLOAT3 position) \
	{ \
		this->position = position; \
	} \
	void SetPosition(const Math::Vector3& position) \
	{ \
		XMStoreFloat3(&this->position, position); \
	}


#define IMPL_COLOR \
	XMFLOAT4 GetColor() const { return color; } \
	void SetColor(float r, float g, float b, float a) \
	{ \
		color.x = r; \
		color.y = g; \
		color.z = b; \
		color.w = a; \
	} \
	void SetColor(XMFLOAT4 color) \
	{ \
		this->color = color; \
	} \
	void SetColor(Color color) \
	{ \
		this->color.x = color.R(); \
		this->color.y = color.G(); \
		this->color.z = color.B(); \
		this->color.w = color.A(); \
	}


#define IMPL_TEXCOORD \
	XMFLOAT2 GetTexcoord() const { return texcoord; } \
	void SetTexcoord(float u, float v) \
	{ \
		texcoord.x = u; \
		texcoord.y = v; \
	} \
	void SetTexcoord(XMFLOAT2 texcoord) \
	{ \
		this->texcoord = texcoord; \
	}


#define IMPL_NORMAL \
	XMFLOAT3 GetNormal() const { return normal; } \
	void SetNormal(float nx, float ny, float nz) \
	{ \
		normal.x = nx; \
		normal.y = ny; \
		normal.z = nz; \
	} \
	void SetNormal(XMFLOAT3 normal) \
	{ \
		this->normal = normal; \
	} \
	void SetNormal(const Math::Vector3& normal) \
	{ \
		XMStoreFloat3(&this->normal, normal); \
	}


struct VertexPosition : public EmptyVertex
{
	VertexPosition() = default;

	VertexPosition(const VertexPosition&) = default;
	VertexPosition& operator=(const VertexPosition&) = default;

	VertexPosition(VertexPosition&&) = default;
	VertexPosition& operator=(VertexPosition&&) = default;

	IMPL_POSITION

	XMFLOAT3 position;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ false };
	static constexpr bool hasColor{ false };
	static constexpr bool hasTexcoord{ false };
};

static_assert(sizeof(VertexPosition) == 3 * sizeof(float), "Size mismatch in VertexPosition!");


struct VertexPositionColor : public EmptyVertex
{
	VertexPositionColor() = default;

	VertexPositionColor(const VertexPositionColor&) = default;
	VertexPositionColor& operator=(const VertexPositionColor&) = default;

	VertexPositionColor(VertexPositionColor&&) = default;
	VertexPositionColor& operator=(VertexPositionColor&&) = default;

	IMPL_POSITION
	IMPL_COLOR

	XMFLOAT3 position;
	XMFLOAT4 color;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ false };
	static constexpr bool hasColor{ true };
	static constexpr bool hasTexcoord{ false };
};

static_assert(sizeof(VertexPositionColor) == 7 * sizeof(float), "Size mismatch in VertexPositionColor!");


struct VertexPositionTexture : public EmptyVertex
{
	VertexPositionTexture() = default;

	VertexPositionTexture(const VertexPositionTexture&) = default;
	VertexPositionTexture& operator=(const VertexPositionTexture&) = default;

	VertexPositionTexture(VertexPositionTexture&&) = default;
	VertexPositionTexture& operator=(VertexPositionTexture&&) = default;

	IMPL_POSITION
	IMPL_TEXCOORD

	XMFLOAT3 position;
	XMFLOAT2 texcoord;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ false };
	static constexpr bool hasColor{ false };
	static constexpr bool hasTexcoord{ true };
};

static_assert(sizeof(VertexPositionTexture) == 5 * sizeof(float), "Size mismatch in VertexPositionTexture!");


struct VertexPositionNormal : public EmptyVertex
{
	VertexPositionNormal() = default;

	VertexPositionNormal(const VertexPositionNormal&) = default;
	VertexPositionNormal& operator=(const VertexPositionNormal&) = default;

	VertexPositionNormal(VertexPositionNormal&&) = default;
	VertexPositionNormal& operator=(VertexPositionNormal&&) = default;

	IMPL_POSITION
	IMPL_NORMAL

	XMFLOAT3 position;
	XMFLOAT3 normal;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ true };
	static constexpr bool hasColor{ false };
	static constexpr bool hasTexcoord{ false };
};

static_assert(sizeof(VertexPositionNormal) == 6 * sizeof(float), "Size mismatch in VertexPositionNormal!");


struct VertexPositionColorTexture : public EmptyVertex
{
	VertexPositionColorTexture() = default;

	VertexPositionColorTexture(const VertexPositionColorTexture&) = default;
	VertexPositionColorTexture& operator=(const VertexPositionColorTexture&) = default;

	VertexPositionColorTexture(VertexPositionColorTexture&&) = default;
	VertexPositionColorTexture& operator=(VertexPositionColorTexture&&) = default;

	IMPL_POSITION
	IMPL_COLOR
	IMPL_TEXCOORD

	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ false };
	static constexpr bool hasColor{ true };
	static constexpr bool hasTexcoord{ true };
};

static_assert(sizeof(VertexPositionColorTexture) == 9 * sizeof(float), "Size mismatch in VertexPositionColorTexture!");


struct VertexPositionNormalColor : public EmptyVertex
{
	VertexPositionNormalColor() = default;

	VertexPositionNormalColor(const VertexPositionNormalColor&) = default;
	VertexPositionNormalColor& operator=(const VertexPositionNormalColor&) = default;

	VertexPositionNormalColor(VertexPositionNormalColor&&) = default;
	VertexPositionNormalColor& operator=(VertexPositionNormalColor&&) = default;

	IMPL_POSITION
	IMPL_NORMAL
	IMPL_COLOR

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 color;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ true };
	static constexpr bool hasColor{ false };
	static constexpr bool hasTexcoord{ true };
};

static_assert(sizeof(VertexPositionNormalColor) == 10 * sizeof(float), "Size mismatch in VertexPositionNormalColor!");


struct VertexPositionNormalTexture : public EmptyVertex
{
	VertexPositionNormalTexture() = default;

	VertexPositionNormalTexture(const VertexPositionNormalTexture&) = default;
	VertexPositionNormalTexture& operator=(const VertexPositionNormalTexture&) = default;

	VertexPositionNormalTexture(VertexPositionNormalTexture&&) = default;
	VertexPositionNormalTexture& operator=(VertexPositionNormalTexture&&) = default;

	IMPL_POSITION
	IMPL_NORMAL
	IMPL_TEXCOORD

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texcoord;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ true };
	static constexpr bool hasColor{ false };
	static constexpr bool hasTexcoord{ true };
};

static_assert(sizeof(VertexPositionNormalTexture) == 8 * sizeof(float), "Size mismatch in VertexPositionNormalTexture!");


struct VertexPositionNormalColorTexture : public EmptyVertex
{
	VertexPositionNormalColorTexture() = default;

	VertexPositionNormalColorTexture(const VertexPositionNormalColorTexture&) = default;
	VertexPositionNormalColorTexture& operator=(const VertexPositionNormalColorTexture&) = default;

	VertexPositionNormalColorTexture(VertexPositionNormalColorTexture&&) = default;
	VertexPositionNormalColorTexture& operator=(VertexPositionNormalColorTexture&&) = default;

	IMPL_POSITION
	IMPL_NORMAL
	IMPL_COLOR
	IMPL_TEXCOORD

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;

	// Type traits
	static constexpr bool hasNormal{ true };
	static constexpr bool hasColor{ true };
	static constexpr bool hasTexcoord{ true };
};

static_assert(sizeof(VertexPositionNormalColorTexture) == 12 * sizeof(float), "Size mismatch in VertexPositionNormalColorTexture!");


template <typename E>
struct IsVertex
{
	static const bool value{ false };
	operator bool() const { return value; }
	bool operator()() const { return value; }
};


template <> struct IsVertex<VertexPosition> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionColor> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionTexture> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionNormal> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionColorTexture> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionNormalColor> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionNormalTexture> { static const bool value{ true }; };
template <> struct IsVertex<VertexPositionNormalColorTexture> { static const bool value{ true }; };

} // namespace Kodiak

#pragma pack(pop)