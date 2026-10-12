#include "pch.h"
#include "AFadeActivity.h"
#include "GameInterface/AGameStateController.h"
#include "MiniGame/AMiniGameManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b5d20, CreateObject 005b5d70
// NOTE: GetClass 005b5d20, CreateObject 005b5d70, CreateObjectInPlace 005b5df0
TDEFINE_CLASS( AFadeActivity );

TPSTRING8_DECLARE( fade );
TPSTRING8_DECLARE( in );
TPSTRING8_DECLARE( out );
TPSTRING8_DECLARE( time );

// NOTE: retail leaves m_uiUnk08 uninitialised
AFadeActivity::AFadeActivity()
    : m_uiFlags( FLAGS_NONE )
    , m_fRate( -1.0f )
{
}

// $Barnyard: FUNCTION 005b5d30
// $Barnyard: FUNCTION 005b5d40
AFadeActivity::~AFadeActivity()
{
}

// $Barnyard: FUNCTION 005b6020
TBOOL AFadeActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strFade;
	a_pProperties->GetOptionalPropertyValue( strFade, TPS8( fade ) );

	if ( strFade == TPS8( in ) )
		m_uiFlags |= FLAGS_FADE_IN;
	else if ( strFade == TPS8( out ) )
		m_uiFlags |= FLAGS_FADE_OUT;

	TFLOAT fTime = 1.0f;

	if ( a_pProperties->GetOptionalPropertyValue( fTime, TPS8( time ) ) && fTime > 0.0f )
		m_fRate = 1.0f / fTime;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b5d60
void AFadeActivity::GetDataSize( TINT* a_pSize )
{
	// The alpha of the overlay
	*a_pSize = sizeof( TFLOAT );
}

// $Barnyard: FUNCTION 005b5e60
AInteractionActivity::RESULT AFadeActivity::OnStart( AInteractionSequence* a_pSequence )
{
	TFLOAT* pAlpha  = TSTATICCAST( TFLOAT, a_pSequence->GetActivityData() );
	FLAGS   uiFlags = m_uiFlags;

	m_uiFlags &= ~( FLAGS_FADED | FLAGS_FINISHED );

	// The alpha to start from, a negative rate fades the other way
	if ( HASANYFLAG( uiFlags, FLAGS_FADE_IN ) )
		*pAlpha = ( m_fRate > 0.0f ) ? 1.0f : 0.0f;
	else if ( HASANYFLAG( uiFlags, FLAGS_FADE_OUT ) )
		*pAlpha = ( m_fRate <= 0.0f ) ? 1.0f : 0.0f;
	else
		return RESULT_DONE;

	if ( !AMiniGameManager::ms_bHasFade || !HASANYFLAG( m_uiFlags, FLAGS_FADE_OUT ) )
	{
		SetOverlayAlpha( *pAlpha );
		AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_3 );
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005b5f10
AInteractionActivity::RESULT AFadeActivity::Update( AInteractionSequence* a_pSequence )
{
	TFLOAT  fDeltaTime = a_pSequence->GetActivityState().fDeltaTime;
	TFLOAT* pAlpha     = TSTATICCAST( TFLOAT, a_pSequence->GetActivityData() );
	RESULT  eResult    = RESULT_RUNNING;

	if ( HASANYFLAG( m_uiFlags, FLAGS_FADE_IN ) )
	{
		AMiniGameManager::ms_bHasFade = TFALSE;

		if ( !HASANYFLAG( m_uiFlags, FLAGS_FADED ) )
		{
			*pAlpha -= fDeltaTime * m_fRate;

			if ( *pAlpha <= 0.0f )
			{
				m_uiFlags |= FLAGS_FADED;
				*pAlpha = 0.0f;
			}
		}
		else if ( !HASANYFLAG( m_uiFlags, FLAGS_FINISHED ) )
		{
			m_uiFlags |= FLAGS_FINISHED;
		}
		else
		{
			// Faded in, hide the overlay
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );
			return RESULT_DONE;
		}
	}
	else
	{
		if ( !HASANYFLAG( m_uiFlags, FLAGS_FADE_OUT ) )
		{
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );
			return RESULT_DONE;
		}

		// A mini game has faded out already
		if ( AMiniGameManager::ms_bHasFade )
		{
			AMiniGameManager::ms_bHasFade = TFALSE;
			return RESULT_DONE;
		}

		if ( !HASANYFLAG( m_uiFlags, FLAGS_FADED ) )
		{
			*pAlpha += fDeltaTime * m_fRate;

			if ( *pAlpha >= 1.0f )
			{
				m_uiFlags |= FLAGS_FADED;
				*pAlpha = 1.0f;
			}
		}
		else if ( !HASANYFLAG( m_uiFlags, FLAGS_FINISHED ) )
		{
			m_uiFlags |= FLAGS_FINISHED;
		}
		else
		{
			// NOTE: the overlay is hidden but its alpha is still set below
			AGameStateController::GetSingleton()->GetCurrentState()->SetOverlay( AGameState::OVERLAY_1 );
			eResult = RESULT_DONE;
		}
	}

	SetOverlayAlpha( *pAlpha );
	return eResult;
}

void AFadeActivity::SetOverlayAlpha( TFLOAT a_fAlpha )
{
	AGameStateController::OverlayData oData;
	oData.uiColorA = TUINT8( TINT( a_fAlpha * 255.0f ) );
	oData.uiColorR = 0;
	oData.uiColorG = 0;
	oData.uiColorB = 0;

	AGameStateController::GetSingleton()->SetOverlayParams( AGameState::OVERLAY_3, oData );
}
