#include "pch.h"
#include "ADummyQuestElement.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005cf360, CreateObject 005cf480, CreateObjectInPlace 005cf4e0
TDEFINE_CLASS( ADummyQuestElement );

// $Barnyard: FUNCTION 005cf370
ADummyQuestElement::ADummyQuestElement()
    : m_strDescription( GetGlobalAllocator() )
{
}

// $Barnyard: FUNCTION 005cf3c0
// $Barnyard: FUNCTION 005cf520
ADummyQuestElement::~ADummyQuestElement()
{
}

// $Barnyard: FUNCTION 005cf420
TBOOL ADummyQuestElement::Create( const PBProperties* a_pProperties, TINT* a_pDataOffset )
{
	return AQuestElement::Create( a_pProperties, a_pDataOffset );
}

// $Barnyard: FUNCTION 005cf430
TBOOL ADummyQuestElement::InitData( AInteractionActivityData* a_pData )
{
	return AQuestElement::InitData( a_pData );
}

// $Barnyard: FUNCTION 005cf440
TBOOL ADummyQuestElement::GetDescription( const TWCHAR** a_ppOutText )
{
	m_strDescription.Format( L"Dummy quest element." );
	*a_ppOutText = m_strDescription.GetString( 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 005cf470
AQuestElement::PROGRESS ADummyQuestElement::CheckProgress( AInteractionActivityData* a_pData )
{
	return PROGRESS_RUNNING;
}
