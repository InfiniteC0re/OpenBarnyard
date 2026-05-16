#include "pch.h"
#include "AHudElement.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS( AHudElement );

// $Barnyard: FUNCTION 0044ef90
AHudElement::AHudElement( Mask a_eMask )
    : m_eMask( a_eMask )
    , m_bForceHide( TFALSE )
    , m_bCurrentlyVisible( TTRUE )
    , m_bOwnedByManager( TFALSE )
{
}

// $Barnyard: FUNCTION 0044efc0
// $Barnyard: FUNCTION 0044ef70
AHudElement::~AHudElement()
{
}

void AHudElement::Unknown1()
{
}

void AHudElement::Unknown2()
{
}

TBOOL AHudElement::OnAttach()
{
	return TTRUE;
}

void AHudElement::OnDetach()
{
}

// $Barnyard: FUNCTION 0044ef80
void AHudElement::Show()
{
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 004512b0
void AHudElement::Hide()
{
	m_bCurrentlyVisible = TFALSE;
}

void AHudElement::Update( TFLOAT a_fDeltaTime )
{
}

void AHudElement::Reset()
{
}

// $Barnyard: FUNCTION 0044f060
void AHudElement::Detach()
{
	if ( IsLinked() )
	{
		OnDetach();
		Remove();
	}
}
