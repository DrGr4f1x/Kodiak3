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

#include "Application.h"
#include "GpuBuffer.h"

class TriangleApp : public Kodiak::Application
{
public:
	TriangleApp() : Application("Triangle") {}

	void Configure() final;
	void Startup() final;
	void Shutdown() final;

	void Render() final;

private:
	void UpdateConstantBuffer();
#if VK
	void InitVk();
	void InitRenderPass();
	void InitDepthStencil();
	void InitFramebuffers();
	void InitDescriptorPool();
	void InitDescriptorSetLayout();
	void InitDescriptorSet();
	void InitPipelineCache();
	void InitPipeline();

	void ShutdownVk();
#endif

private:
	// Vertex layout used in this example
	struct Vertex 
	{
		float position[3];
		float color[3];
	};

	// Vertex buffer and attributes
	Kodiak::VertexBuffer m_vertexBuffer;

	// Index buffer
	Kodiak::IndexBuffer m_indexBuffer;

	// Uniform buffer block object
	Kodiak::ConstantBuffer m_constantBuffer;

	struct
	{
		Math::Matrix4 projectionMatrix;
		Math::Matrix4 modelMatrix;
		Math::Matrix4 viewMatrix;
	} m_vsConstants;

	// Camera controls
	float m_zoom{ -2.5f };

#if VK
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };
	std::vector<VkFramebuffer> m_framebuffers;
	VkImage m_depthStencilImage{ VK_NULL_HANDLE };
	VkDeviceMemory m_depthStencilMem{ VK_NULL_HANDLE };
	VkImageView m_depthStencilView{ VK_NULL_HANDLE };
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline m_pipeline{ VK_NULL_HANDLE };
	VkPipelineCache m_pipelineCache{ VK_NULL_HANDLE };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
	VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
	VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };
#endif
};