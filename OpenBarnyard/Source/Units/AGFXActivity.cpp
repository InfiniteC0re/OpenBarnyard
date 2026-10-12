#include "pch.h"
#include "AGFXActivity.h"
#include "AInteractionComponent.h"
#include "AVisibleUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b6eb0, CreateObject 005b6f10, CreateObjectInPlace 005b6f80
TDEFINE_CLASS( AGFXActivity );

// NOTE: inlined into CreateObject
AGFXActivity::AGFXActivity()
    : m_eVisibility( VISIBILITY_UNCHANGED )
{
}

// $Barnyard: FUNCTION 005b6ec0
// $Barnyard: FUNCTION 005b6ff0
AGFXActivity::~AGFXActivity()
{
}

// $Barnyard: FUNCTION 005b6ed0
TBOOL AGFXActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pVisible = a_pProperties->GetProperty( "visible" );

	if ( pVisible )
		m_eVisibility = pVisible->GetRaw() ? VISIBILITY_SHOW : VISIBILITY_HIDE;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b7010
AInteractionActivity::RESULT AGFXActivity::Update( AInteractionSequence* a_pSequence )
{
	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit && pUnit->IsA( &TGetClass( AVisibleUnit ) ) && m_eVisibility != VISIBILITY_UNCHANGED )
			TSTATICCAST( AVisibleUnit, pUnit )->SetVisible( m_eVisibility == VISIBILITY_SHOW );
	}

	return RESULT_DONE;
}
