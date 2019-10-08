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

#include "UtilVk.h"

#include "GraphicsDevice.h"

#include <unordered_map>


using namespace std;


namespace Kodiak
{

VkImageCreateInfo DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
	uint32_t numSamples, Format format, VkImageUsageFlags usageFlags)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = static_cast<VkFormat>(format);
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = numMips;
	imageCreateInfo.arrayLayers = depthOrArraySize;
	imageCreateInfo.samples = SamplesToFlags(numSamples);
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usageFlags;
	imageCreateInfo.flags = 0;

	return imageCreateInfo;
}


ResourceHandle CreateTextureResource(const string& name, const VkImageCreateInfo& imageCreateInfo)
{
	VkDevice device = GetDevice();

	VkImage image{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &mem));
	ResourceHandle handle = ResourceHandle::Create(image, mem);

	ThrowIfFailed(vkBindImageMemory(device, image, handle, 0));

	SetDebugName(image, name + " image");
	SetDebugName(mem, name + " memory");

	return handle;
}


VkSampleCountFlagBits Kodiak::SamplesToFlags(uint32_t numSamples)
{
	switch (numSamples)
	{
	case 1:
		return VK_SAMPLE_COUNT_1_BIT;
	case 2:
		return VK_SAMPLE_COUNT_2_BIT;
	case 4:
		return VK_SAMPLE_COUNT_4_BIT;
	case 8:
		return VK_SAMPLE_COUNT_8_BIT;
	case 16:
		return VK_SAMPLE_COUNT_16_BIT;
	case 32:
		return VK_SAMPLE_COUNT_32_BIT;
	case 64:
		return VK_SAMPLE_COUNT_64_BIT;
	default:
		assert(false);
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

VkImageType GetImageType(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
	case ResourceType::Texture1D_Array:
		return VK_IMAGE_TYPE_1D;

	case ResourceType::Texture2D:
	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS:
	case ResourceType::Texture2DMS_Array:
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		return VK_IMAGE_TYPE_2D;

	case ResourceType::Texture3D:
		return VK_IMAGE_TYPE_3D;

	default:
		assert(false);
		return VK_IMAGE_TYPE_2D;
	}
}


VkImageViewType GetImageViewType(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
		return VK_IMAGE_VIEW_TYPE_1D;

	case ResourceType::Texture1D_Array:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;

	case ResourceType::Texture2D:
	case ResourceType::Texture2DMS:
		return VK_IMAGE_VIEW_TYPE_2D;

	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS_Array:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	case ResourceType::TextureCube:
		return VK_IMAGE_VIEW_TYPE_CUBE;

	case ResourceType::TextureCube_Array:
		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

	case ResourceType::Texture3D:
		return VK_IMAGE_VIEW_TYPE_3D;

	default:
		assert(false);
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}


VkImageCreateFlagBits GetImageCreateFlags(ResourceType type)
{
	switch (type)
	{
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	default:
		return static_cast<VkImageCreateFlagBits>(0);
	}
}


VkImageLayout GetImageLayout(ResourceState state)
{
	static const auto s_invalidLayout = VkImageLayout(-1);
	
	switch (state)
	{
	case ResourceState::Undefined: 
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case ResourceState::Common: 
		return VK_IMAGE_LAYOUT_GENERAL;
	case ResourceState::VertexBuffer:
	case ResourceState::IndexBuffer:
	case ResourceState::ConstantBuffer:
		return s_invalidLayout;
	case ResourceState::RenderTarget:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case ResourceState::UnorderedAccess:
		return VK_IMAGE_LAYOUT_GENERAL;
	case ResourceState::DepthRead:
	case ResourceState::DepthWrite:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ResourceState::NonPixelShaderResource:
	case ResourceState::PixelShaderResource:
	case ResourceState::ShaderResource:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ResourceState::StreamOut:
	case ResourceState::IndirectArgument:
		return s_invalidLayout;
	case ResourceState::CopyDest:
	case ResourceState::ResolveDest:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case ResourceState::CopySource:
	case ResourceState::ResolveSource:
	case ResourceState::GenericRead:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case ResourceState::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case ResourceState::Predication:
		return s_invalidLayout;
	case ResourceState::PreInitialized:
		return VK_IMAGE_LAYOUT_PREINITIALIZED;

	default:
		assert(false);
		return s_invalidLayout;
	}

}


VkAccessFlagBits GetAccessMask(ResourceState state)
{
	static const auto s_invalidFlags = VkAccessFlagBits(-1);

	switch (state)
	{
	case ResourceState::Undefined:
	case ResourceState::Common:
		return VkAccessFlagBits(0);
	case ResourceState::VertexBuffer:
		return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	case ResourceState::IndexBuffer:
		return VK_ACCESS_INDEX_READ_BIT;
	case ResourceState::ConstantBuffer:
		return VK_ACCESS_UNIFORM_READ_BIT;
	case ResourceState::RenderTarget:
		return  VkAccessFlagBits(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
	case ResourceState::UnorderedAccess:
		return VK_ACCESS_SHADER_WRITE_BIT;
	case ResourceState::DepthRead:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	case ResourceState::DepthWrite:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case ResourceState::NonPixelShaderResource:
	case ResourceState::PixelShaderResource:
	case ResourceState::ShaderResource:
		return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	case ResourceState::StreamOut:
		return VkAccessFlagBits(0);
	case ResourceState::IndirectArgument:
		return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	case ResourceState::CopyDest:
	case ResourceState::ResolveDest:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case ResourceState::CopySource:
	case ResourceState::ResolveSource:
	case ResourceState::GenericRead:
		return VK_ACCESS_TRANSFER_READ_BIT;
	case ResourceState::Present:
		return VkAccessFlagBits(0);
	case ResourceState::Predication:
		return s_invalidFlags;
	case ResourceState::PreInitialized:
		return VkAccessFlagBits(0);

	default:
		assert(false);
		return s_invalidFlags;
	}
}


VkPipelineStageFlags GetShaderStageMask(ResourceState state, bool isSrc)
{
	static const auto s_invalidShaderStage = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM;

	switch (state)
	{
	case ResourceState::Undefined:
	case ResourceState::PreInitialized:
	case ResourceState::Common:
		assert(isSrc);
		return isSrc ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	case ResourceState::VertexBuffer:
	case ResourceState::IndexBuffer:
		return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	case ResourceState::ConstantBuffer:
	case ResourceState::NonPixelShaderResource:
	case ResourceState::ShaderResource:
	case ResourceState::UnorderedAccess:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	case ResourceState::PixelShaderResource:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case ResourceState::RenderTarget:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case ResourceState::DepthRead:
	case ResourceState::DepthWrite:
		return isSrc ? VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;;
	case ResourceState::StreamOut:
		return s_invalidShaderStage;
	case ResourceState::IndirectArgument:
		return VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	case ResourceState::CopyDest:
	case ResourceState::ResolveDest:
	case ResourceState::CopySource:
	case ResourceState::ResolveSource:
	case ResourceState::GenericRead:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case ResourceState::Present:
		return isSrc ? (VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;;
	case ResourceState::Predication:
		return s_invalidShaderStage;
	
	default:
		assert(false);
		return s_invalidShaderStage;
	}
}


VkImageAspectFlags GetAspectFlagsFromFormat(Format format, bool ignoreStencil)
{
	VkImageAspectFlags flags = 0;

	if (IsDepthFormat(format))
	{
		flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if (ignoreStencil == false)
	{
		if (IsStencilFormat(format))
		{
			flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}

	if (IsDepthStencilFormat(format) == false)
	{
		flags |= VK_IMAGE_ASPECT_COLOR_BIT;
	}

	return flags;
}

} // namespace Kodiak