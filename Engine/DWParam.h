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

struct DWParam
{
	DWParam(float f) : _float(f) {}
	DWParam(uint32_t u) : _uint(u) {}
	DWParam(int32_t i) : _int(i) {}

	void operator=(float f) { _float = f; }
	void operator=(uint32_t u) { _uint = u; }
	void operator=(int32_t i) { _int = i; }

	union
	{
		float		_float;
		uint32_t	_uint;
		int32_t		_int;
	};
};

} // namespace Kodiak