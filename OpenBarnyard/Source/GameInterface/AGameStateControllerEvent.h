#pragma once
#include "AGameState.h"

#include <Toshi/Typedefs.h>
#include <Toshi/Core/TGlobalEmitter.h>

class AGameStateControllerEvent :
	public Toshi::TGlobalEvent<AGameStateControllerEvent>
{
public:
	using Type = TUINT32;
	enum Type_ : Type
	{
		Type_Unknown,
		Type_GameStateAdded,
		Type_GameStateSuspended,
		Type_GameStateBeingAdded,
	};

public:
	constexpr AGameStateControllerEvent(AGameState* a_pGameState, Type a_eType) :
		m_pGameState(a_pGameState), 
		m_eType(a_eType) { }

	AGameState* GetGameState() const { return m_pGameState; }
	Type GetEventType() const { return m_eType; }

private:
	AGameState* m_pGameState;
	Type m_eType;
};
