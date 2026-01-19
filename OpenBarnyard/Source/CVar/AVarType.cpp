#include "pch.h"
#include "AVarType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006c1a20
AVarType::AVarType( const TCHAR* a_pchName )
    : pchName( a_pchName )
    , pNext( TNULL )
{
	if ( ms_pHead == TNULL )
	{
		ms_pHead = this;
		ms_pTail = this;
	}
	else
	{
		ms_pTail->pNext = this;
		ms_pTail        = this;
	}
}

AVarType::~AVarType()
{
}

static BOOL s_bInitialised;

// $Barnyard: FUNCTION 006c1a60
void AVarType::Create( Toshi::TPString8Pool* a_pStringPool )
{
	TASSERT( s_bInitialised != TTRUE );
	s_bInitialised = TTRUE;

	for ( AVarType* it = ms_pHead; it != TNULL; it = it->pNext )
	{
		// Get string
		TPooledString8* pStrPooled;
		a_pStringPool->Get( pStrPooled, it->pchName );

		// Save it
		it->strPooled = pStrPooled;
	}
}

// $Barnyard: FUNCTION 006c19e0
void AVarType::Destroy()
{
	TASSERT( s_bInitialised != TFALSE );
	s_bInitialised = TFALSE;

	// Remove all references
	for ( AVarType* it = ms_pHead; it != TNULL; it = it->pNext )
		it->strPooled.SetPooledString( TNULL );
}
