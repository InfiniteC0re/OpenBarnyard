#pragma once
#include "GUI/AGUI2Element.h"
#include "Input/AInputMapManager.h"
#include "AGameStateControllerEvent.h"

#include <Toshi2/T2DList.h>
#include <Toshi/Input/TInputInterface.h>

class AGameState :
	public Toshi::TGenericClassDerived<AGameState, Toshi::TObject, "AGameState", TMAKEVERSION(1, 0), TTRUE>,
	public Toshi::T2DList<AGameState>::Node
{
public:
	template <class Result, class... Args>
	using t_ExecuteForChildCb = Result(AGameState::*)(Args... args);

	struct HUDParams
	{
		TUINT m_uiFlags = 0;
		TUINT m_Unk1 = 0;
		TUINT m_Unk2 = 0;
		TUINT m_Unk3 = 0;
		TUINT m_Unk4 = 0;
		TUINT m_Unk5 = 0;
		TUINT m_Unk6 = 0;
		TUINT m_Unk7 = 0;
		TUINT m_Unk8 = 0;
		TUINT m_Unk9 = 0;

		void SetFlags(TUINT a_uiFlags)
		{
			if (a_uiFlags != m_uiFlags)
			{
				m_uiFlags = a_uiFlags;
			}
		}
	};

public:
	AGameState();
	virtual ~AGameState() = default;

	virtual TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent);
	virtual TBOOL ProcessCommand(AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents);
	virtual TBOOL Unknown1(void* a_pUnk1, void* a_pUnk2);
	virtual void Unknown2(void* a_pUnk1) { }
	virtual void Unknown3(void* a_pUnk1) { }
	virtual void Unknown4(void* a_pUnk1) { }
	virtual void Unknown5() { }
	virtual TBOOL Unknown6() { return TTRUE; }
	virtual TUINT32 GetSound() { return -1; }
	virtual TBOOL Unknown7() { return TTRUE; }
	virtual void Unknown8() { }
	virtual void OnStarted() { }
	virtual void Unknown10() { }
	virtual void Unknown11(void* a_pUnk1, void* a_pUnk2) { }
	virtual void Unknown12(void* a_pUnk1, void* a_pUnk2) { }
	virtual TFLOAT GetFOV();
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime);
	virtual void OnInsertion();
	virtual void OnRemoval();
	virtual void OnSuspend();
	virtual void OnResume(AGameState* a_pPreviousState);
	virtual void OnActivate();
	virtual void OnDeactivate();

	void Destroy();
	void Destroy(TBOOL a_bDeactivate);

	TBOOL SendInputCommands(const Toshi::TInputInterface::InputEvent* a_pEvent);

	void SetInputMap(const Toshi::TPString8& a_MapName);

	void Activate()
	{
		OnActivate();

		Toshi::TGlobalEmitter<AGameStateControllerEvent>::Throw(
			AGameStateControllerEvent(this, AGameStateControllerEvent::Type_GameStateActivated)
		);
	}

	void Deactivate()
	{
		OnDeactivate();

		Toshi::TGlobalEmitter<AGameStateControllerEvent>::Throw(
			AGameStateControllerEvent(
				this,
				AGameStateControllerEvent::Type_GameStateDeactivated
			)
		);
	}

	void Suspend()
	{
		OnSuspend();
	}

	void Remove();

	template <class... Args>
	void ExecuteForAllChildStates(t_ExecuteForChildCb<void, Args...> a_fnCallback, TUINT32 a_uiOffset, Args... args)
	{
		for (auto it = m_ChildStates.Begin(); it != m_ChildStates.End(); it++)
		{
			auto pGameState = TREINTERPRETCAST(
				AGameState*,
				TREINTERPRETCAST(uintptr_t, TSTATICCAST(AGameState*, it)) + a_uiOffset
			);

			(pGameState->*a_fnCallback)(args...);
		}
	}

	template <class... Args>
	TBOOL ExecuteForOneChildState(t_ExecuteForChildCb<TBOOL, Args...> a_fnCallback, TUINT32 a_uiOffset, Args... args)
	{
		for (auto it = m_ChildStates.Begin(); it != m_ChildStates.End(); it++)
		{
			auto pGameState = TREINTERPRETCAST(
				AGameState*,
				TREINTERPRETCAST(uintptr_t, TSTATICCAST(AGameState*, it)) + a_uiOffset
			);

			TBOOL bResult = (pGameState->*a_fnCallback)(args...);

			if (bResult)
			{
				return TTRUE;
			}
		}

		return TFALSE;
	}

protected:
	TBOOL ChildState_ProcessInput(const Toshi::TInputInterface::InputEvent* a_pInputEvent)
	{
		return ProcessInput(a_pInputEvent);
	}

	TBOOL ChildState_Unknown1(void* a_pUnk1, void* a_pUnk2)
	{
		return Unknown1(a_pUnk1, a_pUnk2);
	}

	void ChildState_OnUpdate(TFLOAT a_fDeltaTime)
	{
		OnUpdate(a_fDeltaTime);
	}

	void ChildState_OnInsertion()
	{
		OnInsertion();
	}

	void ChildState_OnRemoval()
	{
		Remove();
	}

	void ChildState_OnSuspend()
	{
		OnSuspend();
	}

	void ChildState_OnResume(AGameState* a_pPreviousState)
	{
		OnResume(a_pPreviousState);
	}

	void ChildState_OnActivate()
	{
		OnActivate();
	}

	void ChildState_OnDeactivate()
	{
		OnDeactivate();
	}

protected:
	HUDParams m_HUDParams;
	TINT m_iOverlayColorIndex;
	Toshi::T2DList<AGameState> m_ChildStates;
	AInputMap* m_pInputMap;
	AGameState* m_pOwnerState;
	TBOOL m_bWasInserted;
	TBOOL m_bIsActivated;
	AGUI2Element m_GUIElement;
	TFLOAT m_fFOV;
};