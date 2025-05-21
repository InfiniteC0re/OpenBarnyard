#include "pch.h"
#include "ASlideshowState.h"
#include "AGameStateController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS_NORUNTIME( ASlideshowState );
TPSTRING8_DECLARE( ASlideshowState );

// $Barnyard: FUNCTION 0042ed40
ASlideshowState::ASlideshowState( const AGUISlideshow::Params& a_rSlideShowParams, AGameState* a_pNextGameState, TBOOL a_bShouldLocalise )
{
	m_pNextState = a_pNextGameState;
	m_pInputMap  = AInputMapManager::GetSingleton()->FindMap( TPS8( ASlideshowState ) );
	m_Slideshow.Setup( &m_HUDParams, a_rSlideShowParams, a_bShouldLocalise );
}

// $Barnyard: FUNCTION 0042ecc0
TBOOL ASlideshowState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	if ( m_Slideshow.ProcessInput( a_pInputEvent ) )
	{
		return TTRUE;
	}

	return AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0042ecf0
TBOOL ASlideshowState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	return m_Slideshow.ProcessCommand( a_eInputCommand );
}

// $Barnyard: FUNCTION 0042ee40
TBOOL ASlideshowState::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_Slideshow.Update( a_fDeltaTime );

	if ( m_Slideshow.IsSlideshowOver() )
	{
		if ( m_pNextState )
		{
			AGameStateController::GetSingleton()->ReplaceState( m_pNextState );
			return TTRUE;
		}
		else
		{
			AGameState::Remove();
			return TTRUE;
		}
	}
	else
	{
		return AGameState::OnUpdate( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0042edf0
void ASlideshowState::OnInsertion()
{
	if ( !m_Slideshow.HasSlides() )
	{
		if ( m_pNextState )
		{
			AGameStateController::GetSingleton()->ReplaceState( m_pNextState );
		}
		else
		{
			AGameState::Remove();
		}
	}

	AInputMapManager::GetSingleton()->SetMap( TPS8( ASlideshowState ) );
	AGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0042eca0
void ASlideshowState::OnActivate()
{
	m_Slideshow.Activate();
	AGameState::OnActivate();
}

// $Barnyard: FUNCTION 0042eed0
void ASlideshowState::OnDeactivate()
{
	m_Slideshow.Deactivate();
	m_Slideshow.Reset();
	AGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0042ed30
void ASlideshowState::AddSlide( const Toshi::TPString8& a_rName )
{
	m_Slideshow.AddSlide( a_rName );
}
