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

#include "TriangleApp.h"

#include "Color.h"
#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Shader.h"
#include "SwapChain.h"


using namespace Kodiak;
using namespace std;


void TriangleApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	auto binDir = filesystem.GetBinaryDir();
	string rootDir = binDir;
	auto pos = binDir.find("Bin");
	if (pos != binDir.npos)
	{
		rootDir = binDir.substr(0, pos);
	}
	filesystem.SetRootDir(rootDir);

#if defined(DX12)
	filesystem.AddSearchPath("Data\\Shaders\\DXIL");
#elif defined(VK)
	filesystem.AddSearchPath("Data\\Shaders\\SPIR-V");
#else
#error No graphics API defined!
#endif
}


void TriangleApp::Startup()
{
	// Setup vertices
	vector<Vertex> vertexData =
	{
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
		{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
	};
	m_vertexBuffer.Create("Vertex buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	// Setup indices
	vector<uint32_t> indexData = { 0, 1, 2 };
	m_indexBuffer.Create("Index buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	// Setup constant buffer
	m_constantBuffer.Create("Constant buffer", 1, sizeof(m_vsConstants));

	UpdateConstantBuffer();

	auto swapChain = m_graphicsDevice->GetSwapChain();

	// Setup render pass
	auto colorFormat = swapChain->GetColorFormat();
	auto depthFormat = m_graphicsDevice->GetDepthFormat();
	m_renderPass.AddColorAttachment(colorFormat, ResourceState::Undefined, ResourceState::Present);
	m_renderPass.AddDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_renderPass.Finalize();

	// Depth stencil buffer
	m_depthBuffer = make_shared<DepthBuffer>(1.0f);
	m_depthBuffer->Create("Depth Buffer", m_displayWidth, m_displayHeight, m_graphicsDevice->GetDepthFormat());

	// Framebuffers
	const uint32_t imageCount = swapChain->GetImageCount();
	m_framebuffers.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		m_framebuffers[i] = make_shared<FrameBuffer>();
		m_framebuffers[i]->Create(swapChain->GetColorBuffer(i), m_depthBuffer, m_renderPass);
	}

#if VK
	InitVk();
#endif

#if DX12
	InitDX12();
#endif

	InitPSO();
}


void TriangleApp::Shutdown()
{
	m_vertexBuffer.Destroy();
	m_indexBuffer.Destroy();
	m_constantBuffer.Destroy();
	m_depthBuffer.reset();
	m_renderPass.Destroy();
	
	m_framebuffers.clear();

#if VK
	ShutdownVk();
#endif
}


void TriangleApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();
	
	Color clearColor{ DirectX::Colors::CornflowerBlue };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

#if VK
	context.BindDescriptorSet(m_pipelineLayout, m_descriptorSet);
#endif

#if DX12
	context.SetRootSignature(m_rootSig);
	context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context.SetConstantBuffer(0, m_constantBuffer);
#endif

	context.SetPipelineState(m_pso);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndRenderPass();

#if VK
	context.Finish(m_graphicsDevice->GetPresentCompleteSemaphore(), m_graphicsDevice->GetRenderCompleteSemaphore());
#elif DX12
	context.Finish();
#endif
}


void TriangleApp::UpdateConstantBuffer()
{
	// Update matrices
	m_vsConstants.projectionMatrix = Math::Matrix4::MakePerspective(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayWidth / (float)m_displayHeight,
		0.1f,
		256.0f);

	m_vsConstants.viewMatrix = Math::AffineTransform::MakeTranslation(Math::Vector3(0.0f, 0.0f, m_zoom));

	m_vsConstants.modelMatrix = Math::Matrix4(Math::kIdentity);

	m_constantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);
}


#if VK
void TriangleApp::InitVk()
{
	InitDescriptorPool();
	InitDescriptorSetLayout();
	InitDescriptorSet();
}


void TriangleApp::InitDescriptorPool()
{
	// We need to tell the API the number of max. requested descriptors per type
	VkDescriptorPoolSize typeCounts[1];
	// This example only uses one descriptor type (uniform buffer) and only requests one descriptor of this type
	typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCounts[0].descriptorCount = 1;
	// For additional types you need to add new entries in the type count list
	// E.g. for two combined image samplers :
	// typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	// typeCounts[1].descriptorCount = 2;

	// Create the global descriptor pool
	// All descriptors used in this example are allocated from this pool
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = typeCounts;
	// Set the max. number of descriptor sets that can be requested from this pool (requesting beyond this limit will result in an error)
	descriptorPoolInfo.maxSets = 1;

	ThrowIfFailed(vkCreateDescriptorPool(GetDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool));
}


void TriangleApp::InitDescriptorSetLayout()
{
	// Setup layout of descriptors used in this example
	// Basically connects the different shader stages to descriptors for binding uniform buffers, image samplers, etc.
	// So every shader binding should map to one descriptor set layout binding

	// Binding 0: Uniform buffer (Vertex shader)
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
	descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayout.pNext = nullptr;
	descriptorLayout.bindingCount = 1;
	descriptorLayout.pBindings = &layoutBinding;

	ThrowIfFailed(vkCreateDescriptorSetLayout(GetDevice(), &descriptorLayout, nullptr, &m_descriptorSetLayout));

	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = 1;
	pPipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;

	ThrowIfFailed(vkCreatePipelineLayout(GetDevice(), &pPipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
}


void TriangleApp::InitDescriptorSet()
{
	// Allocate a new descriptor set from the global descriptor pool
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_descriptorSetLayout;

	ThrowIfFailed(vkAllocateDescriptorSets(GetDevice(), &allocInfo, &m_descriptorSet));

	// Update the descriptor set determining the shader binding points
	// For every binding point used in a shader there needs to be one
	// descriptor set matching that binding point

	VkWriteDescriptorSet writeDescriptorSet = {};

	// Binding 0 : Uniform buffer
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	VkDescriptorBufferInfo bufferInfo = m_constantBuffer.GetDescriptorInfo();
	writeDescriptorSet.pBufferInfo = &bufferInfo;
	// Binds this uniform buffer to binding point 0
	writeDescriptorSet.dstBinding = 0;

	vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
}


void TriangleApp::ShutdownVk()
{
	vkDestroyDescriptorPool(GetDevice(), m_descriptorPool, nullptr);
	m_descriptorPool = VK_NULL_HANDLE;

	vkDestroyPipelineLayout(GetDevice(), m_pipelineLayout, nullptr);
	m_pipelineLayout = VK_NULL_HANDLE;

	vkDestroyDescriptorSetLayout(GetDevice(), m_descriptorSetLayout, nullptr);
	m_descriptorSetLayout = VK_NULL_HANDLE;
}
#endif


#if DX12
void TriangleApp::InitDX12()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
	m_rootSig.Finalize("Root sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
}
#endif


void TriangleApp::InitPSO()
{
#if DX12
	m_pso.SetRootSignature(m_rootSig);
#else
	m_pso.SetPipelineLayout(m_pipelineLayout);
#endif

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	auto vs = Shader::Load("TriangleVS");
	auto ps = Shader::Load("TrianglePS");

	m_pso.SetVertexShader(vs);
	m_pso.SetPixelShader(ps);

	m_pso.SetRenderPass(m_renderPass);

	m_pso.SetPrimitiveTopologyType(PrimitiveTopologyType::Triangle);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "Position", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "Color", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, 2, vertexElements);

	m_pso.Finalize();
}