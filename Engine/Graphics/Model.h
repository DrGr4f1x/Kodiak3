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

#include "Color.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\VertexTypes.h"
#include "Math\BoundingBox.h"


namespace Kodiak
{

// Forward declarations
class GraphicsContext;


enum class ModelLoad
{
	CalcTangentSpace =				1 << 0,
	JoinIdenticalVertices =			1 << 1,
	MakeLeftHanded  =				1 << 2,
	Triangulate =					1 << 3,
	RemoveComponent =				1 << 4,
	GenNormals =					1 << 5,
	GenSmoothNormals =				1 << 6,
	SplitLargeMeshes =				1 << 7,
	PreTransformVertices =			1 << 8,
	LimitBoneWeights =				1 << 9,
	ValidateDataStructure =			1 << 10,
	ImproveCacheLocality =			1 << 11,
	RemoveRedundantMaterials =		1 << 12,
	FixInfacingNormals =			1 << 13,
	SortByPType =					1 << 14,
	FindDegenerates =				1 << 15,
	FindInvalidData =				1 << 16,
	GenUVCoords =					1 << 17,
	TransformUVCoords =				1 << 18,
	FindInstances =					1 << 19,
	OptimizeMeshes =				1 << 20,
	OptimizeGraph =					1 << 21,
	FlipUVs =						1 << 22,
	FlipWindingOrder =				1 << 23,
	SplitByBoneCount =				1 << 24,
	Debone =						1 << 25,
	GlobalScale =					1 << 26,
	EmbedTextures =					1 << 27,
	ForceGenNormals =				1 << 28,
	DropNormals =					1 << 29,
	GenBoundingBoxes =				1 << 30,

	ConvertToLeftHandded =			MakeLeftHanded | 
									FlipUVs | 
									FlipWindingOrder,

	TargetRealtime_Fast =			CalcTangentSpace | 
									GenNormals | 
									JoinIdenticalVertices | 
									Triangulate | 
									GenUVCoords | 
									SortByPType,

	TargetRealtime_Quality =		CalcTangentSpace | 
									GenSmoothNormals | 
									JoinIdenticalVertices | 
									Triangulate | 
									GenUVCoords | 
									SortByPType | 
									ImproveCacheLocality | 
									LimitBoneWeights | 
									RemoveRedundantMaterials | 
									SplitLargeMeshes | 
									FindDegenerates | 
									FindInvalidData,

	TargetRealtime_MaxQuality =		TargetRealtime_Quality |
									FindInstances |
									ValidateDataStructure |
									OptimizeMeshes,

	StandardDefault =				FlipUVs |
									Triangulate |
									PreTransformVertices |
									CalcTangentSpace |
									GenBoundingBoxes
};

template <> struct EnableBitmaskOperators<ModelLoad> { static const bool enable = true; };


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
	VertexLayout(const std::vector<VertexComponent>& components) : components(components)
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

	uint32_t ComputeNumFloats() const
	{
		uint32_t res = 0;

		for (const auto& component : components)
		{
			switch (component)
			{
			case VertexComponent::UV:
				res += 2;
				break;
			case VertexComponent::DummyFloat:
				res += 1;
				break;
			case VertexComponent::DummyVec4:
				res += 4;
				break;
			default:
				res += 3;
				break;
			}
		}

		return res;
	}

	std::vector<VertexComponent> components;
};


struct MeshPart
{
	uint32_t vertexBase{ 0 };
	uint32_t vertexCount{ 0 };
	uint32_t indexBase{ 0 };
	uint32_t indexCount{ 0 };
};


class Mesh
{
	friend class Model;

public:
	// Accessors
	void SetName(const std::string& name);
	const std::string& GetName() const { return m_name; }

	void AddMeshPart(MeshPart meshPart);
	size_t GetNumMeshParts() const { return m_meshParts.size(); }
	MeshPart& GetMeshPart(size_t index) { return m_meshParts[index]; }
	const MeshPart& GetMeshPart(size_t index) const { return m_meshParts[index]; }

	VertexBuffer& GetVertexBuffer() { return m_vertexBuffer; }
	const VertexBuffer& GetVertexBuffer() const { return m_vertexBuffer; }
	IndexBuffer& GetIndexBuffer() { return m_indexBuffer; }
	const IndexBuffer& GetIndexBuffer() const { return m_indexBuffer; }

	VertexBuffer& GetPositionOnlyVertexBuffer() { return m_vertexBufferPositionOnly; }
	const VertexBuffer& GetPositionOnlyVertexBuffer() const { return m_vertexBufferPositionOnly; }

	void SetMatrix(const Math::Matrix4& matrix);
	const Math::Matrix4 GetMatrix() const { return m_matrix; }

	void Render(GraphicsContext& context);
	void RenderPositionOnly(GraphicsContext& context);

private:
	std::string m_name;

	VertexBuffer m_vertexBuffer;
	VertexBuffer m_vertexBufferPositionOnly;
	IndexBuffer m_indexBuffer;

	Math::Matrix4 m_matrix{ Math::kIdentity };
	Math::BoundingBox m_boundingBox;
	
	std::vector<MeshPart> m_meshParts;

	class Model* m_model{ nullptr };
};

using MeshPtr = std::shared_ptr<Mesh>;


class Model
{
public:
	// Accessors
	void SetName(const std::string& name);
	const std::string& GetName() const { return m_name; }

	void AddMesh(MeshPtr mesh);
	size_t GetNumMeshes() const { return m_meshes.size(); }
	MeshPtr GetMesh(size_t index) const { return m_meshes[index]; }

	void SetMatrix(const Math::Matrix4& matrix);
	const Math::Matrix4 GetMatrix() const { return m_matrix; }
	void StorePrevMatrix();
	const Math::Matrix4 GetPrevMatrix() const { return m_prevMatrix; }

	const Math::BoundingBox& GetBoundingBox() const { return m_boundingBox; }

	void Render(GraphicsContext& context);
	void RenderPositionOnly(GraphicsContext& context);

	static std::shared_ptr<Model> Load(const std::string& filename, const VertexLayout& layout, float scale = 1.0f, ModelLoad loadFlags = ModelLoad::StandardDefault);

	static std::shared_ptr<Model> MakePlane(const VertexLayout& layout, float width, float height);
	static std::shared_ptr<Model> MakeCylinder(const VertexLayout& layout, float height, float radius, uint32_t numVerts);
	static std::shared_ptr<Model> MakeSphere(const VertexLayout& layout, float radius, uint32_t numVerts, uint32_t numRings);

	template<typename VertexType>
	static std::shared_ptr<Model> MakeBox(const Math::Vector3& size, bool bRightHanded = true, bool bInvertNormals = false);

	template <typename VertexType>
	static std::shared_ptr<Model> MakeBox(const Color& color, const Math::Vector3& size, bool bRightHanded = true, bool bInvertNormals = false);

	template <typename VertexType>
	static void MakeBox(std::vector<VertexType>& vertices, std::vector<uint16_t>& indices, const Math::Vector3& size, bool bRightHanded = true, bool bInvertNormals = false);

	template <typename VertexType>
	static void MakeBox(std::vector<VertexType>& vertices, std::vector<uint16_t>& indices, const Color& color, const Math::Vector3& size, bool bRightHanded = true, bool bInvertNormals = false);

protected:
	std::string m_name;

	Math::Matrix4 m_matrix{ Math::kIdentity };
	Math::Matrix4 m_prevMatrix{ Math::kIdentity };
	Math::BoundingBox m_boundingBox;

	std::vector<MeshPtr> m_meshes;

private:
	// Helpers for the factory methods
	static void AppendIndex(std::vector<uint16_t>& indices, size_t value);

	template <typename VertexType>
	static void ReverseWinding(std::vector<uint16_t>& indices, std::vector<VertexType>& vertices)
	{
		assert((indices.size() % 3) == 0);
		for (auto it = indices.begin(); it != indices.end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}

		if constexpr (VertexType::hasTexcoord)
		{
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
			{
				XMFLOAT2 texcoord = it->GetTexcoord();
				it->SetTexcoord(1.0f - texcoord.x, texcoord.y);
			}
		}
	}

	template <typename VertexType>
	static void InvertNormals(std::vector<VertexType>& vertices)
	{
		if constexpr (VertexType::hasNormal)
		{
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
			{
				XMFLOAT3 normal = it->GetNormal();
				it->SetNormal(-normal.x, -normal.y, -normal.z);
			}
		}
	}

	template <typename VertexType>
	static std::shared_ptr<Model> BuildModel(const std::string& name, std::vector<VertexType>& vertices, const std::vector<uint16_t>& indices, const Math::BoundingBox& boundingBox)
	{
		MeshPtr mesh = make_shared<Mesh>();
		mesh->SetName(fmt::format("{} Mesh", name));

		MeshPart meshPart{};
		meshPart.indexCount = uint32_t(indices.size());
		meshPart.vertexCount = uint32_t(vertices.size());
		mesh->AddMeshPart(meshPart);

		mesh->m_vertexBuffer.Create("Mesh Vertex Buffer", vertices.size(), sizeof(VertexType), false, vertices.data());
		mesh->m_indexBuffer.Create("Mesh Index Buffer", indices.size(), sizeof(uint16_t), false, indices.data());

		std::vector<VertexPosition> verticesPositionOnly;
		verticesPositionOnly.reserve(vertices.size());
		for (auto it = vertices.begin(); it != vertices.end(); ++it)
		{
			VertexPosition vertPos{};
			vertPos.SetPosition(it->GetPosition());
			verticesPositionOnly.push_back(vertPos);
		}

		mesh->m_vertexBufferPositionOnly.Create("Mesh Position Only Vertex Buffer", verticesPositionOnly.size(), sizeof(VertexPosition), false, verticesPositionOnly.data());

		mesh->m_boundingBox = boundingBox;

		std::shared_ptr<Model> model = make_shared<Model>();
		model->SetName(fmt::format("{} Model", name));

		model->m_meshes.push_back(mesh);
		model->m_boundingBox = boundingBox;

		return model;
	}
};

using ModelPtr = std::shared_ptr<Model>;


// Primitives
#include "Geometry/BoxPrimitive.inl"
#include "Geometry/PlanePrimitive.inl"

} // namespace Kodiak