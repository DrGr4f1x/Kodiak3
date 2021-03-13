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
class ColorBuffer;
class ConstantBuffer;
class DepthBuffer;
class RootSignature;
class StructuredBuffer;
class Texture;


class DescriptorSet
{
	friend class ComputeContext;
	friend class GraphicsContext;
	friend class ResourceSet;

public:
	void Init(const RootSignature& rootSig, int rootParam);

	bool IsInitialized() const { return m_bIsInitialized; }
	bool IsDirty() const { return m_bIsDirty; }

	void SetSRV(int paramIndex, const ColorBuffer& buffer);
	void SetSRV(int paramIndex, const DepthBuffer& buffer, bool depthSrv = true);
	void SetSRV(int paramIndex, const StructuredBuffer& buffer);
	void SetSRV(int paramIndex, const Texture& texture);

	void SetUAV(int paramIndex, const ColorBuffer& buffer);
	void SetUAV(int paramIndex, const DepthBuffer& buffer);
	void SetUAV(int paramIndex, const StructuredBuffer& buffer);
	void SetUAV(int paramIndex, const Texture& texture);

	void SetCBV(int paramIndex, const ConstantBuffer& buffer);

	void SetDynamicOffset(uint32_t offset);

private:
	void Update();

private:
	VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
	std::vector<VkWriteDescriptorSet> m_writeDescriptorSets;
	uint32_t m_dynamicOffset{ 0 };
	bool m_isDynamicCBV{ false };

	bool m_bIsRootCBV{ false };

	bool m_bIsInitialized{ false };
	bool m_bIsDirty{ false };
};

} // namespace Kodiak
