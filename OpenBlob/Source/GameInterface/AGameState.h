#pragma once
#include "Input/AInputHelperSimple.h"
#include "Toshi2/T2DList.h"

class AGameState :
	public Toshi::TGenericClassDerived<AGameState, Toshi::TObject, "AGameState", TMAKEVERSION(1, 0), true>,
	public Toshi::T2DList<AGameState>::Node
{
public:
	AGameState();
	virtual ~AGameState();

	virtual void Unk1();
	virtual bool Unk2();
	virtual void Unk3(void*, void*);
	virtual int OnUpdate(float deltaTime);
	virtual void OnInsertion();
	virtual void OnRemoval();
	virtual void OnSuspend();
	virtual void OnResume(void* unk);
	virtual void OnActivate();
	virtual void OnDeactivate();
	virtual bool CanActivateConsoleState();
	virtual bool CheckForControllerRemoval();
	virtual bool PauseOnControllerReinserted();

	bool HandleMenuRequestToChangeState(uint32_t menuType)
	{

	}

private:
	Toshi::T2DList<AGameState> m_SomeList;
	AInputHelperSimple m_InputHelper;
};