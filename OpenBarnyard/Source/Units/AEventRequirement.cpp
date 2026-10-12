#include "pch.h"
#include "AEventRequirement.h"
#include "AInteractionManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a9780, CreateObject 005a97a0
// NOTE: GetClass 005a9780, CreateObject 005a97a0, CreateObjectInPlace 005a9800
TDEFINE_CLASS( AEventRequirement );

// $Barnyard: FUNCTION 005a9790
// $Barnyard: FUNCTION 005a9850
AEventRequirement::~AEventRequirement()
{
}

// $Barnyard: FUNCTION 005a98c0
TBOOL AEventRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strKey;
	a_pProperties->GetOptionalPropertyValue( strKey, TPS8D( "key" ) );
	TStringManager::String8Copy( m_szKey, strKey.GetString(), sizeof( m_szKey ) );

	a_pProperties->GetOptionalPropertyValue( m_iKeyValue, TPS8D( "keyval" ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a9870
void AEventRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	// NOTE: only the weight is set
	if ( AInteractionManager::GetSingleton()->GetPersistentLatch( m_szKey ) == m_iKeyValue )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}
