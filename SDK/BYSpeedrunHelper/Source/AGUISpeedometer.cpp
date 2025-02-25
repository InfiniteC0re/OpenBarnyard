#include "pch.h"
#include "AGUISpeedometer.h"
#include "HookHelpers.h"
#include "AModSettings.h"

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

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions( fWidth, fHeight );

	m_pTextBox->SetColour( TCOLOR( TUINT( g_oSettings.vecHUDColor.x * 255.0f ), TUINT( g_oSettings.vecHUDColor.y * 255.0f ), TUINT( g_oSettings.vecHUDColor.z * 255.0f ) ) );
	m_pTextBox->SetTransform( -fWidth / 2 + 6.0f, 24.0f );
	m_pTextBox->SetAlpha( g_oSettings.vecHUDColor.w );
	m_pTextBox->SetInFront();
	m_pTextBox->SetTextAlign( AGUI2Font::TextAlign_Left );
	m_pTextBox->SetAttachment( AGUI2Element::Anchor_MiddleLeft, AGUI2Element::Pivot_MiddleLeft );
}

struct AUnitPlayer
    : public TObject
{
};

struct APlayerManager;
struct ASteer : public TObject
{
	virtual void Unknown1();
	virtual void Unknown2();
	virtual void Unknown3();
	virtual void Unknown4();
	virtual void Unknown5();
	virtual void Unknown6();
	virtual void Unknown7();
	virtual void Unknown8();
	virtual void Unknown9();
	virtual void Unknown10();
	virtual void Unknown11();
	virtual void Unknown12();
	virtual void Unknown13();
	virtual void Unknown14();
	virtual void Unknown15();
	virtual void Unknown16();
	virtual void Unknown17();
	virtual void Unknown18();
	virtual void Unknown19();
	virtual void Unknown20();
	virtual void Unknown21();
	virtual void Unknown22();
	virtual void Unknown23();
	virtual void Unknown24();
	virtual void Unknown25();
	virtual void Unknown26();
	virtual void Unknown27();
	virtual void Unknown28();
	virtual void Unknown29();
	virtual void Unknown30();
	virtual void Unknown31();
	virtual void Unknown32();
	virtual void Unknown33();
	virtual void Unknown34();
	virtual void Unknown35();
	virtual void Unknown36();
	virtual void Unknown37();
	virtual void Unknown38();
	virtual void Unknown39();
	virtual void Unknown40();
	virtual void Unknown41();
	virtual void Unknown42();
	virtual void Unknown43();
	virtual void Unknown44();
	virtual void Unknown45();
	virtual void GetVelocityVector( TVector4& a_rOutVector );
};

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

				TFLOAT fVelocity = vecVelocity.MagnitudeSqXZ();

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
	if ( m_pTextBox && g_oSettings.bShowSpeedometer )
	{
		m_pTextBox->PreRender();
		m_pTextBox->Render();
		m_pTextBox->PostRender();
	}
}
