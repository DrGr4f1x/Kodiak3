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
const int s_defaultFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;
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
	model->m_vertexBuffer.Create("Model|VertexBuffer", sizeof(float) * vertexData.size() / stride, stride, vertexData.data());
	model->m_indexBuffer.Create("Model|IndexBuffer", indexData.size(), sizeof(uint32_t), indexData.data());

	return model;
}