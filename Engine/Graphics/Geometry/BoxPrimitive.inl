#pragma once

template<typename VertexType>
ModelPtr Model::MakeBox(const Math::Vector3& size, bool bRightHanded, bool bInvertNormals)
{
	return MakeBox<VertexType>(DirectX::Colors::White, size, bRightHanded, bInvertNormals);
}


template <typename VertexType>
ModelPtr Model::MakeBox(const Color& color, const Math::Vector3& size, bool bRightHanded, bool bInvertNormals)
{
	std::vector<VertexType> vertices;
	std::vector<uint16_t> indices;

	MakeBox<VertexType>(vertices, indices, color, size, bRightHanded, bInvertNormals);

	Math::BoundingBox boundingBox{ Math::Vector3{ Math::kZero }, 0.5f * size };

	return BuildModel<VertexType>("Box", vertices, indices, boundingBox);
}


template <typename VertexType>
void Model::MakeBox(std::vector<VertexType>& vertices, std::vector<uint16_t>& indices, const Math::Vector3& size, bool bRightHanded, bool bInvertNormals)
{
	MakeBox<VertexType>(vertices, indices, DirectX::Colors::White, size, bRightHanded, bInvertNormals);
}


template <typename VertexType>
void Model::MakeBox(std::vector<VertexType>& vertices, std::vector<uint16_t>& indices, const Color& color, const Math::Vector3& size, bool bRightHanded, bool bInvertNormals)
{
	using namespace Math;

	constexpr int faceCount = 6;

	vertices.clear();
	vertices.reserve(4 * faceCount);

	indices.clear();
	indices.reserve(6 * faceCount);	

	static const Math::Vector3 faceNormals[faceCount] =
	{
		Vector3{ 0.0f,  0.0f,  1.0f },
		Vector3{ 0.0f,  0.0f, -1.0f },
		Vector3{ 1.0f,  0.0f,  0.0f },
		Vector3{-1.0f,  0.0f,  0.0f },
		Vector3{ 0.0f,  1.0f,  0.0f },
		Vector3{ 0.0f, -1.0f,  0.0f }
	};

	static const XMFLOAT2 texcoords[4] =
	{
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f }
	};

	Vector3 halfSize = 0.5f * size;

	for (int i = 0; i < faceCount; ++i)
	{
		Vector3 normal = faceNormals[i];
		
		Vector3 basis = (i >= 4) ? Vector3{ kZUnitVector } : Vector3{ kYUnitVector };
		Vector3 side1 = Cross(normal, basis);
		Vector3 side2 = Cross(normal, side1);

		size_t vertBase = vertices.size();

		AppendIndex(indices, vertBase + 0);
		AppendIndex(indices, vertBase + 2);
		AppendIndex(indices, vertBase + 1);

		AppendIndex(indices, vertBase + 0);
		AppendIndex(indices, vertBase + 3);
		AppendIndex(indices, vertBase + 2);

		VertexType vert{};

		Vector3 positions[] =
		{
			halfSize * ((normal - side1) - side2),
			halfSize * ((normal - side1) + side2),
			halfSize * ((normal + side1) + side2),
			halfSize * ((normal + side1) - side2)
		};

		for (int j = 0; j < 4; ++j)
		{
			vert.SetPosition(positions[j]);
			if constexpr (VertexType::hasNormal) vert.SetNormal(normal);
			if constexpr (VertexType::hasColor) vert.SetColor(color);
			if constexpr (VertexType::hasTexcoord) vert.SetTexcoord(texcoords[j]);

			vertices.push_back(vert);
		}
	}

	if (!bRightHanded)
		ReverseWinding<VertexType>(indices, vertices);

	if (bInvertNormals)
		InvertNormals<VertexType>(vertices);
}