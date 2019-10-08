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

class VkBaseHandle
{
public:
	virtual ~VkBaseHandle() = default;
};


template <typename VkType>
class VkHandle : public VkBaseHandle
{
public:
	class VkPointer : public std::shared_ptr<VkHandle<VkType>>
	{
	public:
		using Super = std::shared_ptr<VkHandle<VkType>>;

		VkPointer() = default;
		VkPointer(VkHandle<VkType>* handle) : Super(handle) {}

		static VkPointer Create(VkType wrapped) { return VkPointer(new VkHandle(wrapped)); }

		operator VkType() const { return Get()->m_wrapped; }

	private:
		VkHandle<VkType>* Get() const { return Super::get(); }
	};

	~VkHandle()
	{
		static_assert(false, "VkHandle missing template specialization for destructor");
	}

private:
	friend class VkPointer;
	VkHandle(VkType wrapped) : m_wrapped(wrapped) {}
	VkType m_wrapped;
};


class VkResourceHandle : public VkBaseHandle
{
public:
	class VkPointer : public std::shared_ptr<VkResourceHandle>
	{
	public:
		using Super = std::shared_ptr<VkResourceHandle>;

		VkPointer() = default;
		VkPointer(VkResourceHandle* handle) : Super(handle) {}

		static VkPointer Create(VkImage image, VkDeviceMemory mem, bool ownsImage = true)
		{
			return VkPointer(new VkResourceHandle(image, mem, ownsImage));
		}

		static VkPointer Create(VkBuffer buffer, VkDeviceMemory mem, bool ownsImage = true)
		{
			return VkPointer(new VkResourceHandle(buffer, mem, ownsImage));
		}

		operator VkImage() const { return Get()->m_wrapped.image; }
		operator VkBuffer() const { return Get()->m_wrapped.buffer; }
		operator VkDeviceMemory() const { return Get()->m_wrappedMemory; }

	private:
		VkResourceHandle* Get() const { return Super::get(); }
	};

	~VkResourceHandle();

private:
	friend class VkPointer;
	VkResourceHandle(VkImage image, VkDeviceMemory memory, bool ownsImage) 
		: m_wrapped(image)
		, m_wrappedMemory(memory)
		, m_isImage(true)
		, m_ownsImage(ownsImage) 
	{}
	VkResourceHandle(VkBuffer buffer, VkDeviceMemory memory, bool ownsImage)
		: m_wrapped(buffer)
		, m_wrappedMemory(memory)
		, m_isImage(false)
		, m_ownsImage(ownsImage)
	{}
	
	union WrappedData
	{
		WrappedData(VkImage image) : image(image) {}
		WrappedData(VkBuffer buffer) : buffer(buffer) {}

		VkImage image;
		VkBuffer buffer;
	} m_wrapped;
	VkDeviceMemory m_wrappedMemory{ VK_NULL_HANDLE };

	const bool m_isImage{ true };
	const bool m_ownsImage{ true };
};


class VkDescriptorHandle : public VkBaseHandle
{
public:
	class VkPointer : public std::shared_ptr<VkDescriptorHandle>
	{
	public:
		using Super = std::shared_ptr<VkDescriptorHandle>;

		VkPointer() = default;
		VkPointer(VkDescriptorHandle* handle) : Super(handle) {}

		static VkPointer Create(VkImageView imageView, VkImageLayout layout)
		{
			return VkPointer(new VkDescriptorHandle(imageView, layout));
		}

		static VkPointer Create(VkBufferView bufferView)
		{
			return VkPointer(new VkDescriptorHandle(bufferView));
		}

		static VkPointer Create(VkBuffer buffer, uint32_t offset, uint32_t size)
		{
			return VkPointer(new VkDescriptorHandle(buffer, offset, size));
		}

		operator bool() const { return Get(); }

		operator VkImageView() const { return Get()->m_wrapped.imageView; }
		operator VkBufferView() const { return Get()->m_wrapped.bufferView; }
		operator VkBuffer() const { return Get()->m_wrapped.buffer; }

		operator VkDescriptorImageInfo() const { return Get()->m_wrappedDescriptor.imageInfo; }
		operator VkDescriptorBufferInfo() const { return Get()->m_wrappedDescriptor.bufferInfo; }

		operator const VkDescriptorImageInfo*() const { return &Get()->m_wrappedDescriptor.imageInfo; }
		operator const VkDescriptorBufferInfo*() const { return &Get()->m_wrappedDescriptor.bufferInfo; }

	private:
		VkDescriptorHandle* Get() const { return Super::get(); }
	};

	~VkDescriptorHandle();

private:
	friend class VkPointer;

	VkDescriptorHandle(VkImageView imageView, VkImageLayout layout)
		: m_wrapped(imageView)
		, m_wrappedDescriptor(imageView, layout)
		, m_isImageView(true)
		, m_isBufferView(false)
	{}

	VkDescriptorHandle(VkBufferView bufferView)
		: m_wrapped(bufferView)
		, m_wrappedDescriptor()
		, m_isImageView(false)
		, m_isBufferView(true)
	{}

	VkDescriptorHandle(VkBuffer buffer, uint32_t offset, uint32_t size)
		: m_wrapped(buffer)
		, m_wrappedDescriptor(buffer, offset, size)
		, m_isImageView(false)
		, m_isBufferView(false)
	{}

	union WrappedData
	{
		WrappedData(VkImageView imageView) : imageView(imageView) {}
		WrappedData(VkBufferView bufferView) : bufferView(bufferView) {}
		WrappedData(VkBuffer buffer) : buffer(buffer) {}

		VkImageView imageView;
		VkBufferView bufferView;
		VkBuffer buffer;
	} m_wrapped;

	union WrappedDescriptor
	{
		WrappedDescriptor() = default;
		WrappedDescriptor(VkImageView imageView, VkImageLayout layout)
		{
			imageInfo.sampler = VK_NULL_HANDLE;
			imageInfo.imageView = imageView;
			imageInfo.imageLayout = layout;
		}
		WrappedDescriptor(VkBuffer buffer, uint32_t offset, uint32_t size)
		{
			bufferInfo.buffer = buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = VK_WHOLE_SIZE;
		}

		VkDescriptorImageInfo imageInfo;
		VkDescriptorBufferInfo bufferInfo;
	} m_wrappedDescriptor;

	const bool m_isImageView{ true };
	const bool m_isBufferView{ false };
};


class VkFramebufferHandle : public VkBaseHandle
{
public:
	class VkPointer : std::shared_ptr<VkFramebufferHandle>
	{
	public:
		using Super = std::shared_ptr<VkFramebufferHandle>;

		VkPointer() = default;
		VkPointer(VkFramebufferHandle* handle) : Super(handle) {}

		static VkPointer Create(VkFramebuffer framebuffer, VkRenderPass renderPass)
		{
			return VkPointer(new VkFramebufferHandle(framebuffer, renderPass));
		}

		operator VkFramebuffer() const { return Get()->m_framebuffer; }
		operator VkRenderPass() const { return Get()->m_renderPass; }

	private:
		VkFramebufferHandle* Get() const { return Super::get(); }
	};

	~VkFramebufferHandle();

private:
	friend class VkPointer;

	VkFramebufferHandle(VkFramebuffer framebuffer, VkRenderPass renderPass)
		: m_framebuffer(framebuffer)
		, m_renderPass(renderPass)
	{}

	VkFramebuffer	m_framebuffer{ VK_NULL_HANDLE };
	VkRenderPass	m_renderPass{ VK_NULL_HANDLE };
};

// Template specializations for destructors
template<> VkHandle<VkInstance>::~VkHandle();
template<> VkHandle<VkDevice>::~VkHandle();
template<> VkHandle<VkImageView>::~VkHandle();
template<> VkHandle<VkQueryPool>::~VkHandle();

} // namespace Kodiak