#include "pch.h"
#include "AInteractionActivity.h"
#include "AInteraction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0059f8d0
TDEFINE_CLASS_NORUNTIME( AInteractionActivity );

// $Barnyard: FUNCTION 0059f8e0
AInteractionActivity::AInteractionActivity()
    : m_eCategory( AInteractionSequence::CATEGORY_ALL )
{
}

// $Barnyard: FUNCTION 0059f900
// $Barnyard: FUNCTION 0059f920
AInteractionActivity::~AInteractionActivity()
{
}

// $Barnyard: FUNCTION 0059fa10
// $Barnyard: FUNCTION 005b9fd0
TBOOL AInteractionActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pCategory = a_pProperties->GetProperty( "category" );

	if ( pCategory )
		m_eCategory = AInteractionSequence::GetCategoryFromName( pCategory->GetTPString8().GetString() );

	return TTRUE;
}

// $Barnyard: FUNCTION 0059f910
void AInteractionActivity::GetDataSize( TINT* a_pSize )
{
	*a_pSize = 0;
}

// $Barnyard: FUNCTION 0059f940
AUnit* AInteractionActivity::GetActiveUnit( AInteractionSequence* a_pSequence )
{
	auto it = a_pSequence->GetCategoryBegin( m_eCategory );

	if ( it == a_pSequence->GetCategoryEnd( m_eCategory ) )
		return TNULL;

	return it->GetOwner();
}

// $Barnyard: FUNCTION 0059faa0
AInteractionActivity::RESULT __stdcall AInteractionActivity::UpdateActivities( AInteraction* a_pInteraction, TArray<AInteractionActivity*>* a_pActivities, AInteractionSequence* a_pSequence, AInteractionActivityState* a_pState, TFLOAT a_fDeltaTime, TBOOL a_bForced )
{
	TINT iNumActivities = 0;

	if ( a_pInteraction )
		iNumActivities = a_pInteraction->GetNumActivities();
	else if ( a_pActivities )
		iNumActivities = a_pActivities->Size();

	AInteractionActivityEvent oEvent( a_pSequence );

	while ( a_pState->iActivity < iNumActivities )
	{
		a_pState->fDeltaTime = a_fDeltaTime;
		a_pState->fTime += a_fDeltaTime;

		AInteractionActivity* pActivity = TNULL;

		if ( a_pInteraction )
			pActivity = a_pInteraction->GetActivity( a_pState->iActivity );
		else if ( a_pActivities )
			pActivity = ( *a_pActivities )[ a_pState->iActivity ];

		// The sign bit of the flags is set once the activity has been started
		TBOOL bStarted = TFALSE;

		if ( a_pState->iFlags >= 0 )
		{
			bStarted = TTRUE;

			if ( pActivity->OnStart( a_pSequence ) == RESULT_FINISH_SEQUENCE )
				return RESULT_FINISH_SEQUENCE;

			a_pState->iFlags |= AInteractionActivityState::FLAGS_STARTED;
		}

		RESULT eResult = pActivity->Update( a_pSequence );

		if ( bStarted )
		{
			oEvent.Set( pActivity, AInteractionActivityEvent::TYPE_STARTED );
			TGlobalEmitter<AInteractionActivityEvent>::Throw( oEvent );
		}

		if ( eResult != RESULT_DONE )
		{
			if ( eResult == RESULT_FINISH_SEQUENCE )
			{
				oEvent.Set( pActivity, AInteractionActivityEvent::TYPE_SEQUENCE_FINISH );
				TGlobalEmitter<AInteractionActivityEvent>::Throw( oEvent );
				return RESULT_FINISH_SEQUENCE;
			}

			return RESULT_RUNNING;
		}

		oEvent.Set( pActivity, AInteractionActivityEvent::TYPE_FINISHED );
		TGlobalEmitter<AInteractionActivityEvent>::Throw( oEvent );

		if ( a_bForced && pActivity->CanBeSkipped() )
			return RESULT_DONE;

		a_pState->Next();
	}

	return RESULT_DONE;
}
