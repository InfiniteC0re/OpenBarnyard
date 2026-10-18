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

// Follows jmp rel32 instructions, which incremental linking puts in front of every function
static const TBYTE* SkipJumps( const TBYTE* a_pCode )
{
	while ( *a_pCode == 0xE9 )
		a_pCode = a_pCode + 5 + *(const TINT32*)( a_pCode + 1 );

	return a_pCode;
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
		TERROR( "AModHooks: Target %p is inside the mod itself, is it a virtual function? Pass an instance to InstallHook\n", a_pTarget );
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

void* AModHooks::ResolveVirtual( const void* a_pInstance, void* a_pVirtualThunk )
{
	TASSERT( a_pInstance != TNULL );

	// MSVC x86 vcall thunk: mov eax, [ecx]; jmp [eax + offset]
	const TBYTE* pCode = SkipJumps( (const TBYTE*)a_pVirtualThunk );
	TUINT32      uiOffset;

	if ( pCode[ 0 ] != 0x8B || pCode[ 1 ] != 0x01 || pCode[ 2 ] != 0xFF )
	{
		TERROR( "AModHooks: %p is not a virtual function thunk\n", a_pVirtualThunk );
		return TNULL;
	}

	if ( pCode[ 3 ] == 0x20 )
		uiOffset = 0; // jmp [eax]
	else if ( pCode[ 3 ] == 0x60 )
		uiOffset = pCode[ 4 ]; // jmp [eax + imm8]
	else if ( pCode[ 3 ] == 0xA0 )
		uiOffset = *(const TUINT32*)( pCode + 4 ); // jmp [eax + imm32]
	else
	{
		TERROR( "AModHooks: %p is not a virtual function thunk\n", a_pVirtualThunk );
		return TNULL;
	}

	void* const* pVTable = *(void* const* const*)a_pInstance;
	return pVTable[ uiOffset / sizeof( void* ) ];
}
