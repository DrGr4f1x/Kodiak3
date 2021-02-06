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

#include "GraphicsFeatures.h"


using namespace Kodiak;
using namespace std;


GraphicsFeatureSet Kodiak::g_requiredFeatures;
GraphicsFeatureSet Kodiak::g_optionalFeatures;
const GraphicsFeatureSet Kodiak::g_enabledFeatures;


GraphicsFeatureProxy::GraphicsFeatureProxy(GraphicsFeatureSet* featureSet, GraphicsFeature feature)
	: m_feature(feature)
{
	featureSet->RegisterFeature(this);
}


const string GraphicsFeatureProxy::GetName() const
{
	return GraphicsFeatureToString(m_feature);
}


void GraphicsFeatureSet::RegisterFeature(GraphicsFeatureProxy* featureProxy)
{
	m_features.push_back(featureProxy);
}


namespace Kodiak
{

const string GraphicsFeatureToString(GraphicsFeature feature)
{
	switch (feature)
	{
	case GraphicsFeature::RobustBufferAccess: return "Robust Buffer Access"; break;
	case GraphicsFeature::FullDrawIndexUint32: return "Full Draw Index Uint32"; break;
	case GraphicsFeature::TextureCubeArray: return "Texture Cube Array"; break;
	case GraphicsFeature::IndependentBlend: return "Independent Blend"; break;
	case GraphicsFeature::GeometryShader: return "Geometry Shader"; break;
	case GraphicsFeature::TessellationShader: return "Tessellation Shader"; break;
	case GraphicsFeature::SampleRateShading: return "Sample Rate Shading"; break;
	case GraphicsFeature::DualSrcBlend: return "Dual Source Blend"; break;
	case GraphicsFeature::LogicOp: return "Logic Op"; break;
	case GraphicsFeature::MultiDrawIndirect: return "Multi-Draw Indirect"; break;
	case GraphicsFeature::DrawIndirectFirstInstance: return "Draw Indirect First Instance"; break;
	case GraphicsFeature::DepthClamp: return "Depth Clamp"; break;
	case GraphicsFeature::DepthBiasClamp: return "Depth Bias Clamp"; break;
	case GraphicsFeature::FillModeNonSolid: return "Fill Mode Non-Solid"; break;
	case GraphicsFeature::DepthBounds: return "Depth Bounds"; break;
	case GraphicsFeature::WideLines: return "Wide Lines"; break;
	case GraphicsFeature::LargePoints: return "Large Points"; break;
	case GraphicsFeature::AlphaToOne: return "Alpha To One"; break;
	case GraphicsFeature::MultiViewport: return "Multi-Viewport"; break;
	case GraphicsFeature::SamplerAnisotropy: return "Sampler Anisotropy"; break;
	case GraphicsFeature::TextureCompressionETC2: return "Texture Compression ETC2"; break;
	case GraphicsFeature::TextureCompressionASTC_LDR: return "Texture Compression ASTC LDR"; break;
	case GraphicsFeature::TextureCompressionBC: return "Texture Compression BC"; break;
	case GraphicsFeature::OcclusionQueryPrecise: return "Occlusion Query Precise"; break;
	case GraphicsFeature::PipelineStatisticsQuery: return "Pipeline Statistics Query"; break;
	case GraphicsFeature::VertexPipelineStoresAndAtomics: return "Vertex Pipeline Stores And Atomics"; break;
	case GraphicsFeature::PixelShaderStoresAndAtomics: return "Pixel Shader Stores And Atomics"; break;
	case GraphicsFeature::ShaderTessellationAndGeometryPointSize: return "Shader Tessellation And Geometry Point Size"; break;
	case GraphicsFeature::ShaderTextureGatherExtended: return "Shader Texture Gather Extended"; break;
	case GraphicsFeature::ShaderUAVExtendedFormats: return "Shader UAV Extended Formats"; break;
	case GraphicsFeature::ShaderUAVMultisample: return "Shader UAV Multisample"; break;

	case GraphicsFeature::ShaderClipDistance: return "Shader Clip Distance"; break;
	case GraphicsFeature::ShaderCullDistance: return "Shader Cull Distance"; break;
	case GraphicsFeature::ShaderFloat64: return "Shader Float 64"; break;
	case GraphicsFeature::ShaderFloat16: return "Shader Float 16"; break;
	case GraphicsFeature::ShaderInt64: return "Shader Int 64"; break;
	case GraphicsFeature::ShaderInt16: return "Shader Int 16"; break;
	case GraphicsFeature::ShaderInt8: return "Shader Int 8"; break;

	case GraphicsFeature::VariableMultisampleRate: return "Variable Multisample Rate"; break;

	default: return "Unknown"; break;
	}
}

} // namespace Kodiak