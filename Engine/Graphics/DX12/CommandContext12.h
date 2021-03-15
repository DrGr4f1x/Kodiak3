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
#include "DWParam.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\PipelineState.h"
#include "Graphics\Texture.h"

#include "CommandListManager12.h"
#include "DescriptorSet12.h"
#include "DynamicDescriptorHeap12.h"
#include "LinearAllocator12.h"
#include "ResourceSet12.h"
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
class OcclusionQueryHeap;
class ReadbackBuffer;


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

	// Debug events and markers
	void BeginEvent(const std::string& label);
	void EndEvent();
	void SetMarker(const std::string& label);

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
	CommandListManager* m_owningManager{ nullptr };
	ID3D12GraphicsCommandList* m_commandList{ nullptr };
	ID3D12CommandAllocator* m_currentAllocator{ nullptr };

	ID3D12RootSignature* m_curGraphicsRootSignature{ nullptr };
	ID3D12PipelineState* m_curGraphicsPipelineState{ nullptr };
	ID3D12RootSignature* m_curComputeRootSignature{ nullptr };
	ID3D12PipelineState* m_curComputePipelineState{ nullptr };

	DynamicDescriptorHeap m_dynamicViewDescriptorHeap;
	DynamicDescriptorHeap m_dynamicSamplerDescriptorHeap;

	D3D12_RESOURCE_BARRIER m_resourceBarrierBuffer[16];
	uint32_t m_numBarriersToFlush;

	ID3D12DescriptorHeap* m_currentDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocator m_cpuLinearAllocator;
	LinearAllocator m_gpuLinearAllocator;

	D3D12_PRIMITIVE_TOPOLOGY m_curPrimitiveTopology;

	// Current render targets
	std::array<ColorBuffer*, 8>	m_renderTargets;
	DepthBuffer* m_depthTarget{ nullptr };

	std::string m_id;

	CommandListType m_type;

	bool m_hasPendingDebugEvent{ false };

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

	void BeginOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex);
	void EndOcclusionQuery(OcclusionQueryHeap& queryHeap, uint32_t heapIndex);
	void ResolveOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries, GpuResource& destBuffer, uint64_t destBufferOffset);
	void ResetOcclusionQueries(OcclusionQueryHeap& queryHeap, uint32_t startIndex, uint32_t numQueries) {}

	void SetRootSignature(const RootSignature& rootSig);

	void SetRenderTarget(const ColorBuffer& colorBuffer);
	void SetRenderTarget(const ColorBuffer& colorBuffer, const DepthBuffer& depthBuffer);

	void SetInvertedViewport(bool bInverted) { /* Unused, Vulkan-only */ }
	void SetViewport(float x, float y, float w, float h, float minDepth = 0.0f, float maxDepth = 1.0f);
	void SetScissor(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
	void SetViewportAndScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	void SetStencilRef(uint32_t stencilRef);
	void SetBlendFactor(Color blendFactor);
	void SetPrimitiveTopology(PrimitiveTopology topology);

	void SetPipelineState(const GraphicsPSO& PSO);

	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants);
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset);
	void SetConstant(uint32_t rootIndex, uint32_t offset, DWParam val);
	void SetConstants(uint32_t rootIndex, DWParam x);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z, DWParam w);
	void SetDescriptorSet(uint32_t rootIndex, DescriptorSet& descriptorSet);
	void SetResources(ResourceSet& resources);

	void SetSRV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv = true);
	void SetSRV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetSRV(int rootIndex, int offset, const Texture& texture);

	void SetUAV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const DepthBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const Texture& texture);

	void SetCBV(int rootIndex, int offset, const ConstantBuffer& buffer);

	void SetIndexBuffer(const IndexBuffer& indexBuffer);
	void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
	void SetVertexBuffer(uint32_t slot, const StructuredBuffer& vertexBuffer);
	void SetVertexBuffers(uint32_t startSlot, uint32_t count, VertexBuffer vertexBuffers[]);
	void SetDynamicVB(uint32_t slot, size_t numVertices, size_t vertexStride, const void* data);
	void SetDynamicVB(uint32_t slot, size_t numVertices, size_t vertexStride, DynAlloc vb);
	void SetDynamicIB(size_t indexCount, bool bIndexSize16Bit, const void* data);
	void SetDynamicIB(size_t indexCount, bool bIndexSize16Bit, DynAlloc ib);

	void Draw(uint32_t vertexCount, uint32_t vertexStartOffset = 0);
	void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation = 0, int32_t baseVertexLocation = 0);
	void DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount,
		uint32_t startVertexLocation = 0, uint32_t startInstanceLocation = 0);
	void DrawIndexedInstanced(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation,
		int32_t baseVertexLocation, uint32_t startInstanceLocation);

	void Resolve(ColorBuffer& src, ColorBuffer& dest, Format format);

private:
	void SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle);
};


class ComputeContext : public CommandContext
{
public:
	void SetRootSignature(const RootSignature& RootSig);

	void SetPipelineState(const ComputePSO& PSO);
	
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants);
	void SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset);
	void SetConstant(uint32_t rootIndex, uint32_t offset, DWParam val);
	void SetConstants(uint32_t rootIndex, DWParam x);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z);
	void SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z, DWParam w);
	void SetDescriptorSet(uint32_t rootIndex, DescriptorSet& descriptorSet);
	void SetResources(ResourceSet& resources);

	void SetSRV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv = true);
	void SetSRV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetSRV(int rootIndex, int offset, const Texture& texture);

	void SetUAV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const DepthBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetUAV(int rootIndex, int offset, const Texture& texture);

	void SetCBV(int rootIndex, int offset, const ConstantBuffer& buffer);

	void Dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
	void Dispatch1D(uint32_t threadCountX, uint32_t groupSizeX = 64);
	void Dispatch2D(uint32_t threadCountX, uint32_t threadCountY, uint32_t groupSizeX = 8, uint32_t groupSizeY = 8);
	void Dispatch3D(uint32_t threadCountX, uint32_t threadCountY, uint32_t threadCountZ, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ);

private:
	void SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle);
};


class ScopedDrawEvent
{
public:
	ScopedDrawEvent(CommandContext& context, const std::string& label)
		: m_context(context)
	{
		context.BeginEvent(label);
	}

	~ScopedDrawEvent()
	{
		m_context.EndEvent();
	}

private:
	CommandContext& m_context;
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


inline void GraphicsContext::SetBlendFactor(Color blendFactor)
{
	m_commandList->OMSetBlendFactor(blendFactor.GetPtr());
}


inline void GraphicsContext::SetPrimitiveTopology(PrimitiveTopology topology)
{
	auto newTopology = static_cast<D3D12_PRIMITIVE_TOPOLOGY>(topology);
	if (m_curPrimitiveTopology != newTopology)
	{
		m_commandList->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(newTopology));
		m_curPrimitiveTopology = newTopology;
	}
}


inline void GraphicsContext::SetPipelineState(const GraphicsPSO& pso)
{
	m_curComputePipelineState = nullptr;

	auto pipelineState = pso.GetPipelineStateObject();
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


inline void GraphicsContext::SetConstant(uint32_t rootIndex, uint32_t offset, DWParam val)
{
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, val._uint, offset);
}


inline void GraphicsContext::SetConstants(uint32_t rootIndex, DWParam x)
{
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, x._uint, 0);
}


inline void GraphicsContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y)
{
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, y._uint, 1);
}


inline void GraphicsContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z)
{
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, y._uint, 1);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, z._uint, 2);
}


inline void GraphicsContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z, DWParam w)
{
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, y._uint, 1);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, z._uint, 2);
	m_commandList->SetGraphicsRoot32BitConstant(rootIndex, w._uint, 3);
}


inline void GraphicsContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants)
{
	m_commandList->SetGraphicsRoot32BitConstants(rootIndex, numConstants, constants, 0);
}


inline void GraphicsContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset)
{
	m_commandList->SetGraphicsRoot32BitConstants(rootIndex, numConstants, constants, offset);
}


inline void GraphicsContext::SetDescriptorSet(uint32_t rootIndex, DescriptorSet& descriptorSet)
{
	if (!descriptorSet.IsInitialized())
		return;

	if (descriptorSet.IsDirty())
		descriptorSet.Update();

	if (descriptorSet.m_gpuDescriptor.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_commandList->SetGraphicsRootDescriptorTable((UINT)rootIndex, descriptorSet.m_gpuDescriptor);
	}
	else if (descriptorSet.m_gpuAddress != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_commandList->SetGraphicsRootConstantBufferView(
			(UINT)rootIndex,
			descriptorSet.m_gpuAddress + descriptorSet.m_dynamicOffset);
	}
}


inline void GraphicsContext::SetResources(ResourceSet& resources)
{
	ID3D12DescriptorHeap* heaps[] = {
		g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetHeapPointer(),
		g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].GetHeapPointer()
	};
	m_commandList->SetDescriptorHeaps(2, heaps);

	for (uint32_t i = 0; i < 8; ++i)
	{
		SetDescriptorSet(i, resources.m_descriptorSets[i]);
	}
}


inline void GraphicsContext::SetSRV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetSRV());
}


inline void GraphicsContext::SetSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv)
{
	SetDynamicDescriptor(rootIndex, offset, depthSrv ? buffer.GetDepthSRV() : buffer.GetStencilSRV());
}


inline void GraphicsContext::SetSRV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetSRV());
}


inline void GraphicsContext::SetSRV(int rootIndex, int offset, const Texture& texture)
{
	SetDynamicDescriptor(rootIndex, offset, texture.GetSRV());
}


inline void GraphicsContext::SetUAV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetUAV());
}


inline void GraphicsContext::SetUAV(int rootIndex, int offset, const DepthBuffer& buffer)
{
	assert_msg("Depth buffer UAVs not supported yet.");
}


inline void GraphicsContext::SetUAV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetUAV());
}


inline void GraphicsContext::SetUAV(int rootIndex, int offset, const Texture& texture)
{
	assert_msg("Texture UAVs not supported yet.");
}


inline void GraphicsContext::SetCBV(int rootIndex, int offset, const ConstantBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetCBV());
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


inline void GraphicsContext::SetVertexBuffer(uint32_t slot, const StructuredBuffer& vertexBuffer)
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
	m_commandList->IASetVertexBuffers(startSlot, count, vbv.data());
}


inline void GraphicsContext::SetDynamicVB(uint32_t slot, size_t numVertices, size_t vertexStride, const void* data)
{
	assert(data != nullptr && Math::IsAligned(data, 16));

	const size_t bufferSize = Math::AlignUp(numVertices * vertexStride, 16);
	DynAlloc vb = m_cpuLinearAllocator.Allocate(bufferSize);

	SIMDMemCopy(vb.dataPtr, data, bufferSize >> 4);

	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = vb.gpuAddress;
	vbv.SizeInBytes = (UINT)bufferSize;
	vbv.StrideInBytes = (UINT)vertexStride;

	m_commandList->IASetVertexBuffers(slot, 1, &vbv);
}


inline void GraphicsContext::SetDynamicVB(uint32_t slot, size_t numVertices, size_t vertexStride, DynAlloc vb)
{
	const size_t bufferSize = Math::AlignUp(numVertices * vertexStride, 16);

	D3D12_VERTEX_BUFFER_VIEW vbv{};
	vbv.BufferLocation = vb.gpuAddress;
	vbv.SizeInBytes = (UINT)bufferSize;
	vbv.StrideInBytes = (UINT)vertexStride;

	m_commandList->IASetVertexBuffers(slot, 1, &vbv);
}


inline void GraphicsContext::SetDynamicIB(size_t indexCount, bool bIndexSize16Bit, const void* data)
{
	assert(data != nullptr && Math::IsAligned(data, 16));

	const size_t elementSize = bIndexSize16Bit ? sizeof(uint16_t) : sizeof(uint32_t);

	const size_t bufferSize = Math::AlignUp(indexCount * elementSize, 16);
	DynAlloc ib = m_cpuLinearAllocator.Allocate(bufferSize);

	SIMDMemCopy(ib.dataPtr, data, bufferSize >> 4);

	D3D12_INDEX_BUFFER_VIEW ibv{};
	ibv.BufferLocation = ib.gpuAddress;
	ibv.SizeInBytes = (UINT)(indexCount * elementSize);
	ibv.Format = bIndexSize16Bit ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	m_commandList->IASetIndexBuffer(&ibv);
}


inline void GraphicsContext::SetDynamicIB(size_t indexCount, bool bIndexSize16Bit, DynAlloc ib)
{
	const size_t elementSize = bIndexSize16Bit ? sizeof(uint16_t) : sizeof(uint32_t);

	const size_t bufferSize = Math::AlignUp(indexCount * elementSize, 16);

	D3D12_INDEX_BUFFER_VIEW ibv{};
	ibv.BufferLocation = ib.gpuAddress;
	ibv.SizeInBytes = (UINT)(indexCount * elementSize);
	ibv.Format = bIndexSize16Bit ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

	m_commandList->IASetIndexBuffer(&ibv);
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


inline void GraphicsContext::SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	m_dynamicViewDescriptorHeap.SetGraphicsDescriptorHandles(rootIndex, offset, 1, &handle);
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

	auto pipelineState = pso.GetPipelineStateObject();
	if (pipelineState != m_curComputePipelineState)
	{
		m_commandList->SetPipelineState(pipelineState);
		m_curComputePipelineState = pipelineState;
	}
}


inline void ComputeContext::SetConstant(uint32_t rootIndex, uint32_t offset, DWParam val)
{
	m_commandList->SetComputeRoot32BitConstant(rootIndex, val._uint, offset);
}


inline void ComputeContext::SetConstants(uint32_t rootIndex, DWParam x)
{
	m_commandList->SetComputeRoot32BitConstant(rootIndex, x._uint, 0);
}


inline void ComputeContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y)
{
	m_commandList->SetComputeRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, y._uint, 1);
}


inline void ComputeContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z)
{
	m_commandList->SetComputeRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, y._uint, 1);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, z._uint, 2);
}


inline void ComputeContext::SetConstants(uint32_t rootIndex, DWParam x, DWParam y, DWParam z, DWParam w)
{
	m_commandList->SetComputeRoot32BitConstant(rootIndex, x._uint, 0);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, y._uint, 1);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, z._uint, 2);
	m_commandList->SetComputeRoot32BitConstant(rootIndex, w._uint, 3);
}


inline void ComputeContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants)
{
	m_commandList->SetComputeRoot32BitConstants(rootIndex, numConstants, constants, 0);
}


inline void ComputeContext::SetConstantArray(uint32_t rootIndex, uint32_t numConstants, const void* constants, uint32_t offset)
{
	m_commandList->SetComputeRoot32BitConstants(rootIndex, numConstants, constants, offset);
}


inline void ComputeContext::SetDescriptorSet(uint32_t rootIndex, DescriptorSet& descriptorSet)
{
	if (!descriptorSet.IsInitialized())
		return;

	if (descriptorSet.IsDirty())
		descriptorSet.Update();

	if (descriptorSet.m_gpuDescriptor.ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_commandList->SetComputeRootDescriptorTable((UINT)rootIndex, descriptorSet.m_gpuDescriptor);
	}
	else if (descriptorSet.m_gpuAddress != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_commandList->SetComputeRootConstantBufferView(
			(UINT)rootIndex,
			descriptorSet.m_gpuAddress + descriptorSet.m_dynamicOffset);
	}
}


inline void ComputeContext::SetResources(ResourceSet& resources)
{
	D3D12_DESCRIPTOR_HEAP_TYPE heapTypes[] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	};
	ID3D12DescriptorHeap* heaps[] = 
	{
		g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].GetHeapPointer(),
		g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].GetHeapPointer(),
	};
	SetDescriptorHeaps(2, heapTypes, heaps);

	for (uint32_t i = 0; i < 8; ++i)
	{
		SetDescriptorSet(i, resources.m_descriptorSets[i]);
	}
}


inline void ComputeContext::SetSRV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetSRV());
}


inline void ComputeContext::SetSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv)
{
	SetDynamicDescriptor(rootIndex, offset, depthSrv ? buffer.GetDepthSRV() : buffer.GetStencilSRV());
}


inline void ComputeContext::SetSRV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetSRV());
}


inline void ComputeContext::SetSRV(int rootIndex, int offset, const Texture& texture)
{
	SetDynamicDescriptor(rootIndex, offset, texture.GetSRV());
}


inline void ComputeContext::SetUAV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetUAV());
}


inline void ComputeContext::SetUAV(int rootIndex, int offset, const DepthBuffer& buffer)
{
	assert_msg("Depth buffer UAVs not supported yet.");
}


inline void ComputeContext::SetUAV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetUAV());
}


inline void ComputeContext::SetUAV(int rootIndex, int offset, const Texture& texture)
{
	assert_msg("Texture UAVs not supported yet.");
}


inline void ComputeContext::SetCBV(int rootIndex, int offset, const ConstantBuffer& buffer)
{
	SetDynamicDescriptor(rootIndex, offset, buffer.GetCBV());
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


inline void ComputeContext::SetDynamicDescriptor(uint32_t rootIndex, uint32_t offset, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	m_dynamicViewDescriptorHeap.SetComputeDescriptorHandles(rootIndex, offset, 1, &handle);
}

} // namespace Kodiak