#pragma once
#include <Toshi/Typedefs.h>
#include <Toshi/TGlobalEmitter.h>

class AGameState;

class AGameStateControllerEvent :
	public Toshi::TGlobalEvent<AGameStateControllerEvent>
{
public:
	using Type = TUINT32;
	enum Type_ : Type
	{
		Type_Unknown,
		Type_GameStateActivated,
		Type_GameStateDeactivated,
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
