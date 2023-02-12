#pragma once
#include "Input/AInputHelperSimple.h"
#include "Toshi2/T2DList.h"
#include <Toshi2/T2GUI/T2GUIRectangle.h>

class AGameState :
	public Toshi::TGenericClassDerived<AGameState, Toshi::TObject, "AGameState", TMAKEVERSION(1, 0), true>,
	public Toshi::T2DList<AGameState>::Node
{
public:
	using State = uint8_t;
	enum State_ : State
	{
		State_Null = 0,
		State_Activated = BITFIELD(0),
		State_Inserted = BITFIELD(1),
		State_Suspended = BITFIELD(2),
		State_Unk4 = BITFIELD(3),
	};

	using UpdateResult = uint32_t;
	enum UpdateResult_ : UpdateResult
	{
		UpdateResult_OK,
		UpdateResult_Remove
	};

public:
	AGameState();
	virtual ~AGameState();

	virtual void Unk1();
	virtual bool Unk2();
	virtual void Unk3(void*, void*);
	virtual UpdateResult OnUpdate(float deltaTime);
	virtual void OnInsertion();
	virtual void OnRemoval();
	virtual void OnSuspend();
	virtual void OnResume(AGameState* pOldState);
	virtual void OnActivate();
	virtual void OnDeactivate();
	virtual bool CanActivateConsoleState();
	virtual bool CheckForControllerRemoval();
	virtual bool PauseOnControllerReinserted();

	static void SetupLoadIcon();

	State GetState() const
	{
		return m_State;
	}

	void SetState(State state)
	{
		m_State = state;
	}

	void AddState(State state)
	{
		m_State |= state;
	}

	void RemoveState(State state)
	{
		m_State &= ~state;
	}

	void Activate()
	{
		if (!HASFLAG(GetState() & AGameState::State_Activated))
		{
			OnActivate();
			AddState(AGameState::State_Activated);
		}
	}

	void Deactivate()
	{
		if (HASFLAG(GetState() & AGameState::State_Activated))
		{
			OnDeactivate();
			RemoveState(AGameState::State_Activated);
		}
	}

	void Insert()
	{
		if (!HASFLAG(GetState() & AGameState::State_Inserted))
		{
			OnInsertion();
			AddState(AGameState::State_Inserted);
		}
	}

	void Remove()
	{
		if (HASFLAG(GetState() & AGameState::State_Inserted))
		{
			OnRemoval();
			RemoveState(AGameState::State_Inserted);
		}
	}

	void Suspend()
	{
		if (!HASFLAG(GetState() & AGameState::State_Suspended))
		{
			OnSuspend();
			AddState(AGameState::State_Suspended);
		}
	}

	void Resume(AGameState* pOldState)
	{
		if (HASFLAG(GetState() & AGameState::State_Suspended))
		{
			OnResume(pOldState);
			RemoveState(AGameState::State_Suspended);
		}
	}

	void RemoveSelf();

	bool HandleMenuRequestToChangeState(uint32_t menuType)
	{

	}


	static inline Toshi::T2GUIRectangle* sm_pLoadIconRect = TNULL;
	static inline Toshi::T2GUIMaterial* sm_pLoadIconMat = TNULL;

private:
	Toshi::T2DList<AGameState> m_GameStates; // 0x0C
	AInputHelperSimple m_InputHelper;        // 0x14
	State m_State;                           // 0x28
};