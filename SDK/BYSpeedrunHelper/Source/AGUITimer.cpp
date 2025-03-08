#include "pch.h"
#include "AGUITimer.h"
#include "AModSettings.h"

#include <BYardSDK/AGUI2.h>

#include <Toshi/THPTimer.h>

TOSHI_NAMESPACE_USING

AGUITimer::AGUITimer()
{
	m_pTextBox       = TNULL;
	m_fTotalTime     = 0.0f;
	m_wcsPrefix      = TNULL;
	m_bPrefixEnabled = TFALSE;
}

AGUITimer::~AGUITimer()
{
	if ( m_pTextBox )
	{
		delete m_pTextBox;
		m_pTextBox = TNULL;
	}
}

void AGUITimer::Create()
{
	auto pFont = AGUI2FontManager::FindFont( "Rekord18" );
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
	m_pTextBox->Create( pFont, 200.0f );
	m_pTextBox->SetText( L"0.000" );
	
	ApplyUIStyle();
}

void AGUITimer::ApplyUIStyle()
{
	if ( !m_pTextBox )
		return;

	m_pTextBox->SetColour( TCOLOR( TUINT( g_oSettings.vecHUDColor.x * 255.0f ), TUINT( g_oSettings.vecHUDColor.y * 255.0f ), TUINT( g_oSettings.vecHUDColor.z * 255.0f ) ) );
	m_pTextBox->SetAlpha( g_oSettings.vecHUDColor.w  );
	m_pTextBox->SetInFront();
	m_pTextBox->SetTextAlign( AGUI2Font::TextAlign_Left );
	m_pTextBox->SetAttachment( AGUI2Element::Anchor_MiddleLeft, AGUI2Element::Pivot_MiddleLeft );
}

void AGUITimer::UpdateUIPosition( TFLOAT a_fY )
{
	if ( !m_pTextBox )
		return;

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_pTextBox->SetTransform( -fWidth / 2 + 6.0f, a_fY );
}

void AGUITimer::Update()
{
	TINT iMilliseconds, iSeconds, iMinutes, iHours;
	GetTime( m_fTotalTime, iMilliseconds, iSeconds, iMinutes, iHours );

	TINT iTimeStrStart = 0;
	if ( m_bPrefixEnabled && m_wcsPrefix )
	{
		iTimeStrStart = TStringManager::String16Length( m_wcsPrefix );
		TStringManager::String16Copy( m_wcsBuffer, m_wcsPrefix, iTimeStrStart );
	}

	if ( iHours != 0 )
	{
		TStringManager::String16Format( m_wcsBuffer + iTimeStrStart, TARRAYSIZE( m_wcsBuffer ) - iTimeStrStart, L"%02d:%02d:%02d.%03d", iHours, iMinutes, iSeconds, iMilliseconds );
	}
	else if ( iMinutes != 0 )
	{
		TStringManager::String16Format( m_wcsBuffer + iTimeStrStart, TARRAYSIZE( m_wcsBuffer ) - iTimeStrStart, L"%02d:%02d.%03d", iMinutes, iSeconds, iMilliseconds );
	}
	else
	{
		TStringManager::String16Format( m_wcsBuffer + iTimeStrStart, TARRAYSIZE( m_wcsBuffer ) - iTimeStrStart, L"%d.%03d", iSeconds, iMilliseconds );
	}

	m_pTextBox->SetText( m_wcsBuffer );
}

void AGUITimer::Render()
{
	if ( m_pTextBox )
	{
		m_pTextBox->PreRender();
		m_pTextBox->Render();
		m_pTextBox->PostRender();
	}
}

void AGUITimer::EnablePrefix( const TWCHAR* a_wcsPrefix, TBOOL a_bEnable )
{
	m_wcsPrefix = a_wcsPrefix;
	m_bPrefixEnabled = a_bEnable;
}

void AGUITimer::GetTime( TFLOAT a_fTime, TINT& a_iMilliseconds, TINT& a_iSeconds, TINT& a_iMinutes, TINT& a_iHours )
{
	a_iMilliseconds = TMath::FloorToInt( fmod( a_fTime, 1.0f ) * 1000 );
	a_iSeconds      = TMath::FloorToInt( fmod( a_fTime, 60.0f ) );
	a_iMinutes      = TMath::FloorToInt( fmod( a_fTime / 60.0f, 60.0f ) );
	a_iHours        = TMath::FloorToInt( a_fTime / 3600.0f );
}
