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

template <typename VkType>
std::shared_ptr<VkHandle<VkType>> CreateHandle(VkType wrapped)
{
	return VkHandle<VkType>::Create(wrapped);
}

// Template specializations for destructors
template<> VkHandle<VkInstance>::~VkHandle();
template<> VkHandle<VkDevice>::~VkHandle();
template<> VkHandle<VkDeviceMemory>::~VkHandle();

} // namespace Kodiak