#pragma once
#include "AGameState.h"
#include "THookedSingleton.h"
#include "SDK_T2GUIRectangle.h"

#include <Toshi/TTask.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AGameStateController : public Toshi::TTask
    , public THookedSingleton<AGameStateController, 0x00781788>
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	void ReplaceState( AGameState* a_pGameState );
	void PushState( AGameState* a_pGameState );
	void PopState( AGameState* a_pGameState, void* a_pUnused );

	void ResetStack();

	void PopCurrentGameState();
	void UpdateScreenOverlay();

	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

	AGameState* GetCurrentState()
	{
		return *m_oStates.Back();
	}

	AGameState* GetPreviousState()
	{
		return m_oStates[ m_oStates.Size() - 2 ];
	}

	TINT GetNumStates() const
	{
		return m_oStates.Size();
	}

	TBOOL IsCurrentState( Toshi::TClass* a_pClass )
	{
		return GetCurrentState()->IsA( a_pClass );
	}

private:
	void InsertGameState( AGameState* a_pGameState );

public:
	TUINT                            m_Unk1;
	Toshi::T2Vector<AGameState*, 10> m_oStates;
	void*                            m_VectorData[ 5 ];
	Toshi::T2DynamicVector<void*>    m_UnkVector;
	TUINT                            m_Unk2;
	TUINT                            m_Unk3;
	SDK_T2GUIRectangle               m_oOverlay;
	TUINT16                          m_eFlags;
	TFLOAT                           m_flOverlayAlpha;
	TFLOAT                           m_flSoundVolume;
	void*                            m_Unk4;
};
