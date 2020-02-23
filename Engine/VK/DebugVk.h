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
class Instance;


class DebugReportCallback : public Reference<Instance>, public NonCopyable
{
public:
	DebugReportCallback(const std::shared_ptr<Instance>& instance, VkDebugReportFlagsEXT flags);
	virtual ~DebugReportCallback();

protected:
	void Initialize(VkDebugReportFlagsEXT flags);

private:
	VkDebugReportCallbackEXT m_debugReportCallback{ VK_NULL_HANDLE };
};

} // namespace Kodiak