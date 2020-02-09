// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

class Shader
{
public:
	static void DestroyAll();
	static Shader* Load(const std::string& shaderPath);

	Shader(const std::string& shaderPath);

	const uint8_t* GetByteCode() const { return m_byteCode.get(); }
	size_t GetByteCodeSize() const { return m_byteCodeSize; }

	const std::string& GetPath() const { return m_shaderPath; }

private:
	void WaitForLoad() const;

private:
	std::unique_ptr<uint8_t[]>		m_byteCode;
	size_t						m_byteCodeSize{ 0 };

	bool						m_isLoaded{ false };

	std::string					m_shaderPath;
};

} // namespace Kodiak