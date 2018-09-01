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
	VkHandle(VkType wrapped) : m_wrapped(wrapped) {}
	~VkHandle()
	{
		static_assert(false, "VkHandle missing template specialization for destructor");
	}

	static std::shared_ptr<VkHandle> Create(VkType wrapped)
	{
		return std::make_shared<VkHandle>(wrapped);
	}

	operator VkType() const { return m_wrapped; }

private:
	VkType m_wrapped;
};

template <typename VkType>
std::shared_ptr<VkHandle<VkType>> CreateHandle(VkType wrapped)
{
	return VkHandle<VkType>::Create(wrapped);
}

template<> VkHandle<VkDeviceMemory>::~VkHandle();

} // namespace Kodiak