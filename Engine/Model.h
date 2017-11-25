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

#include "GpuBuffer.h"

namespace Kodiak
{

enum class VertexComponent
{
	Position,
	Normal,
	Color,
	UV,
	Tangent,
	Bitangent,
	DummyFloat,
	DummyVec4
};


struct VertexLayout
{
	VertexLayout(const std::vector<VertexComponent>& components) : components(std::move(components))
	{}

	uint32_t ComputeStride() const
	{
		uint32_t res = 0;

		for (const auto& component : components)
		{
			switch (component)
			{
			case VertexComponent::UV:
				res += 2 * sizeof(float);
				break;
			case VertexComponent::DummyFloat:
				res += sizeof(float);
				break;
			case VertexComponent::DummyVec4:
				res += 4 * sizeof(float);
				break;
			default:
				res += 3 * sizeof(float);
				break;
			}
		}

		return res;
	}

	std::vector<VertexComponent> components;
};


struct ModelPart
{
	uint32_t vertexBase{ 0 };
	uint32_t vertexCount{ 0 };
	uint32_t indexBase{ 0 };
	uint32_t indexCount{ 0 };
};


class Model
{
public:

	static std::shared_ptr<Model> Load(const std::string& filename, const VertexLayout& layout);

protected:
	VertexBuffer m_vertexBuffer;
	IndexBuffer m_indexBuffer;

	std::vector<ModelPart> m_parts;
};

using ModelPtr = std::shared_ptr<Model>;

} // namespace Kodiak