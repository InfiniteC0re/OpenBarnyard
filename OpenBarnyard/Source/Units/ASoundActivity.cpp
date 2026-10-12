#include "pch.h"
#include "ASoundActivity.h"
#include "AUnit.h"
#include "Sound/ASoundManager.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bd9b0, CreateObject 005bd9d0, CreateObjectInPlace 005bda50
TDEFINE_CLASS( ASoundActivity );

// NOTE: inlined in CreateObject
ASoundActivity::ASoundActivity()
    : m_eType( TYPE_SFX )
    , m_iSound( -1 )
    , m_iCue( -1 )
{
}

// $Barnyard: FUNCTION 005bd9c0
// $Barnyard: FUNCTION 005bdac0
ASoundActivity::~ASoundActivity()
{
}

// $Barnyard: FUNCTION 005bdae0
TBOOL ASoundActivity::Create( const PBProperties* a_pProperties )
{
	// NOTE: retail checks the properties one by one in this order
	static constexpr const TCHAR* s_aTypeNames[] = {
		"sfx",          // TYPE_SFX
		"sfx3d",        // TYPE_SFX3D
		"stop",         // TYPE_STOP
		"fadeout",      // TYPE_FADEOUT
		"fadein",       // TYPE_FADEIN
		"fadestop",     // TYPE_FADESTOP
		"streamedvo3d", // TYPE_STREAMEDVO3D
	};

	for ( TINT i = 0; i < TARRAYSIZE( s_aTypeNames ); i++ )
	{
		const PBPropertyValue* pValue = a_pProperties->GetProperty( s_aTypeNames[ i ] );

		if ( pValue )
		{
			m_eType  = i;
			m_iSound = ASoundId( TREINTERPRETCAST( TUINTPTR, pValue->GetRaw() ) );

			return AInteractionActivity::Create( a_pProperties );
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005bdc30
AInteractionActivity::RESULT ASoundActivity::Update( AInteractionSequence* a_pSequence )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( m_iSound == -1 )
		return RESULT_DONE;

	switch ( m_eType )
	{
		case TYPE_SFX3D:
		{
			AUnit* pUnit = ( m_eCategory == AInteractionSequence::CATEGORY_TARGET ) ? a_pSequence->GetUnit( 0 ) : a_pSequence->GetUnitByCategory( m_eCategory );

			// NOTE: retail doesn't keep the cue
			pSoundManager->PlayCueAtPosition( m_iSound, *pUnit->GetSteer()->GetPosition() );
			return RESULT_DONE;
		}
		case TYPE_STOP:
#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: retail jumps back to the StopCue call forever
			pSoundManager->StopCue( m_iCue );
			m_iCue = -1;
			return RESULT_DONE;
#else
			while ( TTRUE )
			{
				pSoundManager->StopCue( m_iCue );
				m_iCue = -1;
			}
#endif
		case TYPE_FADEOUT:
			ASoundManager::GetSingleton()->FadeOutCue( m_iCue, 3.0f, TFALSE, 3.0f );
			return RESULT_DONE;
		case TYPE_FADESTOP:
			ASoundManager::GetSingleton()->FadeOutCue( m_iCue, 3.0f, TTRUE, 3.0f );
			m_iCue = -1;
			return RESULT_DONE;
		case TYPE_FADEIN:
			ASoundManager::GetSingleton()->FadeCue( m_iCue, 3.0f, 1.0f, 3.0f );
			return RESULT_DONE;
		case TYPE_STREAMEDVO3D:
			if ( m_iCue == -1 )
			{
				m_iCue = pSoundManager->PlayStreamedVO( m_iSound );
			}
			else if ( pSoundManager->IsStreamedVOQueued( m_iCue ) )
			{
				pSoundManager->SetStreamedVOPosition( m_iCue, *a_pSequence->GetUnit( 0 )->GetSteer()->GetPosition() );
				return RESULT_DONE;
			}

			return RESULT_RUNNING;
		default:
			m_iCue = pSoundManager->PlayCue( m_iSound );
			return RESULT_DONE;
	}
}
