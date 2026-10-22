#pragma once
#include "AGameState.h"
#include "Units/ASSeqUI.h"

//-----------------------------------------------------------------------------
// Gender and skin selection of the frontend (TClass 0077dfac, vtable 007165f8),
// opened by the Antics button of the main menu. Uses the UI of the SS01 intro
// sequence (ASSeqUI) and opens the minigame selection once it's done.
//-----------------------------------------------------------------------------
class MODAPI AFrontEndCharacterSelectState : public AGameState
{
public:
	TDECLARE_CLASS( AFrontEndCharacterSelectState, AGameState );

public:
	// a_bAntics:         skips the name page and opens the Antics minigame selection
	// a_bStartMainGame:  loads the main game once the character is chosen
	// a_bDebugSSeq:      opens AFrontEndDebugSSeqState once the character is chosen
	AFrontEndCharacterSelectState( TBOOL a_bAntics, TBOOL a_bStartMainGame, TBOOL a_bDebugSSeq );
	~AFrontEndCharacterSelectState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

#ifdef BARNYARD_COMMUNITY_PATCH
	ASSeqUI& Dev_GetUI()
	{
		return m_oUI;
	}
#endif

private:
	ASSeqUI m_oUI;            // 0xa4
	TBOOL   m_bStartMainGame; // 0x57b4
	TBOOL   m_bAntics;        // 0x57b5
	TBOOL   m_bDebugSSeq;     // 0x57b6
};
