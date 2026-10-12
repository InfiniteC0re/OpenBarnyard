#pragma once
#include "AGameState.h"
#include "Input/AInputMapManager.h"
#include "GUI/AFade.h"

class AMoviePlayer;

class AMovieState : public AGameState
{
public:
	TDECLARE_CLASS( AMovieState, AGameState );

public:
	AMovieState( const TCHAR* a_szName, TBOOL a_bFlag1, AGameState* a_pNextGameState, TBOOL a_bFlag2 );
	~AMovieState();

	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) override;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnInsertion() override;
	virtual void  OnRemoval() override;

private:
	void  SkipMovie( TBOOL a_bFastSkip );
	TBOOL EndMovieImmediately();

	static TBOOL __stdcall OnFadeOver( AMovieState* a_pMovieState, AFade* a_pFade, TBOOL* a_pData );
	static TBOOL __stdcall OnMoviePlayerEvent( AMovieState* a_pMovieState, AMoviePlayer* a_pMoviePlayer, TINT* a_pEvent );

private:
	Toshi::TPString8                            m_FileName;
	TBOOL                                       m_bFlag1;
	AGameState*                                 m_pNextGameState;
	TBOOL                                       m_bFlag2;
	Toshi::TGenericListener                     m_Listener1;
	Toshi::TListener<AFade, TBOOL, AMovieState> m_FadeOverListener;
};
