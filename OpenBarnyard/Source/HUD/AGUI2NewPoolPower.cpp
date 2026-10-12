#include "pch.h"
#include "AGUI2NewPoolPower.h"
#include "HUD/AHudManager.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00646880
AGUI2NewPoolPower::AGUI2NewPoolPower()
{
	m_iUnk1       = 0;
	m_iUnk2       = 0;
	m_bBarVisible = TFALSE;
}

// $Barnyard: FUNCTION 00646920
AGUI2NewPoolPower::~AGUI2NewPoolPower()
{
	Detach();
}

// $Barnyard: FUNCTION 00646970
TBOOL AGUI2NewPoolPower::Create( TFLOAT a_fUnused, T2GUIElement* a_pUnused, Mask a_eMask )
{
	AHudManager::GetSingleton()->AddElement( this, TFALSE, TTRUE, TFALSE );

	SetForceHide( TTRUE );
	SetMask( a_eMask );
	m_oBar.SetTranslation( 0.0f, -15.0f );

	TFLOAT fValue = 0.0f;
	TMath::Clip( fValue, 0.0f, 1.0f );
	m_fTargetValue = fValue;

	m_oBar.SetTextureSection( T2GUITextureSectionManager::GetTextureSection( "Pool_Power_Bar" ) );
	SetOrientation( TTRUE, "Dag_Red", "Dag_Red" );
	SetBracketsVisible( TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 006468a0
void AGUI2NewPoolPower::SetBarVisible( TBOOL a_bVisible )
{
	m_bBarVisible = a_bVisible;
	m_oBar.SetVisible( a_bVisible == TTRUE );
	SetForceHide( a_bVisible == TFALSE );
}

// $Barnyard: FUNCTION 006468e0
void AGUI2NewPoolPower::SetValue( TFLOAT a_fValue, TBOOL a_bImmediate )
{
	TMath::Clip( a_fValue, 0.0f, 1.0f );
	m_fTargetValue = a_fValue;

	if ( a_bImmediate )
		m_fValue = a_fValue;
}
