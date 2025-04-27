#include "pch.h"
#include "AGUISpeedometer.h"
#include "HookHelpers.h"
#include "AModSettings.h"

#include <BYardSDK/ASteer.h>
#include <BYardSDK/AGUI2.h>

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
	auto pFont = AGUI2FontManager::FindFont( "Rekord18" );
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
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
	m_pTextBox->SetTextAlign( AGUI2Font::TextAlign_Left );
	m_pTextBox->SetAttachment( AGUI2Element::Anchor_MiddleLeft, AGUI2Element::Pivot_MiddleLeft );
}

void AGUISpeedometer::UpdateUIPosition( TFLOAT a_fY )
{
	if ( !m_pTextBox )
		return;

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_pTextBox->SetTransform( -fWidth / 2 + 6.0f, a_fY );
}

struct AUnitPlayer
    : public TObject
{
};

struct APlayerManager;

void AGUISpeedometer::Update()
{
	if ( !m_pTextBox )
		return;

	APlayerManager* pPlayerManager = *(APlayerManager**)0x007b48a8;
	TFLOAT          fVelocity      = 0.0f;

	if ( pPlayerManager )
	{
		AUnitPlayer* pPlayer = CALL_THIS( 0x0061faf0, APlayerManager*, AUnitPlayer*, pPlayerManager, TINT, 0 );

		if ( pPlayer )
		{
			ASteer* pSteer = *(ASteer**)( TUINTPTR( pPlayer ) + 0x1C );

			if ( pSteer )
			{
				TVector4 vecVelocity;
				pSteer->GetVelocityVector( vecVelocity );

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
