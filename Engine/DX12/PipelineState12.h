//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

namespace Kodiak
{

// Forward declarations
class RootSignature;
class Shader;


class PSO
{
public:
	static void DestroyAll();

	void SetRootSignature(const RootSignature& bindMappings)
	{
		m_rootSignature = &bindMappings;
	}

	const RootSignature& GetRootSignature() const
	{
		assert(m_rootSignature != nullptr);
		return *m_rootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject() const { return m_pso; }

protected:
	const RootSignature* m_rootSignature{ nullptr };
	ID3D12PipelineState* m_pso{ nullptr };
};


class GraphicsPSO : public PSO
{
	friend class CommandContext;

public:
	// Start with empty state
	GraphicsPSO();

	void SetBlendState(const D3D12_BLEND_DESC& blendDesc);
	void SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc);
	void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc);
	void SetSampleMask(uint32_t sampleMask);
	void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);
	void SetRenderTargetFormat(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat, uint32_t msaaCount = 1, uint32_t msaaQuality = 0);
	void SetRenderTargetFormats(uint32_t numRTVs, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat, uint32_t msaaCount = 1, uint32_t msaaQuality = 0);
	void SetInputLayout(uint32_t numElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs);
	void SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibProps);

	// These const_casts shouldn't be necessary, but we need to fix the API to accept "const void* pShaderBytecode"
	void SetVertexShader(const void* binary, size_t size) { m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetPixelShader(const void* binary, size_t size) { m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetGeometryShader(const void* binary, size_t size) { m_psoDesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetHullShader(const void* binary, size_t size) { m_psoDesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetDomainShader(const void* binary, size_t size) { m_psoDesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }

	void SetVertexShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.VS = binary; }
	void SetPixelShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.PS = binary; }
	void SetGeometryShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.GS = binary; }
	void SetHullShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.HS = binary; }
	void SetDomainShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.DS = binary; }

	void SetVertexShader(const Shader* vertexShader);
	void SetPixelShader(const Shader* pixelShader);
	void SetGeometryShader(const Shader* geometryShader);
	void SetHullShader(const Shader* hullShader);
	void SetDomainShader(const Shader* domainShader);

	// Perform validation and compute a hash value for fast state block comparisons
	void Finalize();

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> m_inputLayouts;
};


class ComputePSO : public PSO
{
	friend class CommandContext;

public:
	ComputePSO();

	void SetComputeShader(const void* binary, size_t size) { m_psoDesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<void*>(binary), size); }
	void SetComputeShader(const D3D12_SHADER_BYTECODE& binary) { m_psoDesc.CS = binary; }
	void SetComputeShader(const Shader* computeShader);

	void Finalize();

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
};

} // namespace Kodiak