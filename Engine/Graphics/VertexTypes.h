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

struct VertexPosition
{
	VertexPosition() = default;

	VertexPosition(const VertexPosition&) = default;
	VertexPosition& operator=(const VertexPosition&) = default;

	VertexPosition(VertexPosition&&) = default;
	VertexPosition& operator=(VertexPosition&&) = default;

	VertexPosition(const Math::Vector3& pos)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
	{}

	// Position
	float x;
	float y;
	float z;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionColor
{
	VertexPositionColor() = default;

	VertexPositionColor(const VertexPositionColor&) = default;
	VertexPositionColor& operator=(const VertexPositionColor&) = default;

	VertexPositionColor(VertexPositionColor&&) = default;
	VertexPositionColor& operator=(VertexPositionColor&&) = default;

	VertexPositionColor(const Math::Vector3& pos, const Color& color)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, r{ color.R() }
		, g{ color.G() }
		, b{ color.B() }
		, a{ color.A() }
	{}

	// Position
	float x;
	float y;
	float z;
	// Color
	float r;
	float g;
	float b;
	float a;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionTexture
{
	VertexPositionTexture() = default;

	VertexPositionTexture(const VertexPositionTexture&) = default;
	VertexPositionTexture& operator=(const VertexPositionTexture&) = default;

	VertexPositionTexture(VertexPositionTexture&&) = default;
	VertexPositionTexture& operator=(VertexPositionTexture&&) = default;

	VertexPositionTexture(const Math::Vector3& pos, float u, float v)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, u{ u }
		, v{ v }
	{}

	// Position
	float x;
	float y;
	float z;
	// Texcoord
	float u;
	float v;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionNormal
{
	VertexPositionNormal() = default;

	VertexPositionNormal(const VertexPositionNormal&) = default;
	VertexPositionNormal& operator=(const VertexPositionNormal&) = default;

	VertexPositionNormal(VertexPositionNormal&&) = default;
	VertexPositionNormal& operator=(VertexPositionNormal&&) = default;

	VertexPositionNormal(const Math::Vector3& pos, const Math::Vector3& normal)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, nx{ normal.GetX() }
		, ny{ normal.GetY() }
		, nz{ normal.GetZ() }
	{}

	// Position
	float x;
	float y;
	float z;
	// Normal
	float nx;
	float ny;
	float nz;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionColorTexture
{
	VertexPositionColorTexture() = default;

	VertexPositionColorTexture(const VertexPositionColorTexture&) = default;
	VertexPositionColorTexture& operator=(const VertexPositionColorTexture&) = default;

	VertexPositionColorTexture(VertexPositionColorTexture&&) = default;
	VertexPositionColorTexture& operator=(VertexPositionColorTexture&&) = default;

	VertexPositionColorTexture(const Math::Vector3& pos, const Color& color, float u, float v)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, r{ color.R() }
		, g{ color.G() }
		, b{ color.B() }
		, a{ color.A() }
		, u{ u }
		, v{ v }
	{}

	// Position
	float x;
	float y;
	float z;
	// Color
	float r;
	float g;
	float b;
	float a;
	// Texcoord
	float u;
	float v;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionNormalColor
{
	VertexPositionNormalColor() = default;

	VertexPositionNormalColor(const VertexPositionNormalColor&) = default;
	VertexPositionNormalColor& operator=(const VertexPositionNormalColor&) = default;

	VertexPositionNormalColor(VertexPositionNormalColor&&) = default;
	VertexPositionNormalColor& operator=(VertexPositionNormalColor&&) = default;

	VertexPositionNormalColor(const Math::Vector3& pos, const Math::Vector3& normal, const Color& color)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, nx{ normal.GetX() }
		, ny{ normal.GetY() }
		, nz{ normal.GetZ() }
		, r{ color.R() }
		, g{ color.G() }
		, b{ color.B() }
		, a{ color.A() }
	{}

	// Position
	float x;
	float y;
	float z;
	// Normal
	float nx;
	float ny;
	float nz;
	// Color
	float r;
	float g;
	float b;
	float a;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionNormalTexture
{
	VertexPositionNormalTexture() = default;

	VertexPositionNormalTexture(const VertexPositionNormalTexture&) = default;
	VertexPositionNormalTexture& operator=(const VertexPositionNormalTexture&) = default;

	VertexPositionNormalTexture(VertexPositionNormalTexture&&) = default;
	VertexPositionNormalTexture& operator=(VertexPositionNormalTexture&&) = default;

	VertexPositionNormalTexture(const Math::Vector3& pos, const Math::Vector3& normal, float u, float v)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, nx{ normal.GetX() }
		, ny{ normal.GetY() }
		, nz{ normal.GetZ() }
		, u{ u }
		, v{ v }
	{}

	// Position
	float x;
	float y;
	float z;
	// Normal
	float nx;
	float ny;
	float nz;
	// Texcoord
	float u;
	float v;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};


struct VertexPositionNormalColorTexture
{
	VertexPositionNormalColorTexture() = default;

	VertexPositionNormalColorTexture(const VertexPositionNormalColorTexture&) = default;
	VertexPositionNormalColorTexture& operator=(const VertexPositionNormalColorTexture&) = default;

	VertexPositionNormalColorTexture(VertexPositionNormalColorTexture&&) = default;
	VertexPositionNormalColorTexture& operator=(VertexPositionNormalColorTexture&&) = default;

	VertexPositionNormalColorTexture(const Math::Vector3& pos, const Math::Vector3& normal, const Color& color, float u, float v)
		: x{ pos.GetX() }
		, y{ pos.GetY() }
		, z{ pos.GetZ() }
		, nx{ normal.GetX() }
		, ny{ normal.GetY() }
		, nz{ normal.GetZ() }
		, r{ color.R() }
		, g{ color.G() }
		, b{ color.B() }
		, a{ color.A() }
		, u{ u }
		, v{ v }
	{}

	// Position
	float x;
	float y;
	float z;
	// Normal
	float nx;
	float ny;
	float nz;
	// Color
	float r;
	float g;
	float b;
	float a;
	// Texcoord
	float u;
	float v;

	static const VertexStreamDesc Stream;
	static const std::vector<VertexElementDesc> Layout;
};

} // namespace Kodiak

#pragma pack(pop)