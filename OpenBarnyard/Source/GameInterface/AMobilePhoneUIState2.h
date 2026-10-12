#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIFont.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Purpose: The mobile phone ("mobile" inventory item). Shows the text messages
// the player has received (the quests in APlayerSimProfile's used items list),
// one at a time with the picture and the name of the sender.
// TClass 00781a44, vtable 0071acc0.
//-----------------------------------------------------------------------------
class AMobilePhoneUIState2
    : public AGameState
{
public:
	TDECLARE_CLASS( AMobilePhoneUIState2, AGameState );

	static constexpr TINT NUM_SENDERS = 17;

public:
	AMobilePhoneUIState2();
	~AMobilePhoneUIState2();

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

	// Name of the a_iIndex'th unit that can send messages ("miles", "ben", ...)
	static const Toshi::TPString8* GetSenderName( TINT a_iIndex );

	// Index of the unit a_rcName in the senders, -1 if it can't send messages
	static TINT FindSender( const Toshi::TPString8& a_rcName );

private:
	// Shows the message m_iMessage (the "no messages" text if it's 0)
	void UpdateMessage();

	// Goes a_iDirection messages forward (wraps around)
	void ChangeMessage( TINT a_iDirection );

private:
	TINT                  m_iNumMessages;           // 0x0a4
	TINT                  m_iMessage;               // 0x0a8, 1-based, 0 if there are no messages
	TWCHAR                m_wszMessageNumber[ 16 ]; // 0x0ac, " n/m"
	TWCHAR                m_wszDate[ 32 ];          // 0x0cc, "<week day>  <AM/PM>"
	TWCHAR                m_wszPrompt[ 64 ];        // 0x10c
	Toshi::T2GUIElement   m_oPhone;                 // 0x18c
	Toshi::T2GUIRectangle m_oScreenLeft;            // 0x1e0, "Phone_Screen" flipped
	Toshi::T2GUIRectangle m_oScreenRight;           // 0x258, "Phone_Screen"
	Toshi::T2GUIElement   m_oMessageCounter;        // 0x2d0
	AGUI2TextBox          m_oMessagesLabel;         // 0x324
	AGUI2TextBox          m_oMessageNumber;         // 0x39c
	Toshi::T2GUIRectangle m_aArrows[ 2 ];           // 0x414, "left_arrow", "left_arrow2"
	Toshi::T2GUIRectangle m_oMessageIcon;           // 0x504, "SMS_Message"
	AGUI2TextBox          m_oDate;                  // 0x57c
	Toshi::T2GUIRectangle m_oSenderPicture;         // 0x5f4, "Player_SMS" or the picture of the sender
	Toshi::T2GUIRectangle m_oPlayerPicture;         // 0x66c, the hide of the player, if there are no messages
	AGUI2TextBox          m_oSenderName;            // 0x6e4
	AGUI2TextBox          m_oMessage;               // 0x75c
	AGUI2TextBox          m_oPrompt;                // 0x7d4
	Toshi::T2GUIFont*     m_pMessageFont;           // 0x84c, "Rekord18" with bigger line margins
	TFLOAT                m_fPlayerNameScale;       // 0x850
	TBOOL                 m_bMouseMoved;            // 0x854
	TBOOL                 m_bPromptHovered;         // 0x855
	TBOOL                 m_bLeftArrowHovered;      // 0x856
	TBOOL                 m_bRightArrowHovered;     // 0x857
	Toshi::T2GUIRectangle m_oFade;                  // 0x858
};

TSTATICASSERT( sizeof( AMobilePhoneUIState2 ) == 0x8d0 );
