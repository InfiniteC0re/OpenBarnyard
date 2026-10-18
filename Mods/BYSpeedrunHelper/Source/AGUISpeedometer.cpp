#include "pch.h"
#include "AGUISpeedometer.h"
#include "AModSettings.h"

#include "GUI/AGUI2.h"
#include "Player/APlayerManager.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUISpeedometer::AGUISpeedometer()
{
}

AGUISpeedometer::~AGUISpeedometer()
{
}

void AGUISpeedometer::Create()
{
	auto pFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_pTextBox = new AGUI2TextBox();
	m_pTextBox->Create( pFont, 200.0f );
	m_pTextBox->SetText( L"0.00 m/s" );

	ApplyUIStyle();
}

void AGUISpeedometer::ApplyUIStyle()
{
	if ( !m_pTextBox )
		return;

	m_pTextBox->SetColour( TCOLOR( TUINT( g_oSettings.vecHUDColor.x * 255.0f ), TUINT( g_oSettings.vecHUDColor.y * 255.0f ), TUINT( g_oSettings.vecHUDColor.z * 255.0f ) ) );
	m_pTextBox->SetAlpha( g_oSettings.vecHUDColor.w );
	m_pTextBox->SetInFront();
	m_pTextBox->SetTextAlign( T2GUIFont::TextAlign_Left );
	m_pTextBox->SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
}

void AGUISpeedometer::UpdateUIPosition( TFLOAT a_fY )
{
	if ( !m_pTextBox )
		return;

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_pTextBox->SetTransform( -fWidth / 2 + 6.0f, a_fY );
}

void AGUISpeedometer::Update()
{
	if ( !m_pTextBox )
		return;

	TFLOAT fVelocity = 0.0f;

	if ( APlayerManager::IsSingletonCreated() )
	{
		AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

		if ( pPlayer )
		{
			ASteer* pSteer = pPlayer->GetSteer();

			if ( pSteer )
			{
				TVector4 vecVelocity;
				pSteer->CopyVelocity( vecVelocity );

				fVelocity = vecVelocity.MagnitudeSqXZ();

				if ( fVelocity != 0 )
					fVelocity = TMath::Sqrt( fVelocity );
			}
		}
	}

	static T2FormatString<32, T2StringTraits<TWCHAR>> s_FormatString;
	s_FormatString.Format( L"%.2f m/s", fVelocity );

	m_pTextBox->SetText( s_FormatString.Get() );
}

void AGUISpeedometer::Render()
{
	if ( m_pTextBox )
	{
		m_pTextBox->PreRender();
		m_pTextBox->Render();
		m_pTextBox->PostRender();
	}
}
