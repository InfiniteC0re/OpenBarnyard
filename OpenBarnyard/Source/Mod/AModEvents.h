#pragma once
#include <Mod/ModAPI.h>

TOSHI_NAMESPACE_START

class TRenderInterface;

TOSHI_NAMESPACE_END

class ARenderer;
class AGUI2;
class ACowTippingMiniGameState;

//-----------------------------------------------------------------------------
// Events the game sends to mods, see AModEventList.h for the list.
//
//   static void OnLoadScreen( const ModEvent::ALoadScreen::StateChanged& a_rEvent, void* a_pUserData ) { ... }
//
//   SUBSCRIBE_EVENT( ALoadScreen::StateChanged, OnLoadScreen );
//
// The game fires them with FIRE_EVENT( ALoadScreen::StateChanged, TTRUE ), which
// costs a single array lookup when nobody listens.
//-----------------------------------------------------------------------------

// Index of every event
using AModEventId = TINT;
enum AModEventId_ : AModEventId
{
#define DECLARE_EVENT( CLASS, NAME, ... ) AModEventId_##CLASS##_##NAME,
#include "AModEventList.h"
#undef DECLARE_EVENT

	AModEventId_NUMOF
};

// ModEvent::Class::Name structs
#define DECLARE_EVENT( CLASS, NAME, ... )                                     \
	namespace ModEvent                                                        \
	{                                                                         \
	namespace CLASS                                                           \
	{                                                                         \
	struct NAME                                                               \
	{                                                                         \
		static constexpr AModEventId  ID = AModEventId_##CLASS##_##NAME;      \
		static constexpr const TCHAR* GetName() { return #CLASS "::" #NAME; } \
                                                                              \
		__VA_ARGS__                                                           \
	};                                                                        \
	}                                                                         \
	}
#include "AModEventList.h"
#undef DECLARE_EVENT

#ifdef BARNYARD_COMMUNITY_PATCH
#  define FIRE_EVENT( EVENT, ... ) AModEvents::Fire( ModEvent::EVENT{ __VA_ARGS__ } )
#else
#  define FIRE_EVENT( EVENT, ... ) \
	  do {                         \
	  } while ( TFALSE )
#endif

#define SUBSCRIBE_EVENT( EVENT, ... )   AModEvents::Subscribe<ModEvent::EVENT>( __VA_ARGS__ )
#define UNSUBSCRIBE_EVENT( EVENT, ... ) AModEvents::Unsubscribe<ModEvent::EVENT>( __VA_ARGS__ )

class MODAPI AModEvents
{
public:
	template <class EVENT>
	using t_Callback = void ( * )( const EVENT& a_rEvent, void* a_pUserData );

	static constexpr TINT MAX_NUM_LISTENERS = 32;

public:
	// Starts calling a_fnCallback every time the event is fired
	template <class EVENT>
	static TBOOL Subscribe( t_Callback<EVENT> a_fnCallback, void* a_pUserData = TNULL )
	{
		return Subscribe( EVENT::ID, TREINTERPRETCAST( t_GenericCallback, a_fnCallback ), a_pUserData );
	}

	// Stops calling a_fnCallback. Safe to call from inside the callback.
	template <class EVENT>
	static TBOOL Unsubscribe( t_Callback<EVENT> a_fnCallback, void* a_pUserData = TNULL )
	{
		return Unsubscribe( EVENT::ID, TREINTERPRETCAST( t_GenericCallback, a_fnCallback ), a_pUserData );
	}

	// Calls every listener of the event
	template <class EVENT>
	static void Fire( const EVENT& a_rEvent )
	{
		if ( ms_aEvents[ EVENT::ID ].iNumListeners != 0 )
			Fire( EVENT::ID, &a_rEvent );
	}

	// Removes every listener, called when the mod loader is destroyed
	static void Reset();

private:
	using t_GenericCallback = void ( * )( const void* a_pEvent, void* a_pUserData );

	struct Listener
	{
		t_GenericCallback fnCallback;
		void*             pUserData;
	};

	struct Event
	{
		Listener aListeners[ MAX_NUM_LISTENERS ];
		TINT     iNumListeners;
		TINT     iFireDepth;
		TBOOL    bHasRemovedListeners;
	};

	static TBOOL Subscribe( AModEventId a_eEvent, t_GenericCallback a_fnCallback, void* a_pUserData );
	static TBOOL Unsubscribe( AModEventId a_eEvent, t_GenericCallback a_fnCallback, void* a_pUserData );
	static void  Fire( AModEventId a_eEvent, const void* a_pEvent );
	static void  RemoveDeadListeners( Event& a_rEvent );

private:
	static Event ms_aEvents[ AModEventId_NUMOF ];
};
