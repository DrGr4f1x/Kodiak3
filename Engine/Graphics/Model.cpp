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

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>


using namespace Kodiak;
using namespace std;


namespace
{

const int s_defaultFlags = 
	aiProcess_FlipUVs | 
	//aiProcess_FlipWindingOrder |
	aiProcess_Triangulate | 
	aiProcess_PreTransformVertices | 
	aiProcess_CalcTangentSpace; // | 
	// aiProcess_GenSmoothNormals;

void UpdateExtents(Math::Vector3& minExtents, Math::Vector3& maxExtents, const Math::Vector3& pos)
{
	minExtents = Math::Min(minExtents, pos);
	maxExtents = Math::Max(maxExtents, pos);
}

} // anonymous namespace


ModelPtr Model::Load(const string& filename, const VertexLayout& layout, float scale)
{
	const string fullpath = Filesystem::GetInstance().GetFullPath(filename);
	assert(!fullpath.empty());

	Assimp::Importer aiImporter;

	const auto aiScene = aiImporter.ReadFile(fullpath.c_str(), s_defaultFlags);
	assert(aiScene != nullptr);

	ModelPtr model = make_shared<Model>();

	model->m_parts.clear();
	model->m_parts.resize(aiScene->mNumMeshes);

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

		model->m_parts[i] = {};
		model->m_parts[i].vertexBase = vertexCount;

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

		model->m_parts[i].vertexCount = aiMesh->mNumVertices;

		uint32_t indexBase = static_cast<uint32_t>(indexData.size());
		for (unsigned int j = 0; j < aiMesh->mNumFaces; j++)
		{
			const aiFace& Face = aiMesh->mFaces[j];
			if (Face.mNumIndices != 3)
				continue;
			indexData.push_back(indexBase + Face.mIndices[0]);
			indexData.push_back(indexBase + Face.mIndices[1]);
			indexData.push_back(indexBase + Face.mIndices[2]);
			model->m_parts[i].indexCount += 3;
			indexCount += 3;
		}
	}

	uint32_t stride = layout.ComputeStride();
	model->m_vertexBuffer.Create("Model|VertexBuffer", sizeof(float) * vertexData.size() / stride, stride, false, vertexData.data());
	model->m_indexBuffer.Create("Model|IndexBuffer", indexData.size(), sizeof(uint32_t), false, indexData.data());
	model->m_boundingBox = Math::BoundingBoxFromMinMax(minExtents, maxExtents);

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

	shared_ptr<Model> model = make_shared<Model>();
	model->m_vertexBuffer.Create("Plane|VertexBuffer", vertices.size(), stride, false, vertices.data());

	vector<uint16_t> indices { 0, 2, 1, 3, 1, 2 };
	model->m_indexBuffer.Create("Plane|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	model->m_boundingBox = Math::BoundingBox(Math::Vector3(Math::kZero), Math::Vector3(width / 2.0f, 0.0, height / 2.0f));

	return model;
}


//shared_ptr<Model> Model::MakeSphere(const VertexLayout& layout, uint32_t numVerts, uint32_t numRings, float radius)
//{
//
//}