#include "pch.h"
#include "AModHooks.h"

#include <MinHook.h>
#include <intrin.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TBOOL s_bMinHookInitialised = TFALSE;

static HMODULE GetModuleFromAddress( const void* a_pAddress )
{
	HMODULE hModule = TNULL;
	GetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)a_pAddress, &hModule );

	return hModule;
}

void* AModHooks::Install( void* a_pTarget, void* a_pDetour )
{
	if ( !s_bMinHookInitialised )
	{
		if ( MH_Initialize() != MH_OK )
		{
			TERROR( "AModHooks: Couldn't initialise MinHook\n" );
			return TNULL;
		}

		s_bMinHookInitialised = TTRUE;
	}

	// A target inside the module that installs the hook is almost always the vcall thunk of a virtual function
	if ( GetModuleFromAddress( a_pTarget ) == GetModuleFromAddress( _ReturnAddress() ) )
	{
		TERROR( "AModHooks: Target %p is inside the mod itself, virtual functions can't be hooked\n", a_pTarget );
		return TNULL;
	}

	void*     pOriginal = TNULL;
	MH_STATUS eStatus   = MH_CreateHook( a_pTarget, a_pDetour, &pOriginal );

	if ( eStatus == MH_OK )
		eStatus = MH_EnableHook( a_pTarget );

	if ( eStatus != MH_OK )
	{
		TERROR( "AModHooks: Couldn't hook %p (%s)\n", a_pTarget, MH_StatusToString( eStatus ) );
		MH_RemoveHook( a_pTarget );
		return TNULL;
	}

	return pOriginal;
}

TBOOL AModHooks::Remove( void* a_pTarget )
{
	if ( !s_bMinHookInitialised ) return TFALSE;

	return MH_DisableHook( a_pTarget ) == MH_OK && MH_RemoveHook( a_pTarget ) == MH_OK;
}
