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

class ColorBuffer;
class ConstantBuffer;
class DepthBuffer;
class RootSignature;
class StructuredBuffer;
class Texture;


class ResourceSet
{
	friend class ComputeContext;
	friend class GraphicsContext;

public:

	void Init(const RootSignature* rootSig);
	void Finalize();

	void SetSRV(int rootIndex, int paramIndex, const ColorBuffer& buffer);
	void SetSRV(int rootIndex, int paramIndex, const DepthBuffer& buffer, bool depthSrv = true);
	void SetSRV(int rootIndex, int paramIndex, const StructuredBuffer& buffer);
	void SetSRV(int rootIndex, int paramIndex, const Texture& texture);

	void SetUAV(int rootIndex, int paramIndex, const ColorBuffer& buffer);
	void SetUAV(int rootIndex, int paramIndex, const DepthBuffer& buffer);
	void SetUAV(int rootIndex, int paramIndex, const StructuredBuffer& buffer);
	void SetUAV(int rootIndex, int paramIndex, const Texture& texture);

	void SetCBV(int rootIndex, int paramIndex, const ConstantBuffer& buffer);

	void SetDynamicOffset(int rootIndex, uint32_t offset);

private:
	struct ResourceTable
	{
		VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		int rootIndex{ -1 };
		bool isDynamicCBV{ false };
	};
	std::array<ResourceTable, 8> m_resourceTables;
	std::array<uint32_t, 8> m_dynamicOffsets;
	VkDescriptorSet m_staticSamplers{ VK_NULL_HANDLE };
	int m_staticSamplerIndex{ -1 };

	const RootSignature* m_rootSig{ nullptr };
};

} // namespace Kodiak