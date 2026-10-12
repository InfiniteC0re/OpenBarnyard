#pragma once
#include <Math/TVector4.h>

// Spawn placement handed to AUnit::Create (the steer reads position and, at +0x10, direction).
class AUnitTransform
{
public:
	AUnitTransform( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TINT a_iUnk1, TINT a_iModelIndex, TINT a_iUnk3, TBOOL a_bCreateObstruction );

public:
	Toshi::TVector4 m_vecPosition;        // 0x00
	Toshi::TVector4 m_vecDirection;       // 0x10
	TINT            m_iUnk20;             // 0x20, every caller passes 0
	TINT            m_iModelIndex;        // 0x24, model variant (AAnimatedUnit::CreateAnimatableObject, AStaticUnit::CreateModelInstance), -1 by default
	TINT            m_iUnk28;             // 0x28, every caller passes -1
	TBOOL           m_bCreateObstruction; // 0x2c, every caller passes TTRUE
};
