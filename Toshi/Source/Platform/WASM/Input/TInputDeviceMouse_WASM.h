#pragma once
#include "Input/TInputDeviceMouse.h"

TOSHI_NAMESPACE_START

class TInputWASMDeviceMouse : public TInputDeviceMouse
{
public:
	TDECLARE_CLASS( TInputWASMDeviceMouse, TInputDeviceMouse );

public:
	TInputWASMDeviceMouse();
	~TInputWASMDeviceMouse();

	virtual TBOOL  Acquire() override;
	virtual TBOOL  Unacquire() override;
	virtual void   Release() override;
	virtual void   Update( TFLOAT a_fDeltaTime ) override;
	virtual TBOOL  Flush() override;
	virtual TINT   ProcessEvents( EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime ) override;
	virtual TINT   GetButtonCount() const override { return 3; }
	virtual TINT   GetAxisCount() const override { return 3; }
	virtual TBOOL  IsDown( Doodad a_iDoodad ) const override;
	virtual TINT   GetAxisInt( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TFLOAT GetAxisFloat( Doodad a_iDoodad, TINT a_iAxis ) const override;
	virtual TBOOL  IsEnabled() const override { return TTRUE; }

	void AddMovement( TINT a_iX, TINT a_iY ) { m_iPendingX += a_iX; m_iPendingY += a_iY; }
	void AddWheel( TINT a_iSteps ) { m_iPendingSteps += a_iSteps; }
	void SetButton( TINT a_iButton, TBOOL a_bDown );

private:
	TUINT32 m_uiPendingButtons;
	TUINT32 m_uiButtons;
	TINT    m_iPendingX;
	TINT    m_iPendingY;
	TINT    m_iPendingSteps;
	TINT    m_iDeltaX;
	TINT    m_iDeltaY;
	TINT    m_iDeltaSteps;
};

TOSHI_NAMESPACE_END
