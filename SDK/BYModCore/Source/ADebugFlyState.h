#pragma once
#include <BYardSDK/AGameState.h>
#include <BYardSDK/AGUI2Font.h>
#include <BYardSDK/AGUI2TextBox.h>
#include <Input/TInputInterface.h>

class ACameraHelper;

class ADebugFlyState : public AGameState
{
public:
	TDECLARE_CLASS( ADebugFlyState, AGameState );

public:
	ADebugFlyState();
	~ADebugFlyState();

	virtual TBOOL  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void   OnActivate() OVERRIDE;
	virtual void   OnRemoval() OVERRIDE;
	virtual TBOOL  ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TFLOAT GetFOV() OVERRIDE;

private:
	ACameraHelper*     m_pOldHelper = TNULL;
	Toshi::TVector4    m_vecLookDirection;
	Toshi::TQuaternion m_vecRotation;
	TFLOAT             m_fRoll  = 0.0f;
	TFLOAT             m_fYaw   = 0.0f;
	TFLOAT             m_fPitch = 0.0f;
	TFLOAT             m_fSpeed = 10.0f;
	TFLOAT             m_aUpAxisInput[ 2 ];
	TFLOAT             m_aForwardAxisInput[ 2 ];
	TFLOAT             m_aRightAxisInput[ 2 ];
	AGUI2TextBox*      m_pInfoTextBox;
};
