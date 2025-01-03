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
#include "Graphics\InputLayout.h"

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
	m_boundingBox = m_matrix * m_boundingBox;
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


void Mesh::RenderPositionOnly(GraphicsContext& context)
{
	context.SetIndexBuffer(m_indexBuffer);
	context.SetVertexBuffer(0, m_vertexBufferPositionOnly);

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
	m_boundingBox = m_matrix * m_boundingBox;
}


void Model::StorePrevMatrix()
{
	m_prevMatrix = m_matrix;
}


void Model::Render(GraphicsContext& context)
{
	for (auto mesh : m_meshes)
	{
		mesh->Render(context);
	}
}


void Model::RenderPositionOnly(GraphicsContext& context)
{
	for (auto mesh : m_meshes)
	{
		mesh->RenderPositionOnly(context);
	}
}


ModelPtr Model::Load(const string& filename, const VertexLayoutBase& layout, float scale, ModelLoad modelLoadFlags)
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
	vector<float> vertexDataPositionOnly;
	vector<uint32_t> indexData;

	const VertexComponent components = layout.GetComponents();

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

			if (HasFlag(components, VertexComponent::Position))
			{
				vertexData.push_back(pos->x * scale);
				vertexData.push_back(pos->y * scale);  // TODO: Is this a hack?
				vertexData.push_back(pos->z * scale);

				vertexDataPositionOnly.push_back(pos->x * scale);
				vertexDataPositionOnly.push_back(pos->y * scale);  // TODO: Is this a hack?
				vertexDataPositionOnly.push_back(pos->z * scale);

				UpdateExtents(minExtents, maxExtents, scale * Math::Vector3(pos->x, pos->y, pos->z));
			}

			if (HasFlag(components, VertexComponent::Normal))
			{
				vertexData.push_back(normal->x);
				vertexData.push_back(normal->y); // TODO: Is this a hack?
				vertexData.push_back(normal->z);
			}

			if (HasFlag(components, VertexComponent::Tangent))
			{
				vertexData.push_back(tangent->x);
				vertexData.push_back(tangent->y);
				vertexData.push_back(tangent->z);
			}

			if (HasFlag(components, VertexComponent::Bitangent))
			{
				vertexData.push_back(bitangent->x);
				vertexData.push_back(bitangent->y);
				vertexData.push_back(bitangent->z);
			}

			if (HasFlag(components, VertexComponent::Color))
			{
				vertexData.push_back(color.r);
				vertexData.push_back(color.g);
				vertexData.push_back(color.b);
				vertexData.push_back(1.0f);
			}

			// TODO Color1

			if (HasFlag(components, VertexComponent::Texcoord))
			{
				vertexData.push_back(texCoord->x);
				vertexData.push_back(texCoord->y);
			}

			// TODO Texcoord1-3, BlendIndices, BlendWeight
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

		uint32_t stride = layout.GetSizeInBytes();
		mesh->m_vertexBuffer.Create("Model|VertexBuffer", sizeof(float) * vertexData.size() / stride, stride, false, vertexData.data());
		stride = 3 * sizeof(float);
		mesh->m_vertexBufferPositionOnly.Create("Model|VertexBuffer (Position Only)", sizeof(float) * vertexDataPositionOnly.size() / stride, stride, false, vertexDataPositionOnly.data());
		mesh->m_indexBuffer.Create("Model|IndexBuffer", indexData.size(), sizeof(uint32_t), false, indexData.data());
		mesh->m_boundingBox = Math::BoundingBoxFromMinMax(minExtents, maxExtents);

		mesh->AddMeshPart(meshPart);
		model->AddMesh(mesh);
	}

	return model;
}


shared_ptr<Model> Model::MakePlane(const VertexLayoutBase& layout, float width, float height)
{
	bool bHasNormals = HasFlag(layout.GetComponents(), VertexComponent::Normal);
	bool bHasUVs = HasFlag(layout.GetComponents(), VertexComponent::Texcoord);

	uint32_t stride = 3  * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	vector<float> vertices;
	vector<float> verticesPositionOnly;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve(4 * vertexSize);
	verticesPositionOnly.reserve(4 * 3);

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
	verticesPositionOnly.push_back(width / 2.0f);
	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(-height / 2.0f);

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
	verticesPositionOnly.push_back(width / 2.0f);
	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(height / 2.0f);

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
	verticesPositionOnly.push_back(-width / 2.0f);
	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(-height / 2.0f);

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
	verticesPositionOnly.push_back(-width / 2.0f);
	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(height / 2.0f);

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	mesh->m_vertexBuffer.Create("Plane|VertexBuffer", vertices.size(), stride, false, vertices.data());
	stride = 3 * sizeof(float);
	mesh->m_vertexBufferPositionOnly.Create("Plane|VertexBuffer (Position Only)", verticesPositionOnly.size(), stride, false, verticesPositionOnly.data());

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


shared_ptr<Model> Model::MakeCylinder(const VertexLayoutBase& layout, float height, float radius, uint32_t numVerts)
{
	bool bHasNormals = HasFlag(layout.GetComponents(), VertexComponent::Normal);
	bool bHasUVs = HasFlag(layout.GetComponents(), VertexComponent::Texcoord);

	uint32_t stride = 3 * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	const size_t totalVerts = 4 * numVerts + 2;

	vector<float> vertices;
	vector<float> verticesPositionOnly;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve(totalVerts * vertexSize);
	verticesPositionOnly.reserve(totalVerts * 3);

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

		verticesPositionOnly.push_back(x);
		verticesPositionOnly.push_back(height);
		verticesPositionOnly.push_back(z);

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

		verticesPositionOnly.push_back(x);
		verticesPositionOnly.push_back(0.0f);
		verticesPositionOnly.push_back(z);

		indices.push_back(2 * i);
		indices.push_back(2 * i + 1);

		phi += deltaPhi;
	}

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

		verticesPositionOnly.push_back(x);
		verticesPositionOnly.push_back(0.0f);
		verticesPositionOnly.push_back(z);

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

	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(0.0f);

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

		verticesPositionOnly.push_back(x);
		verticesPositionOnly.push_back(height);
		verticesPositionOnly.push_back(z);

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

	verticesPositionOnly.push_back(0.0f);
	verticesPositionOnly.push_back(height);
	verticesPositionOnly.push_back(0.0f);

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	assert(totalVerts == vertices.size() / vertexSize);
	mesh->m_vertexBuffer.Create("Cylinder|VertexBuffer", totalVerts, stride, false, vertices.data());
	stride = 3 * sizeof(float);
	mesh->m_vertexBufferPositionOnly.Create("Cylinder|VertexBuffer (Position Only)", totalVerts, stride, false, verticesPositionOnly.data());
	mesh->m_indexBuffer.Create("Cylinder|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	mesh->m_boundingBox = Math::BoundingBoxFromMinMax(Math::Vector3(-radius, 0.0f, -radius), Math::Vector3(radius, height, radius));
	model->m_boundingBox = mesh->m_boundingBox;

	MeshPart meshPart = {};
	meshPart.indexCount = uint32_t(indices.size());

	mesh->AddMeshPart(meshPart);
	model->AddMesh(mesh);

	return model;
}


shared_ptr<Model> Model::MakeSphere(const VertexLayoutBase& layout, float radius, uint32_t numVerts, uint32_t numRings)
{
	bool bHasNormals = HasFlag(layout.GetComponents(), VertexComponent::Normal);
	bool bHasUVs = HasFlag(layout.GetComponents(), VertexComponent::Texcoord);

	uint32_t stride = 3 * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	const size_t totalVerts = numVerts * numRings;

	vector<float> vertices;
	vector<float> verticesPositionOnly;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve(totalVerts * vertexSize);
	verticesPositionOnly.reserve(totalVerts * 3);

	vector<uint16_t> indices;

	float phi = 0.0f;
	float theta = 0.0f;
	float deltaPhi = DirectX::XM_2PI / static_cast<float>(numVerts - 1);
	float deltaTheta = DirectX::XM_PI / static_cast<float>(numRings - 1);

	uint16_t curVert = 0;
	for (uint32_t i = 0; i < numRings; ++i)
	{
		phi = 0.0f;
		for (uint32_t j = 0; j < numVerts; ++j)
		{
			float nx = sinf(theta) * cosf(phi);
			float ny = cosf(theta);
			float nz = sinf(theta) * sinf(phi);
			
			vertices.push_back(radius * nx);
			vertices.push_back(radius * ny);
			vertices.push_back(radius * nz);

			if (bHasNormals)
			{
				vertices.push_back(nx);
				vertices.push_back(ny);
				vertices.push_back(nz);
			}

			if (bHasUVs)
			{
				vertices.push_back(float(j) / float(numVerts - 1));
				vertices.push_back(float(i) / float(numRings - 1));
			}

			verticesPositionOnly.push_back(radius * nx);
			verticesPositionOnly.push_back(radius * ny);
			verticesPositionOnly.push_back(radius * nz);

			indices.push_back(curVert + numVerts);
			indices.push_back(curVert);

			++curVert;
			phi += deltaPhi;
		}

		indices.push_back(0xFFFF);

		theta += deltaTheta;
	}

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	assert(totalVerts == vertices.size() / vertexSize);
	mesh->m_vertexBuffer.Create("Sphere|VertexBuffer", totalVerts, stride, false, vertices.data());
	stride = 3 * sizeof(float);
	mesh->m_vertexBufferPositionOnly.Create("Sphere|VertexBuffer (Position Only)", totalVerts, stride, false, verticesPositionOnly.data());
	mesh->m_indexBuffer.Create("Sphere|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	mesh->m_boundingBox = Math::BoundingBoxFromMinMax(Math::Vector3(-radius, -radius, -radius), Math::Vector3(radius, radius, radius));
	model->m_boundingBox = mesh->m_boundingBox;

	MeshPart meshPart = {};
	meshPart.indexCount = uint32_t(indices.size());

	mesh->AddMeshPart(meshPart);
	model->AddMesh(mesh);

	return model;
}


shared_ptr<Model> Model::MakeBox(const VertexLayoutBase& layout, float width, float height, float depth)
{
	bool bHasNormals = HasFlag(layout.GetComponents(), VertexComponent::Normal);
	bool bHasUVs = HasFlag(layout.GetComponents(), VertexComponent::Texcoord);
	
	uint32_t stride = 3 * sizeof(float);
	stride += bHasNormals ? (3 * sizeof(float)) : 0;
	stride += bHasUVs ? (2 * sizeof(float)) : 0;

	const size_t totalVerts = 24;

	vector<float> vertices;
	vector<float> verticesPositionOnly;
	size_t vertexSize = 3; // position
	vertexSize += bHasNormals ? 3 : 0;
	vertexSize += bHasUVs ? 2 : 0;
	vertices.reserve(totalVerts * vertexSize);
	verticesPositionOnly.reserve(totalVerts * 3);

	const float hwidth = 0.5f * width;
	const float hheight = 0.5f * height;
	const float hdepth = 0.5f * depth;

	auto InsertVertex = [&vertices, &verticesPositionOnly, bHasNormals, bHasUVs](float x, float y, float z, float nx, float ny, float nz, float u, float v)
	{
		// Position
		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);

		// Normal
		if (bHasNormals)
		{
			vertices.push_back(nx);
			vertices.push_back(ny);
			vertices.push_back(nz);
		}

		// UV
		if (bHasUVs)
		{
			vertices.push_back(u);
			vertices.push_back(v);
		}

		// Position only
		verticesPositionOnly.push_back(x);
		verticesPositionOnly.push_back(y);
		verticesPositionOnly.push_back(z);
	};

	// -X face
	InsertVertex(-hwidth, -hheight, -hdepth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	InsertVertex(-hwidth,  hheight, -hdepth, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	InsertVertex(-hwidth, -hheight,  hdepth, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	InsertVertex(-hwidth,  hheight,  hdepth, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	// +X face
	InsertVertex( hwidth, -hheight,  hdepth, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	InsertVertex( hwidth,  hheight,  hdepth, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	InsertVertex( hwidth, -hheight, -hdepth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	InsertVertex( hwidth,  hheight, -hdepth, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	// -Y face
	InsertVertex(-hwidth, -hheight, -hdepth, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
	InsertVertex(-hwidth, -hheight,  hdepth, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
	InsertVertex( hwidth, -hheight, -hdepth, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
	InsertVertex( hwidth, -hheight,  hdepth, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
	// +Y face
	InsertVertex( hwidth,  hheight, -hdepth, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f);
	InsertVertex( hwidth,  hheight,  hdepth, 0.0f,  1.0f, 0.0f, 1.0f, 1.0f);
	InsertVertex(-hwidth,  hheight, -hdepth, 0.0f,  1.0f, 0.0f, 0.0f, 0.0f);
	InsertVertex(-hwidth,  hheight,  hdepth, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f);
	// -Z face
	InsertVertex( hwidth, -hheight, -hdepth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	InsertVertex( hwidth,  hheight, -hdepth, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);
	InsertVertex(-hwidth, -hheight, -hdepth, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	InsertVertex(-hwidth,  hheight, -hdepth, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	// +Z face
	InsertVertex(-hwidth, -hheight,  hdepth, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	InsertVertex(-hwidth,  hheight,  hdepth, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	InsertVertex( hwidth, -hheight,  hdepth, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	InsertVertex( hwidth,  hheight,  hdepth, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	vector<uint16_t> indices = {
		1, 0, 3, 2, 0xFFFF,
		5, 4, 7, 6, 0xFFFF,
		9, 8, 11, 10, 0xFFFF,
		13, 12, 15, 14, 0xFFFF,
		17, 16, 19, 18, 0xFFFF,
		21, 20, 23, 22
	};

	auto model = make_shared<Model>();
	auto mesh = make_shared<Mesh>();

	assert(totalVerts == vertices.size() / vertexSize);
	mesh->m_vertexBuffer.Create("Box|VertexBuffer", totalVerts, stride, false, vertices.data());
	stride = 3 * sizeof(float);
	mesh->m_vertexBufferPositionOnly.Create("Box|VertexBuffer", totalVerts, stride, false, verticesPositionOnly.data());
	mesh->m_indexBuffer.Create("Box|IndexBuffer", indices.size(), sizeof(uint16_t), false, indices.data());

	mesh->m_boundingBox = Math::BoundingBoxFromMinMax(Math::Vector3(-hwidth, -hheight, -hdepth), Math::Vector3(hwidth, hheight, hdepth));
	model->m_boundingBox = mesh->m_boundingBox;

	MeshPart meshPart = {};
	meshPart.indexCount = uint32_t(indices.size());

	mesh->AddMeshPart(meshPart);
	model->AddMesh(mesh);

	return model;
}