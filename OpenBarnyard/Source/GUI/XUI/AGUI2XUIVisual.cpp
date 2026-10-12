#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUIVisual.h"
#  include "AGUI2XUITextPresenter.h"
#  include "AGUI2XUIImagePresenter.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUI2XUIVisual );

// $BarnyardWiiJP: FUNCTION 8041fac0
AGUI2XUIVisual::AGUI2XUIVisual()
    : m_oTimelinePlayer( &m_oTimelineControl )
{
}

// $BarnyardWiiJP: FUNCTION 8041fb18
// $BarnyardWiiJP: FUNCTION 8041fe9c
AGUI2XUIVisual::~AGUI2XUIVisual()
{
}

// $BarnyardWiiJP: FUNCTION 8041fb8c
TBOOL AGUI2XUIVisual::Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	AGUI2XUIElement::Create( a_rResource, a_pData, TTRUE );
	m_oTimelineControl.Create( this, a_rResource, a_pData );

	return TTRUE;
}

// $BarnyardWiiJP: FUNCTION 8041fbec
void AGUI2XUIVisual::Tick( TFLOAT a_fDeltaTime )
{
	if ( IsVisible() )
		m_oTimelinePlayer.Tick( a_fDeltaTime );

	AGUI2XUIElement::Tick( a_fDeltaTime );
}

// $BarnyardWiiJP: FUNCTION 8041fc68
void AGUI2XUIVisual::SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath )
{
	if ( !a_wszText && !a_wszImagePath )
		return;

	for ( T2GUIElement* it = ChildRBegin(); it != TNULL; it = GetPrevChild( it ) )
	{
		if ( !it->HasFlags( T2GUIElement::FLAGS_XUIELEMENT ) )
			continue;

		AGUI2XUIElement* pChild = TSTATICCAST( AGUI2XUIElement, it );

		if ( pChild->GetClass()->IsA( &TGetClass( AGUI2XUITextPresenter ) ) )
		{
			if ( a_wszText )
				TSTATICCAST( AGUI2XUITextPresenter, pChild )->SetText( a_wszText );
		}
		else if ( pChild->GetClass()->IsA( &TGetClass( AGUI2XUIImagePresenter ) ) && a_wszImagePath )
		{
			TSTATICCAST( AGUI2XUIImagePresenter, pChild )->SetImage( a_wszImagePath );
		}
	}
}

// $BarnyardWiiJP: FUNCTION 8041fd84
void AGUI2XUIVisual::PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame )
{
	m_oTimelinePlayer.PlayAnimation( a_wszStartFrame, a_wszEndFrame );

	// The children play the same range of the timeline
	AGUI2XUIElement::PlayTimelineRange( m_oTimelinePlayer.GetStartTime(), m_oTimelinePlayer.GetEndTime() );
}

// $BarnyardWiiJP: FUNCTION 8041fdc4
void AGUI2XUIVisual::StopAnimation()
{
	m_oTimelinePlayer.Stop();
	AGUI2XUIElement::StopAnimation();
}

// $BarnyardWiiJP: FUNCTION 8041fdfc
void AGUI2XUIVisual::OnPress( TBOOL a_bEnded )
{
	// Visuals pass it on to the control that owns them
	TSTATICCAST( AGUI2XUIElement, m_pParent )->OnPress( a_bEnded );
}

#endif // BARNYARD_LATEST
