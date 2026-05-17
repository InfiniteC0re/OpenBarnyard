#include "pch.h"
#include "AGUI2Timer.h"

#include "Sound/ASoundManager.h"

#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0064d750
AGUI2Timer::AGUI2Timer()
{
	m_fDuration        = 0.0f;
	m_fTimeRatio       = 0.0f;
	m_fTickAccumulator = 0.0f;
	m_fTickElapsed     = 0.0f;
	m_fTickInterval    = 0.0f;
	m_eTimerCue        = TimerCue_None;
	m_iCueIndex        = -1;
}

// $Barnyard: FUNCTION 0064d800
// $Barnyard: FUNCTION 0064db70
AGUI2Timer::~AGUI2Timer()
{
	
}

// $Barnyard: FUNCTION 0064e1c0
void AGUI2Timer::Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	T2GUITextureSection* pBackingTexSec = T2GUITextureSectionManager::GetTextureSection( "backing" );
	TVALIDPTR( pBackingTexSec );

	m_fTextureWidth  = pBackingTexSec->GetWidth();
	m_fTextureHeight = pBackingTexSec->GetHeight();

	if ( a_fWidth == 0.0f )
	{
		a_fWidth  = m_fTextureWidth;
		a_fHeight = m_fTextureHeight;
	}
	else if ( a_fHeight == 0.0f )
	{
		a_fHeight = ( m_fTextureHeight / m_fTextureWidth ) * a_fWidth;
	}

	m_oRootElement.SetDimensions( a_fWidth, a_fHeight );
	AddChildTail( m_oRootElement );

	m_oBacking.SetDimensions( a_fWidth, a_fHeight );
	m_oBacking.SetTextureSection( pBackingTexSec );
	m_oRootElement.AddChildTail( m_oBacking );

	TFLOAT fWidth, fHeight;
	m_oBacking.GetDimensions( fWidth, fHeight );
	T2GUIElement::SetDimensions( fWidth, fHeight );

	m_pClockTextureSection = T2GUITextureSectionManager::GetTextureSection( "clockwhite" );
	TVALIDPTR( m_pClockTextureSection );

	m_oClockPolygon1.CreateBuffers( 5 );
	m_oClockPolygon1.SetMaterial( m_pClockTextureSection->GetMaterial() );
	m_oClockPolygon1.SetTransform( 0.0f, fHeight * 0.05f, 0.0f );
	m_oClockPolygon1.SetAlpha( 0.5f );
	m_oBacking.AddChildTail( m_oClockPolygon1 );

	m_oClockPolygon2.CreateBuffers( 5 );
	m_oClockPolygon2.SetMaterial( m_pClockTextureSection->GetMaterial() );
	m_oClockPolygon2.SetTransform( 0.0f, fHeight * 0.05f, 0.0f );
	m_oClockPolygon2.SetAlpha( 0.5f );
	m_oBacking.AddChildTail( m_oClockPolygon2 );

	m_fDuration = 1.0f;
	SetTime( 1.0f );
	m_bPaused = TFALSE;
}

// $Barnyard: FUNCTION 0064d8d0
void AGUI2Timer::SetDuration( TFLOAT a_fDuration )
{
	m_fDuration = a_fDuration;
}

// $Barnyard: FUNCTION 0064d8e0
void AGUI2Timer::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_oBacking.SetDimensions( a_fWidth, a_fHeight );
	m_oClockPolygon2.SetTransform( 0.0f, a_fHeight * 0.05f, 0.0f );
	m_oClockPolygon1.SetTransform( 0.0f, a_fHeight * 0.05f, 0.0f );
	T2GUIElement::SetDimensions( a_fWidth, a_fHeight );
}

// $Barnyard: FUNCTION 0064d950
void AGUI2Timer::SetAlpha( TFLOAT a_fAlpha )
{
	m_oBacking.SetAlpha( a_fAlpha );
	m_oClockPolygon2.SetAlpha( a_fAlpha * 0.5f );
	m_oClockPolygon1.SetAlpha( a_fAlpha * 0.5f );
}

// $Barnyard: FUNCTION 0064d9a0
void AGUI2Timer::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bPaused || m_fTimeRatio > TIMER_WARNING_RATIO )
		return;

	if ( m_fTimeRatio > 0.0f )
	{
		m_fTickInterval = 0.01f;
		m_fTickElapsed += a_fDeltaTime;
	}
	else
	{
		m_fTickElapsed = 0.0f;
		m_fTickInterval = ( m_fTimeRatio > TIMER_CRITICAL_RATIO ) ? 0.25f : 0.1f;
	}

	if ( m_fTickElapsed > TIMER_TICK_COMPLETE_TIME )
	{
		m_oBacking.GetTransform().SetIdentity();
	}
	else
	{
		m_fTickAccumulator += a_fDeltaTime;

		if ( m_fTickInterval <= m_fTickAccumulator )
		{
			m_fTickAccumulator = 0.0f;
			m_oBacking.GetTransform().SetIdentity();
			m_oBacking.GetTransform().Rotate( m_fTickRotation );

			ToggleTickRotation();
		}
	}
}

// $Barnyard: FUNCTION 0064daf0
TBOOL AGUI2Timer::IsTimerWarning() const
{
	return m_eTimerCue == TimerCue_Warning;
}

// $Barnyard: FUNCTION 0064db10
TBOOL AGUI2Timer::IsTimerCritical() const
{
	return m_eTimerCue == TimerCue_Critical;
}

// $Barnyard: FUNCTION 0064db30
TBOOL AGUI2Timer::IsTimerFull() const
{
	return m_eTimerCue == TimerCue_Full;
}

// $Barnyard: FUNCTION 0064db50
TBOOL AGUI2Timer::IsTickComplete() const
{
	return m_fTickElapsed > TIMER_TICK_COMPLETE_TIME;
}

// $Barnyard: FUNCTION 0064db90
void AGUI2Timer::SetTime( TFLOAT a_fTime )
{
	m_fTimeRatio = a_fTime / m_fDuration;
	TMath::Clip( m_fTimeRatio, 0.0f, 1.0f );

	TimerCue eNewCue = TimerCue_None;

	if ( m_fTimeRatio > TIMER_WARNING_RATIO )
	{
		SetClockColour( 0xff00ff00, 0.7f );
	}
	else if ( m_fTimeRatio > 0.0f )
	{
		eNewCue = ( m_fTimeRatio > TIMER_CRITICAL_RATIO ) ? TimerCue_Warning : TimerCue_Critical;
	}
	else
	{
		eNewCue = TimerCue_Full;
	}

	if ( m_eTimerCue != eNewCue )
	{
		m_eTimerCue = eNewCue;

		ASoundManager::GetSingleton()->StopCue( m_iCueIndex );

		if ( m_eTimerCue != TimerCue_None )
		{
			m_iCueIndex = ASoundManager::GetSingleton()->PlayCue( m_eTimerCue );
		}

		m_oBacking.GetTransform().SetIdentity();
		m_oBacking.GetTransform().Rotate( m_fTickRotation );
		ToggleTickRotation();
		m_fTickAccumulator = 0.0f;

		if ( m_eTimerCue != TimerCue_Full )
		{
			SetClockColour( m_eTimerCue == TimerCue_Warning ? 0xffffc800 : 0xffff0500, 0.7f );
		}
	}

	UpdateClockPolygons();
}

// $Barnyard: FUNCTION 0064e080
void AGUI2Timer::SetVisible( TBOOL a_bVisible )
{
	m_oBacking.SetVisible( a_bVisible );
	m_oClockPolygon1.SetVisible( a_bVisible );
	m_oClockPolygon2.SetVisible( a_bVisible );

	ASoundManager::GetSingleton()->StopCue( m_iCueIndex );

	m_iCueIndex = -1;
	T2GUIElement::SetVisible( a_bVisible );
}

// $Barnyard: FUNCTION 0064e120
void AGUI2Timer::StopCue()
{
	ASoundManager::GetSingleton()->StopCue( m_iCueIndex );

	m_iCueIndex = -1;
	m_eTimerCue = TimerCue_None;
}

// $Barnyard: FUNCTION 0064e150
void AGUI2Timer::SetPaused( TBOOL a_bPaused )
{
	if ( m_bPaused != a_bPaused )
	{
		m_bPaused = a_bPaused;

		ASoundManager::GetSingleton()->PauseCue( m_iCueIndex, a_bPaused );

		if ( m_bPaused )
		{
			m_oBacking.GetTransform().SetIdentity();
		}
	}
}

// $Barnyard: FUNCTION 0064e380
void AGUI2Timer::Reset()
{
	SetClockColour( 0xff00ff00, 0.7f );
	m_fTickRotation = 0.0f;
	m_oBacking.GetTransform().SetIdentity();
	m_fTickAccumulator = 0.0f;
	m_fTickElapsed = 0.0f;
	m_eTimerCue = TimerCue_None;
	StopCue();
	SetPaused( TFALSE );
}

void AGUI2Timer::SetClockColour( TUINT32 a_uiColour, TFLOAT a_fAlpha )
{
	m_oClockPolygon1.SetColour( a_uiColour );
	m_oClockPolygon2.SetColour( a_uiColour );
	m_oClockPolygon2.SetAlpha( a_fAlpha );
	m_oClockPolygon1.SetAlpha( a_fAlpha );
}

void AGUI2Timer::ToggleTickRotation()
{
	m_fTickRotation = ( m_fTickRotation == -TIMER_TICK_ROTATION ) ? TIMER_TICK_ROTATION : -TIMER_TICK_ROTATION;
}

void AGUI2Timer::UpdateClockPolygons()
{
	if ( !m_pClockTextureSection )
		return;

	m_oClockPolygon1.SetMaterial( m_pClockTextureSection->GetMaterial() );
	m_oClockPolygon2.SetMaterial( m_pClockTextureSection->GetMaterial() );

	TFLOAT fAngle = TMath::TWO_PI * m_fTimeRatio;
	TFLOAT fSecondHalfAngle = fAngle - TMath::PI;
	TVector2* pUVs = m_oClockPolygon1.GetUVs();

	if ( fSecondHalfAngle >= 0.0f )
	{
		m_oClockPolygon1.Show();

		if ( pUVs )
		{
			pUVs[ 0 ].Set( 0.5f, 1.0f );
			pUVs[ 2 ].Set( 0.5f, 0.5f );

			if ( TMath::PI * 0.25f <= fSecondHalfAngle )
			{
				pUVs[ 3 ].x = 1.0f;
				pUVs[ 1 ].Set( 1.0f, 1.0f );

				if ( fSecondHalfAngle < TMath::PI * 0.75f )
				{
					TFLOAT fX = ( TMath::Tan( TMath::PI * 0.5f - fSecondHalfAngle ) + 1.0f ) * 0.5f;
					pUVs[ 4 ].Set( 1.0f, fX );
					pUVs[ 3 ].y = fX;
				}
				else
				{
					TFLOAT fX = ( TMath::Tan( TMath::PI - fSecondHalfAngle ) + 1.0f ) * 0.5f;
					pUVs[ 3 ].y = 0.0f;
					pUVs[ 4 ].Set( fX, 0.0f );
				}
			}
			else
			{
				TFLOAT fX = ( TMath::Tan( fSecondHalfAngle ) + 1.0f ) * 0.5f;
				pUVs[ 1 ].Set( fX, 1.0f );
				pUVs[ 3 ].Set( fX, 1.0f );
				pUVs[ 4 ].Set( fX, 1.0f );
			}
		}
	}
	else
	{
		m_oClockPolygon1.Hide();
	}

	pUVs = m_oClockPolygon2.GetUVs();

	if ( pUVs )
	{
		pUVs[ 0 ].Set( 0.5f, 0.0f );
		pUVs[ 2 ].Set( 0.5f, 0.5f );

		if ( fAngle <= TMath::PI )
		{
			if ( TMath::PI * 0.75f < fAngle )
			{
				TFLOAT fX = ( 1.0f - TMath::Tan( TMath::PI - fAngle ) ) * 0.5f;
				pUVs[ 1 ].Set( 0.0f, 0.0f );
				pUVs[ 3 ].Set( 0.0f, 1.0f );
				pUVs[ 4 ].Set( fX, 1.0f );
			}
			else if ( TMath::PI * 0.25f < fAngle )
			{
				TFLOAT fY = ( TMath::Tan( fAngle - TMath::PI * 0.5f ) + 1.0f ) * 0.5f;
				pUVs[ 1 ].Set( 0.0f, 0.0f );
				pUVs[ 3 ].Set( 0.0f, fY );
				pUVs[ 4 ].Set( 0.0f, fY );
			}
			else
			{
				TFLOAT fX = ( 1.0f - TMath::Tan( fAngle ) ) * 0.5f;
				pUVs[ 1 ].Set( fX, 0.0f );
				pUVs[ 3 ].Set( fX, 0.0f );
				pUVs[ 4 ].Set( fX, 0.0f );
			}
		}
		else
		{
			pUVs[ 1 ].Set( 0.0f, 0.0f );
			pUVs[ 3 ].Set( 0.0f, 1.0f );
			pUVs[ 4 ].Set( 0.5f, 1.0f );
		}
	}

	TFLOAT fWidth = ( m_fWidth / m_fTextureWidth ) * m_pClockTextureSection->GetWidth();
	TFLOAT fHeight = ( m_fHeight / m_fTextureHeight ) * m_pClockTextureSection->GetHeight();

	UpdatePolygonVertices( m_oClockPolygon1, fWidth, fHeight );
	UpdatePolygonVertices( m_oClockPolygon2, fWidth, fHeight );
}

void AGUI2Timer::UpdatePolygonVertices( T2GUIPolygon& a_rPolygon, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TVector2* pVertices = a_rPolygon.GetVertices();
	TVector2* pUVs      = a_rPolygon.GetUVs();

	if ( !pVertices || !pUVs )
		return;

	for ( TINT i = 0; i < a_rPolygon.GetNumVertices(); i++ )
	{
		pVertices[ i ].x = ( pUVs[ i ].x - 0.5f ) * a_fWidth * TIMER_POLYGON_SCALE;
		pVertices[ i ].y = ( pUVs[ i ].y - 0.5f ) * a_fHeight * TIMER_POLYGON_SCALE;
	}

	a_rPolygon.InvalidateBounds();
}
