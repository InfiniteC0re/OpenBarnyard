#pragma once
#include "AGameState.h"
#include "AGameStateControllerEvent.h"
#include "GUI/T2GUIRectangle.h"

#include <Toshi/TTask.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AGameStateController
	: public Toshi::TTask
    , public Toshi::TSingleton<AGameStateController>
{
public:
	TDECLARE_CLASS( AGameStateController, Toshi::TTask );

	struct OverlayData
	{
		TUINT8 uiColorA;
		TUINT8 uiColorR;
		TUINT8 uiColorG;
		TUINT8 uiColorB;
	};

public:
	AGameStateController();
	~AGameStateController();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	void ReplaceState( AGameState* a_pGameState );
	void PushState( AGameState* a_pGameState );
	void PopState( AGameState* a_pGameState );

	void PopCurrentGameState();
	void ResetStack();

	void SetFlags( TUINT16 a_eFlags );

	void         SetOverlayParams( AGameState::OVERLAY a_eOverlay, const OverlayData& a_rcParams );
	OverlayData* GetOverlayParams( AGameState::OVERLAY a_eOverlay );
	void         UpdateScreenOverlay();

	TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

	AGameState* GetCurrentState() { return *m_oStates.Back(); }
	TBOOL       IsCurrentState( Toshi::TClass* a_pClass ) { return GetCurrentState()->IsA( a_pClass ); }

private:
	void InsertGameState( AGameState* a_pGameState );

private:
	inline static OverlayData ms_aOverlays[ AGameState::OVERLAY_NUMOF ];

private:
	// ...
	Toshi::T2Vector<AGameState*, 10> m_oStates;
	void*                            m_VectorData[ 5 ];
	Toshi::T2DynamicVector<void*>    m_UnkVector;
	// ...
	Toshi::T2GUIRectangle m_oOverlay;
	TUINT16               m_eFlags;
	TFLOAT                m_fOverlayAlpha;
	TFLOAT                m_fSoundVolume;
	TFLOAT                m_fOverlayGoal;
};
