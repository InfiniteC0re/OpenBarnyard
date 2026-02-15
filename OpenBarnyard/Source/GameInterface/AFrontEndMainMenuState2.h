#pragma once
#include "ABYardMenuState.h"
#include "Helpers/AWindmillHelper.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AFade.h"

class AFrontEndMainMenuState2 : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndMainMenuState2, ABYardMenuState );

	enum FRONTENDBUTTON
	{
		FRONTENDBUTTON_NEWGAME,
		FRONTENDBUTTON_LOADGAME,
		FRONTENDBUTTON_OPTIONS,
		FRONTENDBUTTON_BONUSES,
		FRONTENDBUTTON_ANTICS,
		FRONTENDBUTTON_QUIT,
		FRONTENDBUTTON_NUMOF
	};

	inline constexpr static TINT FRONTENDBUTTON_TEXTS[ FRONTENDBUTTON_NUMOF ] = {
		292, 293, 294, 296, 298, 333
	};

public:
	// constructors/destructor
	AFrontEndMainMenuState2( AWindmillHelper* a_pWindmillHelper = TNULL, TBOOL a_bFlag = TFALSE );
	~AFrontEndMainMenuState2();

	TBOOL StartIntroVideo();

	// Sets camera to it's position it should have in front end
	static void SetupCamera();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	void UpdateMenuButtons( TFLOAT a_fDeltaTime );
	void UpdateFocusedButton( TFLOAT a_fDeltaTime );

public:
	inline static TBOOL  ms_bIsInserted                = TFALSE;
	inline static TFLOAT ms_fPlayIntroAFKTime          = 30.0f;
	inline static TFLOAT ms_fFocusedButtonMaxOffset    = 3.0f;
	inline static TFLOAT ms_fFocusedButtonMaxScale     = 1.25f;
	inline static TFLOAT ms_fFocusedButtonTextMaxScale = 1.25f;
	inline static TFLOAT ms_fFocusedButtonAnimSpeed    = 20.0f;

private:
	AGUI2Button                                             m_aButtons[ FRONTENDBUTTON_NUMOF ];
	TINT                                                    m_iActivatedButtonID;
	Toshi::T2GUIRectangle                                   m_Rectangle1;
	Toshi::T2GUIRectangle                                   m_Rectangle2;
	Toshi::TGenericListener                                 m_oUnkListener;
	AWindmillHelper*                                        m_pWindmillHelper;
	TBOOL                                                   m_bFlag;
	TFLOAT                                                  m_fAFKTime;
	Toshi::TListener<AFade, TBOOL, AFrontEndMainMenuState2> m_oFadeOverListener;
};
