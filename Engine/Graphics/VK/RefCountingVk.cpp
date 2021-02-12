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

#include "RefCountingVk.h"


#define IMPL_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY Kodiak::name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


IMPL_GUID(IID_UVkInstance, 0x2cb7a094, 0xec8f, 0x4160, 0x84, 0xaa, 0xf1, 0x87, 0x46, 0xfe, 0xf0, 0xbd);
IMPL_GUID(IID_UVkPhysicalDevice, 0xdb19a237, 0x4cde, 0x4609, 0x9c, 0x89, 0x19, 0xe4, 0x6c, 0xba, 0x85, 0x4b);
IMPL_GUID(IID_UVkSurface, 0x9891d0bf, 0x51a4, 0x4a83, 0xa6, 0xd8, 0x44, 0xf6, 0xe3, 0x56, 0x82, 0x43);
IMPL_GUID(IID_UVkDevice, 0x1be6f63b, 0x6ac2, 0x4c35, 0xa0, 0xf3, 0xe2, 0x2d, 0x79, 0x57, 0xe1, 0x46);
IMPL_GUID(IID_UVmaAllocator, 0xa60bcd86, 0xce2b, 0x4c5f, 0xa7, 0x3d, 0xb8, 0x42, 0xa2, 0x71, 0xce, 0xf9);
IMPL_GUID(IID_UVkFence, 0xe1eaa295, 0x40e9, 0x4e41, 0x90, 0xdb, 0xe1, 0xb1, 0xb2, 0xe6, 0x84, 0x8e);
IMPL_GUID(IID_UVkSemaphore, 0x10f03d1d, 0xfc84, 0x4462, 0x87, 0x59, 0xfa, 0xfe, 0x67, 0x2e, 0x5, 0x2a);
IMPL_GUID(IID_UVkDebugUtilsMessenger, 0x100f605a, 0x2b, 0x48d0, 0x80, 0x9c, 0x2b, 0xfb, 0x86, 0x19, 0x2c, 0xad);
IMPL_GUID(IID_UVkImage, 0xd12387cd, 0x9e8, 0x42ef, 0xaa, 0x25, 0xe9, 0xfc, 0x5e, 0x29, 0x29, 0x2c);
IMPL_GUID(IID_UVkSwapchain, 0x2062e1ba, 0xe3b9, 0x4e43, 0xa9, 0xbf, 0x1f, 0x72, 0xae, 0x5b, 0xa, 0x6f);
IMPL_GUID(IID_UVkBuffer, 0xd879f552, 0x9f96, 0x4fa3, 0xbc, 0x4c, 0x6f, 0x56, 0x8e, 0x4d, 0x1e, 0xd6);
IMPL_GUID(IID_UVkImageView, 0x559e7409, 0xd5df, 0x4e19, 0x9e, 0xb7, 0x21, 0xa2, 0x7b, 0x24, 0x31, 0x4b);
IMPL_GUID(IID_UVkBufferView, 0x2cfdbef9, 0x69af, 0x43a1, 0x9b, 0x43, 0x75, 0xb9, 0x5c, 0x17, 0xf8, 0xb9);
IMPL_GUID(IID_UVkQueryPool, 0xcd51d2ff, 0xe501, 0x4cf4, 0xbd, 0x97, 0x70, 0x50, 0xab, 0x4e, 0x26, 0x36);
IMPL_GUID(IID_UVkCommandPool, 0x9eb11631, 0x4fdb, 0x42a1, 0xb3, 0xb8, 0xe3, 0xb3, 0xfb, 0xbb, 0x2d, 0x1);