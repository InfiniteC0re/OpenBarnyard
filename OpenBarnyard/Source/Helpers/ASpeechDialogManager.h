#pragma once
#include "GUI/ASpeechDialog.h"
#include "GUI/ASpeechChoiceDialog.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TDList.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: Shows the speech bubbles (ASpeechDialog) of ASpeechActivity and
// ASpeechSeqActivity: opens the bubble, types the text letter by letter, waits
// for the player and closes it again. Also owns the choice dialog
// (ASpeechChoiceDialog) of AAnimalConversationState.
// TClass 0x00783e9c, vtable 0x007246fc, size 0x44, singleton at 0x00783e98.
//-----------------------------------------------------------------------------
class ASpeechDialogManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ASpeechDialogManager>
{
public:
	TDECLARE_CLASS( ASpeechDialogManager, Toshi::TTask );

	static constexpr TINT NUM_SPEECHES = 1;

	// A speech bubble
	struct Speech
	    : public Toshi::TGenericDList::TNode
	{
		using STATE = TINT;
		enum STATE_ : STATE
		{
			STATE_NONE    = 0, // closed, returned to the free list by OnUpdate
			STATE_OPENING = 1,
			STATE_SHOWN   = 2, // the text is fully shown
			STATE_TYPING  = 3,
			STATE_TYPED   = 4, // the text has been typed, shown for a moment before STATE_SHOWN
			STATE_CLOSING = 5,
			STATE_HIDDEN  = 6, // started hidden
		};

		Speech();
		~Speech();

		// Shows a_wszText said by a_pUnit in the bubble, a_bCreate opens a new bubble
		void Start( TBOOL a_bCreate, const TWCHAR* a_wszName, const TWCHAR* a_wszText, AUnit* a_pUnit, TUINT a_uiId, TBOOL a_bHidden );

		// TFALSE once the speech is closed
		TBOOL Update( TFLOAT a_fDeltaTime );

		// Shows the rest of the text at once
		void FinishTyping();

		TUINT                   uiId;         // 0x08, id of the sequence the speech belongs to
		Toshi::TRef<AUnitProxy> pUnit;        // 0x0c, the speaking unit
		ASpeechDialog*          pDialog;      // 0x10
		TUINT32                 uiTextColour; // 0x14
		TUINT32                 uiNameColour; // 0x18
		TFLOAT                  fScale;       // 0x1c
		TWCHAR*                 wszText;      // 0x20, the text that is being typed
		TWCHAR                  wcSaved;      // 0x24, the character replaced by the terminator while typing
		TINT                    iChar;        // 0x28, index of the typed character, -1 if none
		TFLOAT                  fTime;        // 0x2c
		STATE                   eState;       // 0x30
	};

	// The choice dialog
	// NOTE: Only the first 0x14 bytes of the 0xc14 retail allocates are used.
	struct Choice
	    : public Toshi::TGenericDList::TNode
	{
		using STATE = TINT;
		enum STATE_ : STATE
		{
			STATE_NONE    = 0,
			STATE_OPENING = 1,
			STATE_SHOWN   = 2,
			STATE_CLOSING = 3,
		};

		Choice();
		~Choice();

		// TFALSE once the dialog is closed
		TBOOL Update( TFLOAT a_fDeltaTime );

		// $Barnyard: FUNCTION 0054b9e0
		TINT GetSelection() const { return pDialog->GetSelection(); }
		void SetSelection( TINT a_iSelection );

		ASpeechChoiceDialog* pDialog;                 // 0x08
		TFLOAT               fScale;                  // 0x0c
		STATE                eState;                  // 0x10
		TUINT8               PADDING[ 0xc14 - 0x14 ]; // 0x14
	};

public:
	ASpeechDialogManager();
	~ASpeechDialogManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;                         // +0x14
	virtual TBOOL OnCreate() OVERRIDE;                      // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c

	// Shows a_wszText said by a_pUnit (a_wszName is its name) in the speech bubble,
	// a_uiId identifies the speech in the other methods. Returns 0.0f if no bubble is free.
	TFLOAT StartSpeech( const TWCHAR* a_wszName, const TWCHAR* a_wszText, AUnit* a_pUnit, TUINT a_uiId, TBOOL a_bHidden );

	// Opens an empty bubble if none is shown, TFALSE while a bubble is opening
	TBOOL PrepareSpeech();

	// TRUE if the speech a_uiId is fully shown (or doesn't exist)
	TBOOL IsSpeechFinished( TUINT a_uiId );

	// Shows the rest of the speech a_uiId if it's still typed, TRUE if it was fully shown
	TBOOL SkipSpeech( TUINT a_uiId );

	// Clears the speech a_uiId, the bubble stays open
	void StopSpeech( TUINT a_uiId );

	// Closes every bubble, TRUE if there were none
	TBOOL EndAllSpeeches();

	// Closes the choice dialog
	void CloseChoice();

	Choice* GetChoice() const { return m_pChoice; }

private:
	Speech* FindSpeech( TUINT a_uiId );

	// A bubble that is shown and can be reused
	Speech* FindReusableSpeech();

	// Returns a_pSpeech to the free list
	// NOTE: inlined into Reset and OnUpdate
	void FreeSpeech( Speech* a_pSpeech );

private:
	Toshi::TDList<Speech> m_llFreeSpeeches; // 0x20
	Toshi::TDList<Speech> m_llUsedSpeeches; // 0x28
	Speech*               m_pSpeeches;      // 0x30
	Choice*               m_pChoice;        // 0x34
	TBOOL                 m_bChoiceShown;   // 0x38
	TINT                  m_iUnk3C;         // 0x3c, unused
	Toshi::T2GUIFont*     m_pFont;          // 0x40, copy of Rekord18 with a bigger line margin
};

TSTATICASSERT( sizeof( ASpeechDialogManager::Speech ) == 0x34 );
TSTATICASSERT( sizeof( ASpeechDialogManager::Choice ) == 0xc14 );
