//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once


#include "VectorMath.h"
#include "Math\BoundingBox.h"


namespace Math
{

class BoundingSphere
{
public:
	BoundingSphere() {}
	BoundingSphere(Vector3 center, Scalar radius);
	explicit BoundingSphere(Vector4 sphere);
	explicit BoundingSphere(const BoundingBox& box);

	Vector3 GetCenter() const;
	Scalar GetRadius() const;

private:
	Vector4 m_repr;
};


//=======================================================================================================
// Inline implementations
//

inline BoundingSphere::BoundingSphere(Vector3 center, Scalar radius)
{
	m_repr = Vector4(center);
	m_repr.SetW(radius);
}


inline BoundingSphere::BoundingSphere(Vector4 sphere)
	: m_repr(sphere)
{}


inline BoundingSphere::BoundingSphere(const BoundingBox& box)
{
	m_repr = Vector4(box.GetCenter());
	m_repr.SetW(Length(box.GetExtents()));
}


inline Vector3 BoundingSphere::GetCenter(void) const
{
	return Vector3(m_repr);
}


inline Scalar BoundingSphere::GetRadius(void) const
{
	return m_repr.GetW();
}


} // namespace Math