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


GraphicsFeatureProxy::GraphicsFeatureProxy(GraphicsFeatureSet* featureSet, const string& name, GraphicsFeature feature)
	: m_name(name)
	, m_feature(feature)
{
	featureSet->RegisterFeature(this);
}


void GraphicsFeatureSet::RegisterFeature(GraphicsFeatureProxy* featureProxy)
{
	m_features.push_back(featureProxy);
}


namespace Kodiak
{

GraphicsFeatureSet& RequiredFeatures() 
{ 
	static GraphicsFeatureSet features;
	return features; 
}

GraphicsFeatureSet& OptionalFeatures() 
{ 
	static GraphicsFeatureSet features;
	return features; 
}

const GraphicsFeatureSet& EnabledFeatures() 
{
	static GraphicsFeatureSet features;
	return features;
}

} // namespace Kodiak