#include "ToshiPCH.h"
#include "TGUIMemory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL TGUIMemory::ms_bAllocUsePile = TTRUE;

// $JPOG: FUNCTION TGuiInterface.dll+000127f0
// $JPOG: FUNCTION TGuiInterface.dll+000127e0
TPile TGUIMemory::ms_oMemoryPile = TPile( 0x8000, 0x100000, 0 );

// $JPOG: FUNCTION TGuiInterface.dll+000127b0
void* TGUINew( TSIZE a_uiSize )
{
	if ( TGUIMemory::ms_bAllocUsePile )
		return TGUIMemory::ms_oMemoryPile.RawAlloc( TINT( a_uiSize ), 4 );

	return TMalloc( a_uiSize );
}

// $JPOG: FUNCTION TGuiInterface.dll+00012820
void TGUIDelete( void* a_pMem )
{
	if ( !TGUIMemory::ms_oMemoryPile.RawIsInside( a_pMem ) )
		TFree( a_pMem );
}

// $JPOG: FUNCTION TGuiInterface.dll+00012850
void TGUISetUseMemoryPile( TBOOL a_bEnable )
{
	TGUIMemory::ms_bAllocUsePile = a_bEnable;
}

TOSHI_NAMESPACE_END
