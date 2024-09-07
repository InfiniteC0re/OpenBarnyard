#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

class AOptionsState :
    public ABYardMenuState
{
public:
	TDECLARE_CLASS( AOptionsState, ABYardMenuState );

	enum OPTIONSBUTTON
	{
		OPTIONSBUTTON_AUDIO,
		OPTIONSBUTTON_CONTROLS,
		OPTIONSBUTTON_NUMOF
	};

	inline constexpr static TINT OPTIONSBUTTON_TEXTS[ OPTIONSBUTTON_NUMOF ] = {
		282,
		281
	};

public:
	AOptionsState();
	~AOptionsState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem& a_rMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	AGUI2Button m_aButtons[ OPTIONSBUTTON_NUMOF ];
	TINT        m_iActivatedButtonID;
};
