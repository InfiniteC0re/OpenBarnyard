#pragma once
#include "AGameState.h"
#include "AHookedSingleton.h"
#include "AGUI2Rectangle.h"

#include <Toshi/Core/TTask.h>
#include <Toshi2/T2Vector.h>
#include <Toshi2/T2DynamicVector.h>

TOBJECT(AGameStateController, Toshi::TTask, TFALSE),
	public AHookedSingleton<AGameStateController, 0x00781788>
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(TFLOAT a_fDeltaTime) override;
	virtual void OnDestroy() override;

	void ReplaceState(AGameState* a_pGameState);
	void PushState(AGameState* a_pGameState);
	void PopState(AGameState* a_pGameState, void* a_pUnused);

	void ResetStack();

	void PopCurrentGameState();
	void UpdateScreenOverlay();

	TBOOL ProcessInput(const Toshi::TInputInterface::InputEvent* a_pEvent);
	
	AGameState* GetCurrentState()
	{
		return m_oStates.Back();
	}

	TBOOL IsCurrentState(Toshi::TClass* a_pClass)
	{
		return GetCurrentState()->IsA(a_pClass);
	}

private:
	void InsertGameState(AGameState* a_pGameState);

private:
	TUINT m_Unk1;
	Toshi::T2Vector<AGameState*, 10> m_oStates;
	void* m_VectorData[5];
	Toshi::T2DynamicVector<void*> m_UnkVector;
	TUINT m_Unk2;
	TUINT m_Unk3;
	AGUI2Rectangle m_Rectangle;
	TUINT16 m_eFlags;
	TFLOAT m_fOverlayAlpha;
	TFLOAT m_fSoundVolume;
	void* m_Unk4;
};
