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

#include "Texture12.h"

#include "BinaryReader.h"
#include "Filesystem.h"
#include "KTXTextureLoader.h"

#include "CommandContext12.h"
#include "DDSTextureLoader12.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


namespace
{

map<string, shared_ptr<ManagedTexture>> s_textureCache;


pair<shared_ptr<ManagedTexture>, bool> FindOrLoadTexture(const string& filename)
{
	static mutex s_mutex;
	lock_guard<mutex> CS(s_mutex);

	auto iter = s_textureCache.find(filename);

	// If it's found, it has already been loaded or the load process has begun
	if (iter != s_textureCache.end())
	{
		return make_pair(iter->second, false);
	}

	auto newTexture = make_shared<ManagedTexture>(filename);
	s_textureCache[filename] = newTexture;

	// This was the first time it was requested, so indicate that the caller must read the file
	return make_pair(newTexture, true);
}


template <typename F>
shared_ptr<Texture> MakeTexture(const string& filename, F fn)
{
	auto managedTex = FindOrLoadTexture(filename);

	auto manTex = managedTex.first;
	const bool requestsLoad = managedTex.second;

	if (!requestsLoad)
	{
		manTex->WaitForLoad();
		return manTex;
	}

	fn(manTex.get(), filename);
	return manTex;
}

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


D3D12_UAV_DIMENSION GetUAVDimension(TextureType type, bool isArray)
{
	switch (type)
	{
	case TextureType::Texture1D:
		return (isArray) ? D3D12_UAV_DIMENSION_TEXTURE1DARRAY : D3D12_UAV_DIMENSION_TEXTURE1D;
	case TextureType::Texture2D:
		return (isArray) ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION_TEXTURE2D;
	case TextureType::TextureCube:
		return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
	default:
		assert(false);
		return D3D12_UAV_DIMENSION_UNKNOWN;
	}
}


// TODO MSAA support here
D3D12_SRV_DIMENSION GetSRVDimension(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
		return D3D12_SRV_DIMENSION_TEXTURE1D;
	case TextureType::Texture1D_Array:
		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case TextureType::Texture2D:
		return D3D12_SRV_DIMENSION_TEXTURE2D;
	case TextureType::Texture2D_Array:
		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case TextureType::Texture3D:
		return D3D12_SRV_DIMENSION_TEXTURE3D;
	case TextureType::TextureCube:
		return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case TextureType::TextureCube_Array:
		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
	default:
		assert(false);
		return D3D12_SRV_DIMENSION_UNKNOWN;
	}
}


D3D12_RESOURCE_DIMENSION GetResourceDimension(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
	case TextureType::Texture1D_Array:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

	case TextureType::Texture2D:
	case TextureType::Texture2D_Array:
	case TextureType::TextureCube:
	case TextureType::TextureCube_Array:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	case TextureType::Texture3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	default:
		assert(false);
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}

} // anonymous namespace


void TextureInitializer::Initialize()
{
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

			m_data[index].pData = nullptr;
			m_data[index].RowPitch = static_cast<UINT>(rowBytes);
			m_data[index].SlicePitch = static_cast<UINT>(numBytes);
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
	m_data[index].pData = data;
}


Texture::Texture()
{
	m_cpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


Texture::Texture(D3D12_CPU_DESCRIPTOR_HANDLE handle)
	: m_cpuDescriptorHandle(handle)
{}


void Texture::Create1D(uint32_t width, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture1D, format, width, 1, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture2D, format, width, height, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture3D, format, width, height, depth, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create(TextureInitializer& init)
{
	m_width = init.m_width;
	m_height = init.m_height;
	m_depthOrArraySize = init.m_depthOrArraySize;
	m_format = init.m_format;
	m_numMips = init.m_numMips;

	m_usageState = ResourceState::CopyDest;

	D3D12_RESOURCE_DESC texDesc = {};
	texDesc.Dimension = GetResourceDimension(init.m_type);
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.DepthOrArraySize = static_cast<UINT16>(m_depthOrArraySize);
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
		static_cast<D3D12_RESOURCE_STATES>(m_usageState), nullptr, IID_PPV_ARGS(&m_resource)));

	m_resource->SetName(L"Texture");

	uint32_t arraySize = (init.m_type == TextureType::Texture3D) ? 1 : m_depthOrArraySize;

	CommandContext::InitializeTexture(*this, arraySize * m_numMips, init.m_data.data());

	if (m_cpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_cpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
	SRVDesc.Format = texDesc.Format;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRVDesc.ViewDimension = GetSRVDimension(init.m_type);
	switch (init.m_type)
	{
	case TextureType::Texture1D:
		SRVDesc.Texture1D.MipLevels = (UINT)-1;
		break;
	case TextureType::Texture1D_Array:
		SRVDesc.Texture1DArray.MipLevels = (UINT)-1;
		SRVDesc.Texture1DArray.ArraySize = m_depthOrArraySize;
		break;
	case TextureType::Texture2D:
		SRVDesc.Texture2D.MipLevels = (UINT)-1;
		break;
	case TextureType::Texture2D_Array:
		SRVDesc.Texture2DArray.MipLevels = (UINT)-1;
		SRVDesc.Texture2DArray.ArraySize = m_depthOrArraySize;
		break;
	case TextureType::TextureCube:
		SRVDesc.TextureCube.MipLevels = (UINT)-1;
		break;
	case TextureType::TextureCube_Array:
		SRVDesc.TextureCubeArray.MipLevels = (UINT)-1;
		SRVDesc.TextureCubeArray.NumCubes = m_depthOrArraySize;
		break;
	case TextureType::Texture3D:
		SRVDesc.Texture3D.MipLevels = (UINT)-1;
		break;
	}
	device->CreateShaderResourceView(m_resource, &SRVDesc, m_cpuDescriptorHandle);
}


shared_ptr<Texture> Texture::Load(const string& filename, bool sRgb)
{
	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(filename);

	assert_msg(!fullpath.empty(), "Could not find texture file %s", filename.c_str());

	string extension = filesystem.GetFileExtension(filename);

	if (extension == ".dds")
	{
		return MakeTexture(fullpath, [sRgb](Texture* texture, const string& filename) { texture->LoadDDS(filename, sRgb); });
	}
	else if (extension == ".ktx")
	{
		return MakeTexture(fullpath, [sRgb](Texture* texture, const string& filename) { texture->LoadKTX(filename, sRgb); });
	}
	else
	{
		assert_msg(false, "Unknown file extension %s", extension.c_str());
		return nullptr;
	}
}


shared_ptr<Texture> Texture::GetBlackTex2D()
{
	auto makeBlackTex = [](Texture* texture, const string& filename)
	{
		uint32_t blackPixel = 0u;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &blackPixel);
	};

	return MakeTexture("DefaultWhiteTexture", makeBlackTex);
}


shared_ptr<Texture> Texture::GetWhiteTex2D()
{
	auto makeWhiteTex = [](Texture* texture, const string& filename)
	{
		uint32_t whitePixel = 0xFFFFFFFFul;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &whitePixel);
	};

	return MakeTexture("DefaultWhiteTexture", makeWhiteTex);
}


shared_ptr<Texture> Texture::GetMagentaTex2D()
{
	auto makeMagentaTex = [](Texture* texture, const string& filename)
	{
		uint32_t magentaPixel = 0x00FF00FF;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &magentaPixel);
	};

	return MakeTexture("DefaultMagentaTexture", makeMagentaTex);
}


void Texture::DestroyAll()
{
	s_textureCache.clear();
}


void Texture::LoadDDS(const string& fullpath, bool sRgb)
{
	unique_ptr<byte[]> data;
	size_t dataSize;

	if (m_cpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_cpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, data, &dataSize));

	auto device = GetDevice();

	ThrowIfFailed(CreateDDSTextureFromMemory(device, data.get(), dataSize, 0, sRgb, &m_resource, m_cpuDescriptorHandle));
}


void Texture::LoadKTX(const string& fullpath, bool sRgb)
{
	unique_ptr<byte[]> data;
	size_t dataSize;

	if (m_cpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_cpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, data, &dataSize));

	auto device = GetDevice();

	ThrowIfFailed(CreateKTXTextureFromMemory(data.get(), dataSize, 0, sRgb, this));
}


void ManagedTexture::WaitForLoad() const
{
	volatile D3D12_CPU_DESCRIPTOR_HANDLE& volHandle = (volatile D3D12_CPU_DESCRIPTOR_HANDLE&)m_cpuDescriptorHandle;
	volatile bool& volValid = (volatile bool&)m_isValid;
	while (volHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN && volValid)
	{
		this_thread::yield();
	}
}


void ManagedTexture::SetToInvalidTexture()
{
	m_cpuDescriptorHandle = Texture::GetMagentaTex2D()->GetSRV();
	m_isValid = false;
}


uint32_t Kodiak::BytesPerPixel(Format format)
{
	return BitsPerPixel(format) / 8;
}