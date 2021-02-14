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

}