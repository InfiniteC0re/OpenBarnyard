#include "pch.h"
#include "AContextButtonHudElement.h"
#include "GUI/AGUI2.h"

#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00450e00, CreateObject 004510e0 and CreateObjectInPlace 00451140 of the TClass
// $Barnyard: FUNCTION 004510e0
// $Barnyard: FUNCTION 00451140
TDEFINE_CLASS( AContextButtonHudElement );

T2GUITextureSection* AContextButtonHudElement::ms_apButtonSections[ NUM_BUTTONS ];

// 0x00773654
static const TCHAR* s_apszButtonSections[ AContextButtonHudElement::NUM_BUTTONS ] = {
	"button_triangle",
	"button_circle",
	"button_cross",
	"button_square"
};

// Position of the container (0x0078a6ec and 0x0078a6f4, never set)
static TINT s_iPosX;
static TINT s_iPosY;

AContextButtonHudElement::Button::Button()
{
}

AContextButtonHudElement::Button::~Button()
{
}

// $Barnyard: FUNCTION 00451030
AContextButtonHudElement::AContextButtonHudElement()
    : AHudElement( MASK_CONTEXT_BUTTON )
{
	m_iUnk14 = 0;

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		ms_apButtonSections[ i ] = T2GUITextureSectionManager::GetTextureSection( s_apszButtonSections[ i ] );
}

// $Barnyard: FUNCTION 00450fc0
// $Barnyard: FUNCTION 004510c0
AContextButtonHudElement::~AContextButtonHudElement()
{
}

// $Barnyard: FUNCTION 00450f40
TBOOL AContextButtonHudElement::OnAttach()
{
	// NOTE: the original adds 1 to m_iUnk14 for each of the buttons
	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		m_iUnk14++;

	return TTRUE;
}

// $Barnyard: FUNCTION 00450f60
void AContextButtonHudElement::OnDetach()
{
	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
	{
		m_aButtons[ i ].oElement.RemoveSelf();
		m_aButtons[ i ].oIcon.RemoveSelf();
		m_aButtons[ i ].oText.RemoveSelf();
	}
}

// $Barnyard: FUNCTION 00450e10
void AContextButtonHudElement::Show()
{
	T2GUIElement* pRoot = AGUI2::GetRootElement();

	m_oContainer.SetTransform( TFLOAT( s_iPosX ), TFLOAT( s_iPosY ) );
	m_oContainer.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	pRoot->AddChildTail( m_oContainer );

	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00450e60
void AContextButtonHudElement::Hide()
{
	m_oContainer.RemoveSelf();
	m_bCurrentlyVisible = TFALSE;
}
