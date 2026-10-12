#include "pch.h"
#include "AGenerateEventActivity.h"
#include "AInteractionComponent.h"
#include "AInteractionEventManager.h"
#include "AUnit.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b6a90, CreateObject 005b6ab0, CreateObjectInPlace 005b6b30
TDEFINE_CLASS( AGenerateEventActivity );

// NOTE: inlined into CreateObject, retail leaves m_fForceInteractionPeriod uninitialised
AGenerateEventActivity::AGenerateEventActivity()
    : m_pEventClass( TNULL )
    , m_fForceInteractionPeriod( 0.0f )
    , m_bCreateNew( TFALSE )
{
}

// $Barnyard: FUNCTION 005b6aa0
// $Barnyard: FUNCTION 005b6ba0
AGenerateEventActivity::~AGenerateEventActivity()
{
}

// $Barnyard: FUNCTION 005b6bc0
TBOOL AGenerateEventActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pEvent = a_pProperties->GetProperty( "event" );

	if ( pEvent )
	{
		TString8 strClassName;
		strClassName.Format( "ainteraction%sevent", pEvent->GetTPString8().GetString() );

		m_pEventClass = TClass::Find( strClassName, &TGetClass( AInteractionEvent ) );
	}

	a_pProperties->GetOptionalPropertyValue( m_fForceInteractionPeriod, TPS8D( "forceinteractionperiod" ) );

	const PBPropertyValue* pCreateNew = a_pProperties->GetProperty( "createnew" );

	if ( pCreateNew )
		m_bCreateNew = pCreateNew->GetRaw() != TNULL;

	// NOTE: retail doesn't call AInteractionActivity::Create, "category" is ignored
	return TTRUE;
}

// $Barnyard: FUNCTION 005b6d10
AInteractionActivity::RESULT AGenerateEventActivity::Update( AInteractionSequence* a_pSequence )
{
	AInteractionEventManager* pEventManager = AInteractionEventManager::GetSingleton();
	AInteractionEvent*        pEvent        = TNULL;

	if ( m_bCreateNew )
	{
		pEvent = pEventManager->NewActiveEvent( m_pEventClass );
	}
	else if ( !pEventManager->FindEvent( m_pEventClass ) )
	{
		// Only throw the event if there isn't one already
		pEvent = pEventManager->ActivateEvent( m_pEventClass );

		if ( !pEvent )
			pEvent = pEventManager->NewActiveEvent( m_pEventClass );
	}

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit )
		{
			if ( pEvent )
				pEvent->SetUnit( pUnit );

			if ( 0.0f < m_fForceInteractionPeriod )
				it->SetForcedInteraction( TPS8D( "trigger_force_rumour" ), TPS8D( "trigger_override_rumour" ), m_fForceInteractionPeriod );
		}
	}

	return RESULT_DONE;
}
