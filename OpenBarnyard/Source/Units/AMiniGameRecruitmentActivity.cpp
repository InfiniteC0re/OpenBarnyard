#include "pch.h"
#include "AMiniGameRecruitmentActivity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ba7a0, CreateObject 005ba8e0, CreateObjectInPlace 005ba950
TDEFINE_CLASS( AMiniGameRecruitmentActivity );

// NOTE: inlined into CreateObject 005ba8e0 / CreateObjectInPlace 005ba950
AMiniGameRecruitmentActivity::AMiniGameRecruitmentActivity()
{
}

// $Barnyard: FUNCTION 005ba7b0
// $Barnyard: FUNCTION 005ba9c0
AMiniGameRecruitmentActivity::~AMiniGameRecruitmentActivity()
{
}

// $Barnyard: FUNCTION 005ba820
TBOOL AMiniGameRecruitmentActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pMiniGameState = a_pProperties->GetProperty( "minigamestate" );

	if ( pMiniGameState )
		m_strMiniGameState = pMiniGameState->GetTPString8();

	return AInteractionActivity::Create( a_pProperties );
}
