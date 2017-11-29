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
class Texture;


HRESULT __cdecl CreateKTXTextureFromMemory(
	const uint8_t* ktxData,
	size_t ktxDataSize,
	size_t maxsize,
	bool forceSRGB,
	Texture* texture
);


HRESULT __cdecl CreateKTXTextureFromFile(
	const char* szFileName,
	size_t maxsize,
	bool forceSRGB,
	Texture* texture
);

} // namespace Kodiak