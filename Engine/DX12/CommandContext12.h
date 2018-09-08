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
#include "GpuBuffer.h"

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "PipelineState.h"
#include "Texture.h"

#include "DynamicDescriptorHeap12.h"
#include "LinearAllocator12.h"

#include "RootSignature12.h"


namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandListManager;
class ComputeContext;
class DepthBuffer;
class FrameBuffer;
class GraphicsContext;


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
	std::array<ColorBuffer*, 8>		m_renderTargets;
	DepthBuffer* m_depthTarget;

	std::string m_id;

	CommandListType m_type;

private:
	CommandContext(CommandListType type);

	void Reset();
};


class GraphicsContext : public CommandContext
{
public:

	static GraphicsContext& Begin(const std::string& id = "")
	{
		return CommandContext::Begin(id).GetGraphicsContext();
	}

	void ClearColor(ColorBuffer& target);
	void ClearColor(ColorBuffer& target, Color clearColor);
	void ClearDepth(DepthBuffer& target);
	void ClearStencil(DepthBuffer& target);
	void ClearDepthAndStencil(DepthBuffer& target);

	void BeginRenderPass(FrameBuffer& framebuffer);
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
	void SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);

	void Resolve(ColorBuffer& src, ColorBuffer& dest, Format format);
};


class ComputeContext : public CommandContext
{
public:
	void SetRootSignature(const RootSignature& RootSig);

	void SetPipelineState(const ComputePSO& PSO);
	void SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer);
	void SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer);
	void SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);
	void SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer);
	void SetUAV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer);

	void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
	void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);
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


inline void GraphicsContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curComputeRootSignature = nullptr;

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
	m_curComputePipelineState = nullptr;

	auto pipelineState = pso.GetHandle();
	if (pipelineState != m_curGraphicsPipelineState)
	{
		m_commandList->SetPipelineState(pipelineState);
		m_curGraphicsPipelineState = pipelineState;
	}

	auto topology = static_cast<D3D12_PRIMITIVE_TOPOLOGY>(pso.GetTopology());
	if (topology != m_curPrimitiveTopology)
	{
		m_commandList->IASetPrimitiveTopology(topology);
		m_curPrimitiveTopology = topology;
	}
}


inline void GraphicsContext::SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer)
{
	m_commandList->SetGraphicsRootConstantBufferView(rootIndex, constantBuffer.GetGpuAddress());
}


inline void GraphicsContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handles[] = { constantBuffer.GetCBV().GetHandle() };
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, handles);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = colorBuffer.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = depthBuffer.GetDepthSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = buffer.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void GraphicsContext::SetIndexBuffer(const IndexBuffer& indexBuffer)
{
	m_commandList->IASetIndexBuffer(&indexBuffer.GetIBV().GetHandle());
}


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer)
{
	D3D12_VERTEX_BUFFER_VIEW vbv[1] = { vertexBuffer.GetVBV().GetHandle() };
	m_commandList->IASetVertexBuffers(slot, 1, vbv);
}


inline void GraphicsContext::SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[])
{
	std::vector<D3D12_VERTEX_BUFFER_VIEW> vbv(count);
	for (uint32_t i = 0; i < count; ++i)
	{
		vbv[i] = vertexBuffers[i].GetVBV().GetHandle();
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


inline void GraphicsContext::Resolve(ColorBuffer& src, ColorBuffer& dest, Format format)
{
	FlushResourceBarriers();
	auto dxFormat = static_cast<DXGI_FORMAT>(format);
	m_commandList->ResolveSubresource(dest.m_resource.Get(), 0, src.m_resource.Get(), 0, dxFormat);
}


inline void ComputeContext::SetRootSignature(const RootSignature& rootSig)
{
	m_curGraphicsRootSignature = nullptr;

	auto signature = rootSig.GetSignature();
	if (signature == m_curComputeRootSignature)
	{
		return;
	}

	m_commandList->SetComputeRootSignature(signature);
	m_curComputeRootSignature = signature;

	m_dynamicViewDescriptorHeap.ParseComputeRootSignature(rootSig);
	m_dynamicSamplerDescriptorHeap.ParseComputeRootSignature(rootSig);
}


inline void ComputeContext::SetPipelineState(const ComputePSO& pso)
{
	m_curGraphicsPipelineState = nullptr;

	auto pipelineState = pso.GetHandle();
	if (pipelineState != m_curComputePipelineState)
	{
		m_commandList->SetPipelineState(pipelineState);
		m_curComputePipelineState = pipelineState;
	}
}


inline void ComputeContext::SetRootConstantBuffer(uint32_t rootIndex, const ConstantBuffer& constantBuffer)
{
	m_commandList->SetComputeRootConstantBufferView(rootIndex, constantBuffer.GetGpuAddress());
}


inline void ComputeContext::SetConstantBuffer(uint32_t rootIndex, uint32_t offset, const ConstantBuffer& constantBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handles[] = { constantBuffer.GetCBV().GetHandle() };
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, handles);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const Texture& texture)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = texture.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = colorBuffer.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const DepthBuffer& depthBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = depthBuffer.GetDepthSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetSRV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = buffer.GetSRV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetUAV(uint32_t rootIndex, uint32_t offset, const ColorBuffer& colorBuffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = colorBuffer.GetUAV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::SetUAV(uint32_t rootIndex, uint32_t offset, const GpuBuffer& buffer)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle = buffer.GetUAV().GetHandle();
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}


inline void ComputeContext::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	FlushResourceBarriers();
	m_dynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(m_commandList);
	m_dynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(m_commandList);
	m_commandList->Dispatch(groupCountX, groupCountY, groupCountZ);
}


inline void ComputeContext::Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX)
{
	Dispatch(Math::DivideByMultiple(threadCountX, groupSizeX), 1, 1);
}


inline void ComputeContext::Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX, uint32_t groupSizeY)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY), 1);
}


inline void ComputeContext::Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ)
{
	Dispatch(
		Math::DivideByMultiple(threadCountX, groupSizeX),
		Math::DivideByMultiple(threadCountY, groupSizeY),
		Math::DivideByMultiple(threadCountZ, groupSizeZ));
}

} // namespace Kodiak