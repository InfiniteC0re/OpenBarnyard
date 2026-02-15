#include "pch.h"
#include "AMiniGame.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00469a00
AMiniGame::AMiniGame()
    : m_iNumVariants( 0 )
    , m_iNumUnknown1( 0 )
    , field21_0x100( 0 )
    , field6_0x18( 1 )
{
}

AMiniGame::~AMiniGame()
{
}

const AMiniGame::Variant& AMiniGame::GetVariant( TINT a_iIndex )
{
	TASSERT( m_iNumVariants < a_iIndex );
	return m_aVariants[ a_iIndex ];
}

AMiniGame& AMiniGame::AddVariant( TINT a_eLocaleTitle, TINT a_eLocaleHint, TINT a_eLocaleDescription )
{
	TASSERT( m_iNumVariants < MAX_NUM_VARIANTS );

	Variant& rVariant           = m_aVariants[ m_iNumVariants++ ];
	rVariant.eLocaleTitle       = a_eLocaleTitle;
	rVariant.eLocaleHint        = a_eLocaleHint;
	rVariant.eLocaleDescription = a_eLocaleDescription;

	m_aUnknown[ m_iNumUnknown1 ] = 0;
	m_iNumUnknown1++;
	m_iNumSelectableVariants++;

	return *this;
}
