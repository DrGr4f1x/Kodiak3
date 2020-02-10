// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Filesystem.h"


using namespace Kodiak;
using namespace std;


namespace
{
constexpr uint32_t s_maxPathLength = 4096;
shared_mutex s_mutex;
}


Filesystem& Filesystem::GetInstance()
{
	static Filesystem instance;
	return instance;
}


Filesystem::Filesystem()
{
	Initialize();
}


void Filesystem::SetDefaultRootPath()
{
	SetRootPath(m_binaryPath.parent_path());
}


void Filesystem::SetRootPath(const string& rootPathStr)
{
	unique_lock<shared_mutex> CS(s_mutex);

	if (m_rootPath == rootPathStr)
	{
		return;
	}

	RemoveAllSearchPaths();

	m_rootPath = rootPathStr;
	m_logPath = m_rootPath / "Logs";
}


void Filesystem::SetRootPath(const filesystem::path& rootPath)
{
	unique_lock<shared_mutex> CS(s_mutex);

	if (m_rootPath == rootPath)
	{
		return;
	}

	RemoveAllSearchPaths();

	m_rootPath = rootPath;
	m_logPath = m_rootPath / "Logs";
}


void Filesystem::AddSearchPath(const string& searchPathStr, bool appendPath)
{
	PathDesc* pathPtr = nullptr;
	PathDesc* cur = m_searchPaths;
	PathDesc* prev = nullptr;

	const filesystem::path searchPath{ searchPathStr };

	unique_lock<shared_mutex> CS(s_mutex);

	while (cur)
	{
		if (cur->localPath == searchPath)
		{
			pathPtr = cur;
			break;
		}
		prev = cur;
		cur = cur->next;
	}

	if (pathPtr)
	{
		return;
	}

	filesystem::path fullSearchPath = m_rootPath / searchPath;

	if (!filesystem::exists(fullSearchPath))
	{
		assert_msg(false, "Path %s is not a valid path beneath root.", fullSearchPath.c_str());
		return;
	}

	PathDesc* newPathDesc = new PathDesc;
	newPathDesc->localPath = searchPath;
	newPathDesc->fullPath = fullSearchPath;

	if (appendPath)
	{
		if (prev == nullptr)
		{
			m_searchPaths = newPathDesc;
		}
		else
		{
			prev->next = newPathDesc;
		}
	}
	else
	{
		newPathDesc->next = m_searchPaths;
		m_searchPaths = newPathDesc;
	}
}


void Filesystem::RemoveSearchPath(const string& searchPathStr)
{
	const filesystem::path searchPath{ searchPathStr };

	unique_lock<shared_mutex> CS(s_mutex);

	if (searchPath == m_rootPath)
	{
		return;
	}

	PathDesc* cur = m_searchPaths;
	PathDesc* prev = nullptr;
	while (cur)
	{
		if (cur->localPath == searchPath)
		{
			// Head
			if (prev == nullptr)
			{
				m_searchPaths = cur->next;
			}
			else
			{
				prev->next = cur->next;
			}
			delete cur;
			break;
		}
		prev = cur;
		cur = cur->next;
	}
}


vector<filesystem::path> Filesystem::GetSearchPaths() const
{
	shared_lock<shared_mutex> CS(s_mutex);

	vector<filesystem::path> paths;

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		paths.push_back(cur->fullPath);
		cur = cur->next;
	}

	return paths;
}


bool Filesystem::Exists(const string& fname) const
{
	const filesystem::path filePath{ fname };

	shared_lock<shared_mutex> CS(s_mutex);

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		auto fullPath = cur->fullPath / filePath;
		if (filesystem::exists(fullPath))
		{
			return true;
		}
		cur = cur->next;
	}
	return false;
}


bool Filesystem::IsRegularFile(const string& fname) const
{
	const filesystem::path filePath{ fname };

	shared_lock<shared_mutex> CS(s_mutex);

	if (filesystem::exists(filePath) && filesystem::is_regular_file(filePath))
	{
		return true;
	}

	return false;
}


bool Filesystem::IsDirectory(const string& dname) const
{
	const filesystem::path dirPath{ dname };

	shared_lock<shared_mutex> CS(s_mutex);

	if (filesystem::exists(dirPath) && filesystem::is_directory(dirPath))
	{
		return true;
	}

	return false;
}

string Filesystem::GetFullPath(const string& fname)
{
	const filesystem::path filePath{ fname };

	shared_lock<shared_mutex> CS(s_mutex);

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		auto fullPath = cur->fullPath / filePath;
		if (filesystem::exists(fullPath))
		{
			return fullPath.string();
		}
		cur = cur->next;
	}
	return "";
}


string Filesystem::GetFileExtension(const string& fname)
{
	string extension = filesystem::path(fname).extension().string();

	transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	return extension;
}


bool Filesystem::EnsureDirectory(const string& pathStr)
{
	shared_lock<shared_mutex> CS(s_mutex);

	if (IsDirectory(pathStr))
		return true;

	const filesystem::path dirPath{ pathStr };

	return filesystem::create_directory(pathStr);
}


bool Filesystem::EnsureLogDirectory()
{
	return EnsureDirectory(m_logPath.string());
}


void Filesystem::Initialize()
{
	unique_lock<shared_mutex> CS(s_mutex);

	// Get Path
	string pathStr;
	pathStr.resize(s_maxPathLength, 0);
	GetModuleFileNameA(nullptr, &pathStr[0], s_maxPathLength);

	filesystem::path binFilePath{ pathStr };
	filesystem::path binPath = binFilePath.remove_filename().parent_path();

	filesystem::current_path(binPath);

	m_binaryPath = binPath;
	m_rootPath = m_binaryPath;

	assert(m_searchPaths == nullptr);
}


void Filesystem::RemoveAllSearchPaths()
{
	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		PathDesc* temp = cur->next;
		delete cur;
		cur = temp;
	}
	m_searchPaths = nullptr;
}