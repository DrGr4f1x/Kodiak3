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

#include "Texture.h"

#include "BinaryReader.h"
#include "Filesystem.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\Resources\KTXTextureLoader.h"

#include "CommandContext12.h"
#include "DDSTextureLoader12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace std;


namespace
{

void GetSurfaceInfo(size_t width, size_t height, DXGI_FORMAT format, size_t& numBytes, size_t& rowBytes, size_t& numRows)
{
	numBytes = 0;
	rowBytes = 0;
	numRows = 0;

	bool bc = false;
	bool packed = false;
	bool planar = false;
	size_t bpe = 0;
	switch (format)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bc = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bc = true;
		bpe = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		packed = true;
		bpe = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		packed = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		planar = true;
		bpe = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		planar = true;
		bpe = 4;
		break;

	}

	if (bc)
	{
		size_t numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
		}
		size_t numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
		}
		rowBytes = numBlocksWide * bpe;
		numRows = numBlocksHigh;
		numBytes = rowBytes * numBlocksHigh;
	}
	else if (packed)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numRows = height;
		numBytes = rowBytes * height;
	}
	else if (format == DXGI_FORMAT_NV11)
	{
		rowBytes = ((width + 3) >> 2) * 4;
		numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
		numBytes = rowBytes * numRows;
	}
	else if (planar)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
		numRows = height + ((height + 1) >> 1);
	}
	else
	{
		size_t bpp = BitsPerPixel(MapDXGIFormatToEngine(format));
		rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
		numRows = height;
		numBytes = rowBytes * height;
	}
}

} // anonymous namespace


namespace Kodiak
{

struct TextureInitializer::PlatformData
{
	PlatformData(uint32_t size)
		: data(size)
	{}

	vector<D3D12_SUBRESOURCE_DATA> data;
};

} // namespace Kodiak


TextureInitializer::~TextureInitializer()
{
	delete m_platformData;
}


void TextureInitializer::PlatformCreate()
{
	m_platformData = new PlatformData(m_numMips * m_depthOrArraySize);

	DXGI_FORMAT format = static_cast<DXGI_FORMAT>(m_format);

	size_t index = 0;
	for (uint32_t i = 0; i < m_depthOrArraySize; ++i)
	{
		uint32_t width = m_width;
		uint32_t height = m_height;
		uint32_t depth = m_depthOrArraySize;

		for (uint32_t j = 0; j < m_numMips; ++j)
		{
			size_t numBytes;
			size_t rowBytes;
			size_t numRows;
			GetSurfaceInfo(width, height, format, numBytes, rowBytes, numRows);

			if (i == 0)
			{
				m_faceSize[j] = numBytes;
			}

			m_platformData->data[index].pData = nullptr;
			m_platformData->data[index].RowPitch = static_cast<UINT>(rowBytes);
			m_platformData->data[index].SlicePitch = static_cast<UINT>(numBytes);
			++index;

			width = width >> 1;
			height = height >> 1;
			depth = depth >> 1;

			width = (width == 0) ? 1 : width;
			height = (height == 0) ? 1 : height;
			depth = (depth == 0) ? 1 : depth;
		}
	}
}


void TextureInitializer::SetData(uint32_t slice, uint32_t mipLevel, const void* data)
{
	int index = slice * m_numMips + mipLevel;
	m_platformData->data[index].pData = data;
}


void Texture::Create1D(uint32_t width, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture1D, format, width, 1, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture2D, format, width, height, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture3D, format, width, height, depth, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create(TextureInitializer& init)
{
	m_width = init.m_width;
	m_height = init.m_height;
	m_arraySize = init.m_depthOrArraySize;
	m_format = init.m_format;
	m_numMips = init.m_numMips;
	m_type = init.m_type;

	m_usageState = ResourceState::CopyDest;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = GetResourceDimension(init.m_type);
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.DepthOrArraySize = static_cast<UINT16>(m_arraySize);
	texDesc.MipLevels = m_numMips;
	texDesc.Format = static_cast<DXGI_FORMAT>(m_format);
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	auto device = GetDevice();

	assert_succeeded(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
		GetResourceState(m_usageState), nullptr, IID_PPV_ARGS(&m_resource)));

	m_resource->SetName(L"Texture");

	uint32_t arraySize = (init.m_type == ResourceType::Texture3D) ? 1 : m_arraySize;

	CommandContext::InitializeTexture(*this, arraySize * m_numMips, init.m_platformData->data.data());

	CreateDerivedViews();
}


void Texture::LoadDDS(const string& fullpath, Format format, bool sRgb)
{
	// TODO this is janky
	auto& descriptorHandle = m_srv.GetHandle();

	if (descriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		descriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, m_data, &m_dataSize));

	ThrowIfFailed(CreateDDSTextureFromMemory(GetDevice(), m_data.get(), m_dataSize, 0, format, sRgb, &m_resource, descriptorHandle));

	ClearRetainedData();
}


void Texture::LoadKTX(const string& fullpath, Format format, bool sRgb)
{
	// TODO this is janky
	auto& descriptorHandle = m_srv.GetHandle();

	if (descriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		descriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, m_data, &m_dataSize));

	ThrowIfFailed(CreateKTXTextureFromMemory(m_data.get(), m_dataSize, 0, format, sRgb, this));

	ClearRetainedData();
}


void ManagedTexture::WaitForLoad() const
{
	volatile D3D12_CPU_DESCRIPTOR_HANDLE& volHandle = (volatile D3D12_CPU_DESCRIPTOR_HANDLE&)m_srv.GetHandle();
	volatile bool& volValid = (volatile bool&)m_isValid;
	while (volHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN && volValid)
	{
		this_thread::yield();
	}
}