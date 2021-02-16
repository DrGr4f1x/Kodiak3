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

#include "SmokeSimUtils.h"


using namespace Kodiak;
using namespace std;


void ComputeFlattened3DTextureDims(uint32_t depth, uint32_t& outRows, uint32_t& outCols)
{
	uint32_t rows = (uint32_t)floorf(sqrtf((float)depth));
	uint32_t cols = rows;
	while (rows * cols < depth)
		cols++;
	assert(rows * cols >= depth);

	outRows = rows;
	outCols = cols;
}