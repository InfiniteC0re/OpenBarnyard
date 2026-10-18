//-----------------------------------------------------------------------------
// Compiled into every mod DLL (see Mods/ModProject.lua), never into the game.
//
// The game replaces the global new/delete with the Toshi allocator, but that only
// applies to the module it's linked into. Without this file a mod would allocate
// from the CRT heap and crash when the game frees its objects, or the other way.
//-----------------------------------------------------------------------------
#if !defined( MODAPI_MOD )
#  error "ModMemory.cpp is only meant to be compiled into mods"
#endif

#include <ToshiPCH.h>
#include <Core/TMemory.h>

#include <new>

void* __CRTDECL operator new( TSIZE a_uiSize )
{
	return TMalloc( a_uiSize, TNULL, TNULL, -1 );
}

void* __CRTDECL operator new( TSIZE a_uiSize, ::std::nothrow_t const& ) noexcept
{
	return TMalloc( a_uiSize, TNULL, TNULL, -1 );
}

void* __CRTDECL operator new[]( TSIZE a_uiSize )
{
	return TMalloc( a_uiSize, TNULL, TNULL, -1 );
}

void* __CRTDECL operator new[]( TSIZE a_uiSize, ::std::nothrow_t const& ) noexcept
{
	return TMalloc( a_uiSize, TNULL, TNULL, -1 );
}

void __CRTDECL operator delete( void* a_pMem ) noexcept
{
	TFree( a_pMem );
}

void __CRTDECL operator delete( void* a_pMem, ::std::nothrow_t const& ) noexcept
{
	TFree( a_pMem );
}

void __CRTDECL operator delete( void* a_pMem, TSIZE ) noexcept
{
	TFree( a_pMem );
}

void __CRTDECL operator delete[]( void* a_pMem ) noexcept
{
	TFree( a_pMem );
}

void __CRTDECL operator delete[]( void* a_pMem, ::std::nothrow_t const& ) noexcept
{
	TFree( a_pMem );
}

void __CRTDECL operator delete[]( void* a_pMem, TSIZE ) noexcept
{
	TFree( a_pMem );
}
