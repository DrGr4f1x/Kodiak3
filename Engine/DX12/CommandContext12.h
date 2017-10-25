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

#include "GpuBuffer12.h"
#include "LinearAllocator12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeContext;
class DepthBuffer;
class GraphicsContext;


class ContextManager
{
public:
	ContextManager() {}

	CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE type);
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
		assert_msg(m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE, "Cannot convert async compute context to graphics");
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

	static void InitializeBuffer(GpuResource& dest, const void* data, size_t numBytes, size_t offset = 0);

	void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
	void BeginResourceTransition(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool flushImmediate = false);
	void InsertUAVBarrier(GpuResource& resource, bool flushImmediate = false);
	void InsertAliasBarrier(GpuResource& before, GpuResource& after, bool flushImmediate = false);
	inline void FlushResourceBarriers();

protected:
	void SetID(const std::string& id) { m_id = id; }

protected:
	CommandListManager* m_owningManager;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12CommandAllocator* m_currentAllocator;

	D3D12_RESOURCE_BARRIER m_resourceBarrierBuffer[16];
	UINT m_numBarriersToFlush;

	ID3D12RootSignature* m_curGraphicsRootSignature;
	ID3D12PipelineState* m_curGraphicsPipelineState;
	ID3D12RootSignature* m_curComputeRootSignature;
	ID3D12PipelineState* m_curComputePipelineState;

	LinearAllocator m_cpuLinearAllocator;
	LinearAllocator m_gpuLinearAllocator;

	std::string m_id;

	D3D12_COMMAND_LIST_TYPE m_type;

private:
	CommandContext(D3D12_COMMAND_LIST_TYPE type);

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

	void SetRootSignature(const RootSignature& rootSig);

	void SetRenderTarget(const ColorBuffer& colorBuffer);
	void SetRenderTarget(const ColorBuffer& colorBuffer, const DepthBuffer& depthBuffer);

	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	void SetPipelineState(const GraphicsPSO& PSO);
	void SetConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer);

	void SetIndexBuffer(IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, VertexBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
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

	// TODO
#if 0
	m_DynamicViewDescriptorHeap.ParseGraphicsRootSignature(RootSig);
	m_DynamicSamplerDescriptorHeap.ParseGraphicsRootSignature(RootSig);
#endif
}


inline void GraphicsContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology)
{
	m_commandList->IASetPrimitiveTopology(topology);
}


inline void GraphicsContext::SetPipelineState(const GraphicsPSO& pso)
{
	auto pipelineState = pso.GetPipelineStateObject();
	if (pipelineState == m_curGraphicsPipelineState)
	{
		return;
	}

	m_commandList->SetPipelineState(pipelineState);
	m_curGraphicsPipelineState = pipelineState;
}


inline void GraphicsContext::SetConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer)
{
	m_commandList->SetGraphicsRootConstantBufferView(rootIndex, constantBuffer.RootConstantBufferView());
}


inline void GraphicsContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
	m_commandList->IASetIndexBuffer(&indexBuffer.GetIBV());
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, VertexBuffer& vertexBuffer)
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


inline void GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation)
{
	FlushResourceBarriers();
	m_commandList->DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

} // namespace Kodiak