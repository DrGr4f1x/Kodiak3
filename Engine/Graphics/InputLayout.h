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

namespace Kodiak
{

struct VertexStreamDesc
{
	uint32_t inputSlot;
	uint32_t stride;
	InputClassification inputClassification;
};


struct VertexElementDesc
{
	const char*				semanticName;
	uint32_t				semanticIndex;
	Format					format;
	uint32_t				inputSlot;		// Which input vertex stream we're coming from
	uint32_t				alignedByteOffset;
	InputClassification		inputClassification;
	uint32_t				instanceDataStepRate;
};


enum class VertexComponent
{
	None =			0,
	Position =		1 << 0,
	Normal =		1 << 1,
	Tangent =		1 << 2,
	Bitangent =		1 << 3,
	Color0 =		1 << 4,
	Color1 =		1 << 5,
	Texcoord0 =		1 << 6,
	Texcoord1 =		1 << 7,
	Texcoord2 =		1 << 8,
	Texcoord3 =		1 << 9,
	BlendIndices =	1 << 10,
	BlendWeight =	1 << 11,

	Color = Color0,
	Texcoord = Texcoord0,

	PositionColor = Position | Color,
	PositionNormal = Position | Normal,
	PositionTexcoord = Position | Texcoord,
	PositionColorTexcoord = Position | Color | Texcoord,
	PositionNormalColor = Position | Normal | Color,
	PositionNormalTexcoord = Position | Normal | Texcoord,
	PositionNormalColorTexcoord = Position | Normal | Color | Texcoord
};

template <> struct EnableBitmaskOperators<VertexComponent> { static const bool enable = true; };


uint32_t GetVertexComponentSizeInBytes(VertexComponent component);
uint32_t GetVertexComponentNumFloats(VertexComponent component);


class VertexLayoutBase
{
public:
	uint32_t GetSizeInBytes() const { return m_sizeInBytes;  }
	uint32_t GetNumFloats() const { return m_numFloats; }
	const std::vector<VertexElementDesc>& GetElements() const { return *m_elements; }
	virtual VertexComponent GetComponents() const = 0;

protected:
	void Setup(VertexComponent components);

protected:
	uint32_t m_sizeInBytes{ 0 };
	uint32_t m_numFloats{ 0 };
	const std::vector<VertexElementDesc>* m_elements{ nullptr };
};


template <VertexComponent VC>
class VertexLayout : public VertexLayoutBase
{
public:
	VertexLayout() { Setup(VC); }

	VertexComponent GetComponents() const final { return VC; }
};


} // namespace Kodiak