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
#include "ColorBuffer12.h"
#include "DepthBuffer12.h"
#include "DynamicDescriptorHeap12.h"
#include "GpuBuffer12.h"
#include "LinearAllocator12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"
#include "Texture12.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeContext;
class DepthBuffer;
class FrameBuffer;
class GraphicsContext;
class RenderPass;


class ContextManager
{
public:
	ContextManager() {}

	CommandContext* AllocateContext(CommandListType type);
	void FreeContext(CommandContext*);
	void DestroyAllContexts();

private:
	std::vector<std::unique_ptr<CommandContext> > sm_contextPool[4];
	std::queue<CommandContext*> sm_availableContexts[4];
	std::mutex sm_contextAllocationMutex;
};


class CommandContext : public NonCopyable
{
	friend ContextManager;

public:
	~CommandContext();

	static void DestroyAllContexts();

	static CommandContext& Begin(const std::string id = "");

	// Flush existing commands and release the current context
	uint64_t Finish(bool waitForCompletion = false);

	// Prepare to render by reserving a command list
	void Initialize();

	GraphicsContext& GetGraphicsContext() 
	{
		assert_msg(m_type != CommandListType::Compute, "Cannot convert async compute context to graphics");
		return reinterpret_cast<GraphicsContext&>(*this);
	}

	ComputeContext& GetComputeContext() 
	{
		return reinterpret_cast<ComputeContext&>(*this);
	}

	DynAlloc ReserveUploadMemory(size_t sizeInBytes)
	{
		return m_cpuLinearAllocator.Allocate(sizeInBytes);
	}

	static void InitializeTexture(GpuResource& dest, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA subData[]);
	static void InitializeBuffer(GpuResource& dest, const void* data, size_t numBytes, size_t offset = 0);

	void TransitionResource(GpuResource& resource, ResourceState newState, bool flushImmediate = false);
	void BeginResourceTransition(GpuResource& resource, ResourceState newState, bool flushImmediate = false);
	void InsertUAVBarrier(GpuResource& resource, bool flushImmediate = false);
	void InsertAliasBarrier(GpuResource& before, GpuResource& after, bool flushImmediate = false);
	inline void FlushResourceBarriers();

	void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr);
	void SetDescriptorHeaps(uint32_t heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[]);

protected:
	void SetID(const std::string& id) { m_id = id; }

	void BindDescriptorHeaps();

protected:
	CommandListManager* m_owningManager;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12CommandAllocator* m_currentAllocator;

	ID3D12RootSignature* m_curGraphicsRootSignature;
	ID3D12PipelineState* m_curGraphicsPipelineState;
	ID3D12RootSignature* m_curComputeRootSignature;
	ID3D12PipelineState* m_curComputePipelineState;

	DynamicDescriptorHeap m_dynamicViewDescriptorHeap;		// HEAP_TYPE_CBV_SRV_UAV
	DynamicDescriptorHeap m_dynamicSamplerDescriptorHeap;	// HEAP_TYPE_SAMPLER

	D3D12_RESOURCE_BARRIER m_resourceBarrierBuffer[16];
	uint32_t m_numBarriersToFlush;

	ID3D12DescriptorHeap* m_currentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator m_cpuLinearAllocator;
	LinearAllocator m_gpuLinearAllocator;

	D3D12_PRIMITIVE_TOPOLOGY m_curPrimitiveTopology;

	// Current render targets
	std::array<ColorBuffer*, 8> m_renderTargets;
	std::array<ResourceState, 8> m_renderTargetStates;
	DepthBuffer* m_depthTarget;
	ResourceState m_depthTargetState;
	bool m_depthTargetValid{ false };
	uint32_t m_numRenderTargets{ 0 };

	std::string m_id;

	CommandListType m_type;

private:
	CommandContext(CommandListType type);

	void Reset();
};


inline void CommandContext::FlushResourceBarriers()
{
	if (m_numBarriersToFlush > 0)
	{
		m_commandList->ResourceBarrier(m_numBarriersToFlush, m_resourceBarrierBuffer);
		m_numBarriersToFlush = 0;
	}
}


inline void CommandContext::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, ID3D12DescriptorHeap* heapPtr)
{
	if (m_currentDescriptorHeaps[type] != heapPtr)
	{
		m_currentDescriptorHeaps[type] = heapPtr;
		BindDescriptorHeaps();
	}
}


inline void CommandContext::SetDescriptorHeaps(uint32_t heapCount, D3D12_DESCRIPTOR_HEAP_TYPE type[], ID3D12DescriptorHeap* heapPtrs[])
{
	bool anyChanged = false;

	for (uint32_t i = 0; i < heapCount; ++i)
	{
		if (m_currentDescriptorHeaps[type[i]] != heapPtrs[i])
		{
			m_currentDescriptorHeaps[type[i]] = heapPtrs[i];
			anyChanged = true;
		}
	}

	if (anyChanged)
	{
		BindDescriptorHeaps();
	}
}


class GraphicsContext : public CommandContext
{
public:

	static GraphicsContext& Begin(const std::string& id = "")
	{
		return CommandContext::Begin(id).GetGraphicsContext();
	}

	void ClearColor(ColorBuffer& target);
	void ClearDepth(DepthBuffer& target);
	void ClearStencil(DepthBuffer& target);
	void ClearDepthAndStencil(DepthBuffer& target);

	void BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor);
	void BeginRenderPass(RenderPass& pass, FrameBuffer& framebuffer, Color& clearColor, float clearDepth, uint32_t clearStencil);
	void EndRenderPass();

	void SetRootSignature(const RootSignature& rootSig);

	void SetRenderTarget(const ColorBuffer& colorBuffer);
	void SetRenderTarget(const ColorBuffer& colorBuffer, const DepthBuffer& depthBuffer);

	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);

	void SetPipelineState(const GraphicsPSO& PSO);
	void SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer);
	void SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer);

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);
};


inline void GraphicsContext::SetRootSignature(const RootSignature& rootSig)
{
	auto signature = rootSig.GetSignature();
	if (signature == m_curGraphicsRootSignature)
	{
		return;
	}

	m_commandList->SetGraphicsRootSignature(signature);
	m_curGraphicsRootSignature = signature;

	m_dynamicViewDescriptorHeap.ParseGraphicsRootSignature(rootSig);
	m_dynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(rootSig);
}


inline void GraphicsContext::SetStencilRef(uint32_t stencilRef)
{
	m_commandList->OMSetStencilRef(stencilRef);
}


inline void GraphicsContext::SetPipelineState(const GraphicsPSO& pso)
{
	auto pipelineState = pso.GetPipelineStateObject();
	if (pipelineState != m_curGraphicsPipelineState)
	{
		m_commandList->SetPipelineState(pipelineState);
		m_curGraphicsPipelineState = pipelineState;
	}

	auto topology = pso.GetTopology();
	if (topology != m_curPrimitiveTopology)
	{
		m_commandList->IASetPrimitiveTopology(topology);
		m_curPrimitiveTopology = topology;
	}
}


inline void GraphicsContext::SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer)
{
	m_commandList->SetGraphicsRootConstantBufferView(rootIndex, constantBuffer.RootConstantBufferView());
}


inline void GraphicsContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &constantBuffer.GetCBV());
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &texture.GetSRV());
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &colorBuffer.GetSRV());
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
{
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &depthBuffer.GetDepthSRV());
}


inline void GraphicsContext::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
	m_commandList->IASetIndexBuffer(&indexBuffer.GetIBV());
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer)
{
	D3D12_VERTEX_BUFFER_VIEW vbv[1] = { vertexBuffer.GetVBV() };
	m_commandList->IASetVertexBuffers(slot, 1, vbv);
}


inline void GraphicsContext::SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[])
{
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbv(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		vbv[i] = vertexBuffers[i].GetVBV();
	}
	m_commandList->IASetVertexBuffers(startSlot, 1, vbv.data());
}


inline void GraphicsContext::Draw(uint32_t vertexCount, uint32_t vertexStartOffset)
{
	DrawInstanced(vertexCount, 1, vertexStartOffset, 0);
}


inline void GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}


inline void GraphicsContext::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
	uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	m_dynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_dynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_commandList->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}


inline void GraphicsContext::DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
	int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	FlushResourceBarriers();
	m_dynamicViewDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_dynamicSamplerDescriptorHeap.CommitGraphicsRootDescriptorTables(m_commandList);
	m_commandList->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

} // namespace Kodiak