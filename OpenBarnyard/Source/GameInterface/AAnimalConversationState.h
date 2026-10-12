#pragma once
#include "AGameState.h"
#include "Cameras/ACameraHelperSpline.h"
#include "Units/AInteractionSequence.h"
#include "Units/AInteractionActivity.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TGlobalEmitter.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: Thrown when the player picks a choice of AAnimalConversationState.
// NOTE: original name from the Wii build. vtable 0x0071a50c, listener list at
// 0x007818e0. On PC nothing throws it, AAnimalConversationState passes it to
// its own handler.
//-----------------------------------------------------------------------------
class AAnimalConversationEvent
    : public Toshi::TGlobalEvent<AAnimalConversationEvent>
{
public:
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_NONE   = 0,
		TYPE_CHOICE = 1, // the player picked m_iChoice
	};

public:
	AAnimalConversationEvent( TYPE a_eType, TINT a_iChoice )
	    : m_iUnk4( 0 )
	    , m_eType( a_eType )
	    , m_iChoice( a_iChoice )
	{}

	TYPE GetType() const { return m_eType; }
	TINT GetChoice() const { return m_iChoice; }

private:
	TINT m_iUnk4;   // 0x04
	TYPE m_eType;   // 0x08
	TINT m_iChoice; // 0x0c
};

//-----------------------------------------------------------------------------
// Purpose: The player talks to a unit (APlayerControlComponent pushes it with
// the sequence of the "player" interaction). Plays the interactions of the
// sequence one after the other: once an interaction is over it picks the next
// one or, if there are several candidates, the first of them sorted by their
// display order (the choice dialog is never shown on PC). The player stands
// still meanwhile and can turn the camera with the right stick.
// TClass 0x007818e8, vtable 0x0071a510, size 0x1ac.
//-----------------------------------------------------------------------------
class AAnimalConversationState
    : public AGameState
{
public:
	TDECLARE_CLASS( AAnimalConversationState, AGameState );

	static constexpr TINT MAX_NUM_CHOICES = 6;

	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_PLAYING  = 0, // an interaction is played
		STATE_STARTED  = 1, // waiting for the current interaction to end
		STATE_CHOOSING = 2, // the next interaction has been picked from the candidates
		STATE_UNUSED   = 3,
		STATE_DONE     = 4,
	};

	using ActivityListener = Toshi::TGlobalListener<AAnimalConversationState, AInteractionActivityEvent>;

public:
	AAnimalConversationState( AUnit* a_pUnit, AInteractionSequence* a_pSequence );
	~AAnimalConversationState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;                                                        // +0x0c
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	// TRUE if a_pUnit has an AInteractionComponent that allows conversations
	static TBOOL CanConverse( AUnit* a_pUnit );

	// Starts the "player" interaction between a_pUnit and the player (ends the current sequence of
	// a_pUnit first), TNULL if a_pUnit can't talk now
	static AInteractionSequence* StartConversation( AUnit* a_pUnit );

private:
	// Picks the next interaction of the sequence, TFALSE if there's none
	TBOOL PickNextInteraction();

	// Collects the candidates for the next interaction and starts the first of them
	TBOOL FindChoices( AInteractionSequenceRef a_oSequence );

	void OnConversationEvent( const AAnimalConversationEvent& a_rcEvent );
	void OnActivityEvent( const AInteractionActivityEvent& a_rcEvent );

	static void OnActivityEventCallback( void* a_pCaller, const AInteractionActivityEvent& a_rcEvent );

	// Skips the speech the player is listening to
	static void SkipPlayerSpeech();

private:
	TINT                    m_aiCandidates[ MAX_NUM_CHOICES ]; // 0x0a4, interactions the next one is picked from
	TINT                    m_iNumCandidates;                  // 0x0bc
	ACameraHelperSpline     m_oCameraSpline;                   // 0x0c0
	Toshi::TPString8        m_strMiniGame;                     // 0x140, minigame started once the conversation is over, never set
	Toshi::TPString8        m_strPlayer;                       // 0x144, "player"
	Toshi::TPString8        m_strUnk148;                       // 0x148
	AInteractionSequence*   m_pStartSequence;                  // 0x14c
	AInteractionSequenceRef m_oSequence;                       // 0x150
	STATE                   m_eState;                          // 0x15c
	TINT                    m_iInteraction;                    // 0x160, the interaction started next
	TBOOL                   m_bReady;                          // 0x164, the units reached each other
	TFLOAT                  m_fStartDelay;                     // 0x168
	TFLOAT                  m_fStartTime;                      // 0x16c
	TBOOL                   m_bStartPending;                   // 0x170, m_iInteraction starts after m_fStartDelay
	TFLOAT                  m_fWalkDistance;                   // 0x174, twice the radius of the walk activity, never read
	Toshi::TRef<AUnitProxy> m_pPlayer;                         // 0x178
	AUnit*                  m_pUnit;                           // 0x17c, the unit the player talks to
	ActivityListener        m_oActivityListener;               // 0x180
	Toshi::TRef<AUnitProxy> m_pUnitProxy;                      // 0x190
	TFLOAT                  m_fCameraYaw;                      // 0x194, right stick input
	TFLOAT                  m_fCameraPitch;                    // 0x198, right stick input
	TINT                    m_eUnitSteerMode;                  // 0x19c, AInteriorSteer mode of m_pUnit restored by OnRemoval
	TINT                    m_iNumChoices;                     // 0x1a0
	TINT                    m_iPickedChoice;                   // 0x1a4, -1 if none
	TFLOAT                  m_fInputDelay;                     // 0x1a8
};

TSTATICASSERT( sizeof( AAnimalConversationState ) == 0x1ac );
