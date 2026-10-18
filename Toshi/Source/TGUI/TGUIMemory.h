#pragma once
#include <Toshi/TPile.h>

TOSHI_NAMESPACE_START

// Allocates memory for the GUI objects (from TGUIMemory::ms_oMemoryPile when it's enabled)
void* TGUINew( TSIZE a_uiSize );

// Frees memory allocated by TGUINew (memory of the pile isn't freed)
void TGUIDelete( void* a_pMem );

// Enables or disables allocating from the memory pile
void TGUISetUseMemoryPile( TBOOL a_bEnable );

//-----------------------------------------------------------------------------
// Purpose: base of the GUI objects that are allocated with TGUINew
// (styles, widgets, components and elements).
//-----------------------------------------------------------------------------
class TGUIMemory
{
public:
	// NOTE: JPOG has a copy of these operators in each derived class
	static void* operator new( TSIZE a_uiSize ) { return TGUINew( a_uiSize ); }
	static void* operator new[]( TSIZE a_uiSize ) { return TGUINew( a_uiSize ); }
	static void* operator new( TSIZE a_uiSize, void* a_pMem ) { return a_pMem; }

	static void operator delete( void* a_pMem ) { TGUIDelete( a_pMem ); }
	static void operator delete[]( void* a_pMem ) { TGUIDelete( a_pMem ); }
	static void operator delete( void* a_pMem, void* a_pPlace ) {}

	// $JPOG: FUNCTION TGuiInterface.dll+00008560
	TGUIMemory& operator=( const TGUIMemory& a_rcOther ) { return *this; }

public:
	static TBOOL ms_bAllocUsePile;
	static TPile ms_oMemoryPile;
};

TOSHI_NAMESPACE_END
