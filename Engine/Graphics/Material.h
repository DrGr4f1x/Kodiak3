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
class GraphicsPSO;
class ResourceSet;
class RootSignature;


// TODO - This is *very* placeholder.  Need a full system to deal with materials (shader introspection, etc)
struct Material
{
	RootSignature* rootSig{ nullptr };
	GraphicsPSO* pso{ nullptr };
	ResourceSet* resources{ nullptr };
};

using MaterialPtr = std::shared_ptr<Material>;


inline MaterialPtr MakeMaterial(RootSignature* rootSig, GraphicsPSO* pso, ResourceSet* resources)
{
	auto material = std::make_shared<Material>();
	material->rootSig = rootSig;
	material->pso = pso;
	material->resources = resources;
	return material;
}

} // namespace Kodiak
