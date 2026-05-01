#pragma once

#include <Math/TVector4.h>

class AInteriorNetwork;

class AInteriorGoal
{
public:
	AInteriorGoal()  = default;
	~AInteriorGoal() = default;

	TBOOL UpdateHeight();
	TBOOL GetPosition( Toshi::TVector4& a_rOutPosition );

public:
	Toshi::TVector4   m_vForward;
	TFLOAT            m_fPositionX;
	TFLOAT            m_fPositionZ;
	TINT              m_iUnk1;
	TINT              m_iUnk2;
	AInteriorNetwork* m_pNetwork;
	TFLOAT            m_fPositionY;
	TINT              m_iGroupId;
	TINT              m_iExportId;
	TINT              m_iAreaIndex;

	// HACK: swap this with pooled name after reading the TRB data
	union
	{
		TCHAR*            m_szName_Internal;
		Toshi::TPString8* m_strName;
	};

	// HACK: swap this with pooled name after reading the TRB data
	union
	{
		TCHAR*            m_szSecondaryName_Internal;
		Toshi::TPString8* m_strSecondaryName;
	};

	TINT m_iUnk3;
};
