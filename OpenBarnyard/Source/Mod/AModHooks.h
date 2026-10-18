#pragma once
#include <Mod/ModAPI.h>

#include <cstring>

//-----------------------------------------------------------------------------
// Function hooks for mods. The game owns MinHook, mods only call these.
//
// Hooks are declared with the macros below and point at real game functions:
//
//   MEMBER_HOOK( &APlayerProfileManager::UpdateMainProfile, APlayerProfileManager, APlayerProfileManager_UpdateMainProfile, TBOOL, TFLOAT a_fDeltaTime )
//   {
//       return CallOriginal( a_fDeltaTime );
//   }
//
//   InstallHook<APlayerProfileManager_UpdateMainProfile>();
//
// The target must be exported from the game (MODAPI on its class or itself),
// otherwise the mod fails to link.
//
// NOTE: virtual functions can't be hooked this way, &Class::Func of a virtual
// function points to a small thunk compiled into the mod, not to the game code.
//-----------------------------------------------------------------------------

namespace AModHooks
{

// Hooks a_pTarget so it calls a_pDetour instead.
// Returns the address to call the original function with or TNULL on failure.
MODAPI void* Install( void* a_pTarget, void* a_pDetour );

// Removes a hook installed by Install
MODAPI TBOOL Remove( void* a_pTarget );

// Returns the code address of a function or a member function pointer
template <class T>
inline void* GetFunctionAddress( T a_pFunction )
{
	// Member function pointers of classes with multiple inheritance are larger than
	// a code pointer, the code address is always stored first
	static_assert( sizeof( T ) >= sizeof( void* ) );

	void* pAddress;
	std::memcpy( &pAddress, &a_pFunction, sizeof( pAddress ) );

	return pAddress;
}

} // namespace AModHooks

template <class HOOK>
inline TBOOL InstallHook()
{
	HOOK::_orig_func_ptr = AModHooks::Install( HOOK::GetTarget(), HOOK::GetDetour() );
	return HOOK::_orig_func_ptr != TNULL;
}

template <class HOOK>
inline TBOOL RemoveHook()
{
	if ( !HOOK::_orig_func_ptr ) return TFALSE;

	if ( !AModHooks::Remove( HOOK::GetTarget() ) ) return TFALSE;

	HOOK::_orig_func_ptr = TNULL;
	return TTRUE;
}

// clang-format off
//-----------------------------------------------------------------------------
// Declares the hook object. CallOriginal jumps straight into the trampoline, so it
// works for any calling convention as long as the hook has the same signature.
//-----------------------------------------------------------------------------
#define _MODHOOK_DECLARE_COMMON()                 \
	static inline void* _orig_func_ptr = TNULL; \
	static void*        GetDetour() { return AModHooks::GetFunctionAddress( &_hook_obj::_hook_func ); }

#define _MODHOOK_DECLARE_MEMBER( TARGET, OBJECT, NAME, CONST, RETURN, ... )                                       \
	struct NAME                                                                                                   \
	{                                                                                                             \
		using _target_type = RETURN ( OBJECT::* )( __VA_ARGS__ ) CONST;                                           \
                                                                                                                  \
		struct _hook_obj : public OBJECT                                                                          \
		{                                                                                                         \
			RETURN CallOriginal( __VA_ARGS__ ) CONST;                                                             \
			RETURN _hook_func( __VA_ARGS__ ) CONST;                                                               \
		};                                                                                                        \
                                                                                                                  \
		static void* GetTarget() { return AModHooks::GetFunctionAddress( static_cast<_target_type>( TARGET ) ); } \
		_MODHOOK_DECLARE_COMMON()                                                                                 \
	};                                                                                                            \
                                                                                                                  \
	__declspec( naked ) RETURN NAME::_hook_obj::CallOriginal( __VA_ARGS__ ) CONST                                 \
	{                                                                                                             \
		__asm jmp DWORD PTR[ NAME::_orig_func_ptr ]                                                               \
	}                                                                                                             \
                                                                                                                  \
	RETURN NAME::_hook_obj::_hook_func( __VA_ARGS__ ) CONST

// Hooks a member function. TARGET is &Class::Function, the arguments must match its signature.
#define MEMBER_HOOK( TARGET, OBJECT, NAME, RETURN, ... ) \
	_MODHOOK_DECLARE_MEMBER( TARGET, OBJECT, NAME, , RETURN, __VA_ARGS__ )

// Hooks a const member function
#define CONST_MEMBER_HOOK( TARGET, OBJECT, NAME, RETURN, ... ) \
	_MODHOOK_DECLARE_MEMBER( TARGET, OBJECT, NAME, const, RETURN, __VA_ARGS__ )

// Hooks a free function or a static member function (__cdecl)
#define HOOK( TARGET, NAME, RETURN, ... )                                                                         \
	struct NAME                                                                                                   \
	{                                                                                                             \
		using _target_type = RETURN ( * )( __VA_ARGS__ );                                                         \
                                                                                                                  \
		struct _hook_obj                                                                                          \
		{                                                                                                         \
			static RETURN CallOriginal( __VA_ARGS__ );                                                            \
			static RETURN _hook_func( __VA_ARGS__ );                                                              \
		};                                                                                                        \
                                                                                                                  \
		static void* GetTarget() { return AModHooks::GetFunctionAddress( static_cast<_target_type>( TARGET ) ); } \
		_MODHOOK_DECLARE_COMMON()                                                                                 \
	};                                                                                                            \
                                                                                                                  \
	__declspec( naked ) RETURN NAME::_hook_obj::CallOriginal( __VA_ARGS__ )                                       \
	{                                                                                                             \
		__asm jmp DWORD PTR[ NAME::_orig_func_ptr ]                                                               \
	}                                                                                                             \
                                                                                                                  \
	RETURN NAME::_hook_obj::_hook_func( __VA_ARGS__ )
// clang-format on
