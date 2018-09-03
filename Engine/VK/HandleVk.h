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


// Template specializations for destructors
template<> VkHandle<VkInstance>::~VkHandle();
template<> VkHandle<VkDevice>::~VkHandle();
template<> VkHandle<VkImageView>::~VkHandle();

} // namespace Kodiak