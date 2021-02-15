//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "Model.h"

#include "Filesystem.h"
#include "Graphics\CommandContext.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>


using namespace Kodiak;
using namespace Math;
using namespace std;


namespace
{

void UpdateExtents(Math::Vector3& minExtents, Math::Vector3& maxExtents, const Math::Vector3& pos)
{
	minExtents = Math::Min(minExtents, pos);
	maxExtents = Math::Max(maxExtents, pos);
}

uint32_t GetPreprocessFlags(ModelLoad modelLoadFlags)
{
	uint32_t flags = 0;

	flags |= HasFlag(modelLoadFlags, ModelLoad::CalcTangentSpace) ? aiProcess_CalcTangentSpace : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::JoinIdenticalVertices) ? aiProcess_JoinIdenticalVertices : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::MakeLeftHanded) ? aiProcess_CalcTangentSpace : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::Triangulate) ? aiProcess_Triangulate : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::RemoveComponent) ? aiProcess_RemoveComponent : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::GenNormals) ? aiProcess_GenNormals : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::GenSmoothNormals) ? aiProcess_GenSmoothNormals : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::SplitLargeMeshes) ? aiProcess_SplitLargeMeshes : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::PreTransformVertices) ? aiProcess_PreTransformVertices : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::LimitBoneWeights) ? aiProcess_LimitBoneWeights : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::ValidateDataStructure) ? aiProcess_ValidateDataStructure : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::ImproveCacheLocality) ? aiProcess_ImproveCacheLocality : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::RemoveRedundantMaterials) ? aiProcess_RemoveRedundantMaterials : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FixInfacingNormals) ? aiProcess_FixInfacingNormals : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::SortByPType) ? aiProcess_SortByPType : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FindDegenerates) ? aiProcess_FindDegenerates : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FindInvalidData) ? aiProcess_FindInvalidData : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::GenUVCoords) ? aiProcess_GenUVCoords : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::TransformUVCoords) ? aiProcess_TransformUVCoords : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FindInstances) ? aiProcess_FindInstances : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::OptimizeMeshes) ? aiProcess_OptimizeMeshes : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::OptimizeGraph) ? aiProcess_OptimizeGraph : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FlipUVs) ? aiProcess_FlipUVs : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::FlipWindingOrder) ? aiProcess_FlipWindingOrder : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::SplitByBoneCount) ? aiProcess_SplitByBoneCount : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::Debone) ? aiProcess_Debone : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::GlobalScale) ? aiProcess_GlobalScale : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::EmbedTextures) ? aiProcess_EmbedTextures : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::ForceGenNormals) ? aiProcess_ForceGenNormals : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::DropNormals) ? aiProcess_DropNormals : 0;
	flags |= HasFlag(modelLoadFlags, ModelLoad::GenBoundingBoxes) ? aiProcess_GenBoundingBoxes : 0;

	return flags;
}

} // anonymous namespace


void Mesh::SetName(const string& name)
{
	m_name = name;
}


void Mesh::AddMeshPart(MeshPart meshPart)
{
	m_meshParts.push_back(meshPart);
}


void Mesh::SetMatrix(const Matrix4& matrix)
{
	m_matrix = matrix;
	// TODO transform bounding box
}


void Mesh::Render(GraphicsContext& context)
{
	context.SetIndexBuffer(m_indexBuffer);
	context.SetVertexBuffer(0, m_vertexBuffer);

	for (const auto& meshPart : m_meshParts)
	{
		context.DrawIndexed(meshPart.indexCount, meshPart.indexBase, meshPart.vertexBase);
	}
}


void Model::SetName(const string& name)
{
	m_name = name;
}


void Model::AddMesh(MeshPtr mesh)
{
	mesh->m_model = this;
	m_meshes.push_back(mesh);
}


void Model::SetMatrix(const Matrix4& matrix)
{
	m_matrix = matrix;
}


void Model::Render(GraphicsContext& context)
{
	for (auto mesh : m_meshes)
	{
		mesh->Render(context);
	}
}


ModelPtr Model::Load(const string& filename, const VertexLayout& layout, float scale, ModelLoad modelLoadFlags)
{
	const string fullpath = Filesystem::GetInstance().GetFullPath(filename);
	assert(!fullpath.empty());

	Assimp::Importer aiImporter;

	const auto aiScene = aiImporter.ReadFile(fullpath.c_str(), GetPreprocessFlags(modelLoadFlags));
	assert(aiScene != nullptr);

	ModelPtr model = make_shared<Model>();

	model->m_meshes.clear();
	model->m_meshes.reserve(aiScene->mNumMeshes);

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	const aiVector3D zero(0.0f, 0.0f, 0.0f);

	vector<float> vertexData;
	vector<uint32_t> indexData;

	// Min/max for bounding box computation
	float maxF = std::numeric_limits<float>::max();
	Math::Vector3 minExtents(maxF, maxF, maxF);
	Math::Vector3 maxExtents(-maxF, -maxF, -maxF);

	for (uint32_t i = 0; i < aiScene->mNumMeshes; ++i)
	{
		const auto aiMesh = aiScene->mMeshes[i];

		MeshPtr mesh = make_shared<Mesh>();
		MeshPart meshPart = {};
		
		meshPart.vertexBase = vertexCount;

		vertexCount += aiMesh->mNumVertices;

		aiColor3D color(0.0f, 0.0f, 0.0f);
		aiScene->mMaterials[aiMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

		for (uint32_t j = 0; j < aiMesh->mNumVertices; ++j)
		{
			const aiVector3D* pos = &(aiMesh->mVertices[j]);
			const aiVector3D* normal = &(aiMesh->mNormals[j]);
			const aiVector3D* texCoord = (aiMesh->HasTextureCoords(0)) ? &(aiMesh->mTextureCoords[0][j]) : &zero;
			const aiVector3D* tangent = (aiMesh->HasTangentsAndBitangents()) ? &(aiMesh->mTangents[j]) : &zero;
			const aiVector3D* bitangent = (aiMesh->HasTangentsAndBitangents()) ? &(aiMesh->mBitangents[j]) : &zero;

			for (const auto& component : layout.components)
			{
				switch (component)
				{
				case VertexComponent::Position:
					vertexData.push_back(pos->x * scale);
					vertexData.push_back(pos->y * scale);  // TODO: Is this a hack?
					vertexData.push_back(pos->z * scale);
					UpdateExtents(minExtents, maxExtents, scale * Math::Vector3(pos->x, pos->y, pos->z));
					break;
				case VertexComponent::Normal:
					vertexData.push_back(normal->x);
					vertexData.push_back(normal->y); // TODO: Is this a hack?
					vertexData.push_back(normal->z);
					break;
				case VertexComponent::UV:
					vertexData.push_back(texCoord->x);
					vertexData.push_back(texCoord->y);
					break;
				case VertexComponent::Color:
					vertexData.push_back(color.r);
					vertexData.push_back(color.g);
					vertexData.push_back(color.b);
					break;
				case VertexComponent::Tangent:
					vertexData.push_back(tangent->x);
					vertexData.push_back(tangent->y);
					vertexData.push_back(tangent->z);
					break;
				case VertexComponent::Bitangent:
					vertexData.push_back(bitangent->x);
					vertexData.push_back(bitangent->y);
					vertexData.push_back(bitangent->z);
					break;
					// Dummy components for padding
				case VertexComponent::DummyFloat:
					vertexData.push_back(0.0f);
					break;
				case VertexComponent::DummyVec4:
					vertexData.push_back(0.0f);
					vertexData.push_back(0.0f);
					vertexData.push_back(0.0f);
					vertexData.push_back(0.0f);
					break;
				}
			}
		}

		meshPart.vertexCount = aiMesh->mNumVertices;

		uint32_t indexBase = static_cast<uint32_t>(indexData.size());
		for (unsigned int j = 0; j < aiMesh->mNumFaces; j++)
		{
			const aiFace& Face = aiMesh->mFaces[j];
			if (Face.mNumIndices != 3)
				continue;
			indexData.push_back(indexBase + Face.mIndices[0]);
			indexData.push_back(indexBase + Face.mIndices[1]);
			indexData.push_back(indexBase + Face.mIndices[2]);
			meshPart.indexCount += 3;
			indexCount += 3;
		}

		uint32_t stride = layout.ComputeStride();
		mesh->m_vertexBuffer.Create("Model|VertexBuffer", sizeof(float) * vertexData.size() / stride, stride, false, vertexData.data());
		mesh->m_indexBuffer.Create("Model|IndexBuffer", indexData.size(), sizeof(uint32_t), false, indexData.data());
		mesh->m_boundingBox = Math::BoundingBoxFromMinMax(minExtents, maxExtents);

		mesh->AddMeshPart(meshPart);
		model->AddMesh(mesh);
	}

	return model;
}


shared_ptr<Model> Model::MakePlane(const VertexLayout& layout, float width, float height)
{
	bool bHasNormals = false;
	bool bHasUVs = false;
	for (auto component : layout.components)
	{
		if (component == VertexComponent::Normal) bHasNormals = true;
		if (component == VertexComponent::UV) bHasUVs = true;
		if (bHasNormals && bHasUVs) break;
	}

	uint32_t stride = 3  * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	vector<float> vertices;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve(4 * vertexSize);

	// Vertex 0
	vertices.push_back(width / 2.0f);
	vertices.push_back(0.0f);
	vertices.push_back(-height / 2.0f);
	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}
	if (bHasUVs)
	{
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
	}

	// Vertex 1
	vertices.push_back(width / 2.0f);
	vertices.push_back(0.0f);
	vertices.push_back(height / 2.0f);
	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}
	if (bHasUVs)
	{
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}

	// Vertex 2
	vertices.push_back(-width / 2.0f);
	vertices.push_back(0.0f);
	vertices.push_back(-height / 2.0f);
	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}
	if (bHasUVs)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
	}

	// Vertex 3
	vertices.push_back(-width / 2.0f);
	vertices.push_back(0.0f);
	vertices.push_back(height / 2.0f);
	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}
	if (bHasUVs)
	{
		vertices.push_back(1.0f);
		vertices.push_back(1.0f);
	}

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	mesh->m_vertexBuffer.Create("Plane|VertexBuffer", vertices.size(), stride, false, vertices.data());

	vector<uint16_t> indices { 0, 2, 1, 3, 1, 2 };
	mesh->m_indexBuffer.Create("Plane|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	mesh->m_boundingBox = Math::BoundingBox(Math::Vector3(Math::kZero), Math::Vector3(width / 2.0f, 0.0, height / 2.0f));
	model->m_boundingBox = mesh->m_boundingBox;

	MeshPart meshPart = {};
	meshPart.indexCount = uint32_t(indices.size());

	mesh->AddMeshPart(meshPart);
	model->AddMesh(mesh);

	return model;
}


shared_ptr<Model> Model::MakeCylinder(const VertexLayout& layout, float height, float radius, uint32_t numVerts)
{
	bool bHasNormals = false;
	bool bHasUVs = false;
	for (auto component : layout.components)
	{
		if (component == VertexComponent::Normal) bHasNormals = true;
		if (component == VertexComponent::UV) bHasUVs = true;
		if (bHasNormals && bHasUVs) break;
	}

	uint32_t stride = 3 * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	vector<float> vertices;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve((4 * numVerts + 2) * vertexSize);

	vector<uint16_t> indices;

	float deltaPhi = DirectX::XM_2PI / static_cast<float>(numVerts - 1);

	// Cylinder side
	float phi = 0.0f;
	for (uint32_t i = 0; i < numVerts; ++i)
	{
		const float nx = sinf(phi);
		const float nz = cosf(phi);
		const float x = radius * nx;
		const float z = radius * nz;
		const float u = phi / DirectX::XM_2PI;

		// Position top
		vertices.push_back(x);
		vertices.push_back(height);
		vertices.push_back(z);

		if (bHasNormals)
		{
			vertices.push_back(nx);
			vertices.push_back(0.0f);
			vertices.push_back(nz);
		}

		if (bHasUVs)
		{
			vertices.push_back(u);
			vertices.push_back(1.0);
		}

		// Position bottom
		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);

		if (bHasNormals)
		{
			vertices.push_back(nx);
			vertices.push_back(0.0f);
			vertices.push_back(nz);
		}

		if (bHasUVs)
		{
			vertices.push_back(u);
			vertices.push_back(0.0);
		}

		indices.push_back(2 * i);
		indices.push_back(2 * i + 1);

		phi += deltaPhi;
	}

	#if 1

	// Restart strip
	indices.push_back(0xFFFF);

	// Cylinder bottom
	phi = 0.0f;
	uint16_t startVert = 2 * numVerts;
	for (uint32_t i = 0; i < numVerts; ++i)
	{
		const float nx = sinf(phi);
		const float nz = cosf(phi);
		const float x = radius * nx;
		const float z = radius * nz;

		// Position bottom
		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);

		if (bHasNormals)
		{
			vertices.push_back(0.0f);
			vertices.push_back(-1.0f);
			vertices.push_back(0.0f);
		}

		if (bHasUVs)
		{
			vertices.push_back(0.5f * nx + 0.5f);
			vertices.push_back(0.5f * nz + 0.5f);
		}

		indices.push_back(3 * numVerts);
		indices.push_back(startVert + i);

		phi += deltaPhi;
	}
	// Position center
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);
	vertices.push_back(0.0f);

	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(-1.0f);
		vertices.push_back(0.0f);
	}

	if (bHasUVs)
	{
		vertices.push_back(0.5f);
		vertices.push_back(0.5f);
	}

	// Restart strip
	indices.push_back(0xFFFF);

	// Cylinder top
	phi = 0.0f;
	startVert = 3 * numVerts + 1;
	for (uint32_t i = 0; i < numVerts; ++i)
	{
		const float nx = sinf(phi);
		const float nz = cosf(phi);
		const float x = radius * nx;
		const float z = radius * nz;

		// Position top
		vertices.push_back(x);
		vertices.push_back(height);
		vertices.push_back(z);

		if (bHasNormals)
		{
			vertices.push_back(0.0f);
			vertices.push_back(1.0f);
			vertices.push_back(0.0f);
		}

		if (bHasUVs)
		{
			vertices.push_back(0.5f * nx + 0.5f);
			vertices.push_back(0.5f * nz + 0.5f);
		}

		indices.push_back(4 * numVerts + 1);
		indices.push_back(startVert + i);

		phi += deltaPhi;
	}
	// Position center
	vertices.push_back(0.0f);
	vertices.push_back(height);
	vertices.push_back(0.0f);

	if (bHasNormals)
	{
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);
		vertices.push_back(0.0f);
	}

	if (bHasUVs)
	{
		vertices.push_back(0.5f);
		vertices.push_back(0.5f);
	}
#endif

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	mesh->m_vertexBuffer.Create("Cylinder|VertexBuffer", vertices.size(), stride, false, vertices.data());
	mesh->m_indexBuffer.Create("Cylinder|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	mesh->m_boundingBox = Math::BoundingBoxFromMinMax(Math::Vector3(-radius, 0.0f, -radius), Math::Vector3(radius, height, radius));
	model->m_boundingBox = mesh->m_boundingBox;

	MeshPart meshPart = {};
	meshPart.indexCount = uint32_t(indices.size());

	mesh->AddMeshPart(meshPart);
	model->AddMesh(mesh);

	return model;
}


//shared_ptr<Model> Model::MakeSphere(const VertexLayout& layout, uint32_t numVerts, uint32_t numRings, float radius)
//{
//
//}