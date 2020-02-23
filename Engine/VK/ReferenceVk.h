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

template <typename T>
class Reference
{
public:
	Reference(const std::shared_ptr<T>& ref)
		: m_ref(ref)
	{}

	const std::shared_ptr<T>& Get() const
	{
		return ref;
	}

private:
	std::shared_ptr<T>	m_ref;
};

} // namespace Kodiak