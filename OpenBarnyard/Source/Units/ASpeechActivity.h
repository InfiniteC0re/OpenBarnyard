#pragma once
#include "AInteractionActivity.h"

class AUnitFacialComponent;

//-----------------------------------------------------------------------------
// A unit says one of the "message" texts (randomly picked) in a speech bubble,
// playing the talk animation while the other units play the listen animation.
//-----------------------------------------------------------------------------
class ASpeechActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASpeechActivity, AInteractionActivity );

	static constexpr TINT MAX_NUM_TEXTS = 4;

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                      = 0,
		FLAGS_BUBBLE_VISIBLE            = BITFLAG( 0 ), // "bubblevisible"
		FLAGS_BUBBLE_VISIBLE_FOR_PLAYER = BITFLAG( 1 ), // "bubblevisibleforplayer"
		FLAGS_FORCE_VISIBLE             = BITFLAG( 2 ), // "forcevisible"
		FLAGS_SFX                       = BITFLAG( 3 ), // "sfx" is set
		FLAGS_FORCE_NO_ANIM             = BITFLAG( 4 ), // "forcenoanim"
	};

	// Scratch memory of the activity in the sequence (see GetDataSize)
	// NOTE: Only the fields OnStart and Update use are known.
	struct SpeechData
	{
		using STATE = TUINT8;
		enum STATE_ : STATE
		{
			STATE_SPEAKING = 0, // the speech is shown
			STATE_STOPPED  = 1, // the speech was stopped, waits for the units to idle
		};

		TINT   iSpeech;        // 0x00, speech bubble the activity shows, -1 if none
		TUINT  uiSequenceId;   // 0x04, id of the sequence the speech was started in
		STATE  uiState;        // 0x08
		TUINT8 uiUnk09;        // 0x09
		TINT   iUnk0c;         // 0x0c, sound played with the speech, -1 if none
		TWCHAR wszName[ 32 ];  // 0x10, upper case name of the speaking unit
		TWCHAR wszText[ 256 ]; // 0x50, the text with the tags replaced
	};

public:
	ASpeechActivity();
	~ASpeechActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;        // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                       // +0x10
	virtual TBOOL  GetText( TWCHAR** a_ppText, TBOOL a_bKeep ) OVERRIDE;        // +0x14
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE;       // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;        // +0x1c
	virtual AUnit* GetTargetUnit( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x28
	virtual TBOOL  CanBeSkipped() OVERRIDE { return TTRUE; }                    // +0x30, RETURNS_TRUE

	// Skips the speech of a_pSequence if it's fully shown, shows the rest of it otherwise
	static TBOOL SkipSpeech( AInteractionSequence* a_pSequence );

	FLAGS GetFlags() const { return m_uiFlags; }

	// Plays the animation a_rcName on a_pUnit, unless a_bForce, only if the unit is an
	// AAnimatedUnit that doesn't move. TRUE if the animation plays or is planned
	static TBOOL PlayAnim( AUnit* a_pUnit, const Toshi::TPString8& a_rcName, TBOOL a_bForce );

	// TRUE if a_pUnit has the animation a_rcName
	static TBOOL HasAnim( AUnit* a_pUnit, const Toshi::TPString8& a_rcName );

private:
	// TRUE if the speech camera shows one of the units of a_pSequence
	static TBOOL IsSpeakerShown( AInteractionSequence* a_pSequence );

	// The text a_iText with the %tags replaced, written to the SpeechData of a_pSequence
	TWCHAR* FormatText( AInteractionSequence* a_pSequence, TINT a_iText );

	// Shows the text said by a_pUnit in the speech bubble, returns the index of the text or -1
	TINT ShowSpeech( AInteractionSequence* a_pSequence, AUnit* a_pUnit, TUINT a_uiId, void* a_pUnused, TBOOL a_bVisible, TINT a_iUnused );

	// Plays the idle animation on a_pUnit
	void PlayIdleAnim( AUnit* a_pUnit, TUINT a_uiId );

private:
	TINT                  m_aiTexts[ MAX_NUM_TEXTS ]; // 0x08, "message", locale string ids, -1 if unused
	Toshi::TPString8      m_strTalkAnim;              // 0x18, "talk_01" by default
	Toshi::TPString8      m_strListenAnim;            // 0x1c, "listenanim", "idle_lp" by default
	TINT                  m_iUnk20;                   // 0x20
	TINT                  m_iText;                    // 0x24, index of the text picked by GetText, -1 if none
	FLAGS                 m_uiFlags;                  // 0x28
	TINT8                 m_iNumTexts;                // 0x29
	TUINT8                m_uiUnk2a;                  // 0x2a
	TINT8                 m_iCamera;                  // 0x2b, "camera"
	TINT16                m_iSfx;                     // 0x2c, "sfx"
	AUnit*                m_pPlayer;                  // 0x30, set by OnStart
	AUnitFacialComponent* m_pPlayerFacial;            // 0x34, AUnitFacialComponent of m_pPlayer
};

TSTATICASSERT( sizeof( ASpeechActivity ) == 0x38 );
