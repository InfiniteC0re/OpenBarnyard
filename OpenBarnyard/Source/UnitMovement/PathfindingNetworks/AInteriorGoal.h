#pragma once

#include <Math/TVector4.h>

class AInteriorNetwork;
class AUnit;

class AInteriorGoal
{
public:
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_HAS_EXIT = BITFLAG( 0 ), // m_fExitX and m_fExitZ is a point to walk to once the goal is reached (AInteriorNetwork::BuildPathToGoalExit)
	};

	AInteriorGoal()  = default;
	~AInteriorGoal() = default;

	TBOOL UpdateHeight();
	TBOOL GetPosition( Toshi::TVector4& a_rOutPosition );

	// Resolves the goal's height once something collides with it (the argument is unused)
	void UpdateHeightIfUnknown( class AGoalCollisionUpdateInterface* a_pInterface );

	// Whether a_pUnit can use the goal, assigns the secondary name as an interaction to the unit if it has one (a_iUnused isn't used)
	TBOOL CanUnitUse( TINT a_iUnused, AUnit* a_pUnit );

public:
	Toshi::TVector4   m_vForward;
	TFLOAT            m_fPositionX;
	TFLOAT            m_fPositionZ;
	TFLOAT            m_fExitX; // see FLAGS_HAS_EXIT
	TFLOAT            m_fExitZ;
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

	FLAGS m_eFlags;
};
