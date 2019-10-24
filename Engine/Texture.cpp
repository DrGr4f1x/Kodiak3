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

#include "Filesystem.h"
#include "GraphicsDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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

} // anonymous namespace


Texture::Texture() = default;


Texture::~Texture()
{
	g_graphicsDevice->ReleaseResource(m_resource);
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
		return MakeTexture(fullpath, [sRgb](Texture* texture, const string& filename) { texture->LoadImage(filename, sRgb); });
	}
}


shared_ptr<Texture> Texture::GetBlackTex2D()
{
	auto makeBlackTex = [](Texture* texture, const string& filename)
	{
		uint32_t blackPixel = 0u;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &blackPixel);
	};

	return MakeTexture("DefaultBlackTexture", makeBlackTex);
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


void Texture::LoadImage(const string& fullpath, bool sRgb)
{
	int x, y, n;
	unsigned char* data = stbi_load(fullpath.c_str(), &x, &y, &n, 4);
	assert_msg(data != nullptr, "Failed to load image %s", fullpath.c_str());

	Format format = Format::Unknown;
	if (n == 1)
		format = Format::R8_UNorm;
	else if (n == 2)
		format = Format::R8G8_UNorm;
	else if (n == 3 || n == 4)
		format = Format::R8G8B8A8_UNorm;

	Create2D(x, y, format, data);

	stbi_image_free(data);
}


void Texture::CreateDerivedViews()
{
	TextureViewDesc desc = {};
	desc.format = m_format;
	desc.usage = ResourceState::ShaderResource;
	desc.arraySize = m_arraySize;
	desc.mipCount = m_numMips;
	desc.isDepth = false;
	desc.isStencil = false;

	m_srv.Create(m_resource, m_type, desc);
}


void ManagedTexture::SetToInvalidTexture()
{
	m_srv = Texture::GetMagentaTex2D()->GetSRV();
	m_isValid = false;
}


uint32_t Kodiak::BytesPerPixel(Format format)
{
	return BitsPerPixel(format) / 8;
}