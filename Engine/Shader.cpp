// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Shader.h"

#include "BinaryReader.h"
#include "FileSystem.h"


using namespace std;
using namespace Kodiak;


static map<size_t, unique_ptr<Shader>> s_shaderHashMap;
static mutex s_shaderMapMutex;


namespace
{

#if defined(DX12)
string AppendShaderFileExtension(const string& shaderPath)
{
	string res = shaderPath + ".cso";
	return res;
}
#elif defined(VK)
string AppendShaderFileExtension(const string& shaderPath)
{
	string res = shaderPath + ".spv";
	return res;
}
#else
#error No graphics API defined!
#endif


pair<Shader*, bool> FindOrLoadShader(const string& shaderPath)
{
	size_t hashCode = hash<string>{}(shaderPath);

	lock_guard<mutex> CS(s_shaderMapMutex);

	auto iter = s_shaderHashMap.find(hashCode);

	// If it's found, it has already been loaded or the load process has begun
	if (iter != s_shaderHashMap.end())
	{
		return make_pair(iter->second.get(), false);
	}

	Shader* shader = new Shader(shaderPath);
	s_shaderHashMap[hashCode].reset(shader);

	// This was the first time it was requested, so indicate that the caller must read the file
	return make_pair(shader, true);
}

} // anonymous namespace


Shader::Shader(const string& shaderPath)
	: m_shaderPath(shaderPath)
{}


void Shader::DestroyAll()
{
	lock_guard<mutex> CS(s_shaderMapMutex);
	s_shaderHashMap.clear();
}


Shader* Shader::Load(const string& shaderPath)
{
	string shaderPathWithExtension = AppendShaderFileExtension(shaderPath);

	auto managedShader = FindOrLoadShader(shaderPathWithExtension);

	Shader* shader = managedShader.first;
	const bool RequestsLoad = managedShader.second;

	// Wait on the load and return
	if (!RequestsLoad)
	{
		shader->WaitForLoad();
		return shader;
	}
	
	// Kick off the load
	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(shaderPathWithExtension);

	assert_succeeded(BinaryReader::ReadEntireFile(fullpath, shader->m_byteCode, &shader->m_byteCodeSize));
	shader->m_isLoaded = true;

	return shader;
}


void Shader::WaitForLoad() const
{
	volatile bool& volIsLoaded = (volatile bool&)m_isLoaded;
	while (!volIsLoaded)
	{
		this_thread::yield();
	}
}