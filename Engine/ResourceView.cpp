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

#include "ResourceView.h"


using namespace Kodiak;


ResourceViewDesc Kodiak::DescribeIndexBuffer(size_t elementSize, size_t bufferSize)
{
	ResourceViewDesc resDesc = {};
	resDesc.elementSize = (uint32_t)elementSize;
	resDesc.bufferSize = (uint32_t)bufferSize;

	return resDesc;
}


ResourceViewDesc Kodiak::DescribeVertexBuffer(size_t elementSize, size_t elementCount, size_t bufferSize)
{
	ResourceViewDesc resDesc = {};
	resDesc.elementSize = (uint32_t)elementSize;
	resDesc.elementCount = (uint32_t)elementCount;
	resDesc.bufferSize = (uint32_t)bufferSize;

	return resDesc;
}


ResourceViewDesc Kodiak::DescribeConstantBuffer(size_t bufferSize)
{
	ResourceViewDesc resDesc = {};
	resDesc.bufferSize = (uint32_t)bufferSize;

	return resDesc;
}