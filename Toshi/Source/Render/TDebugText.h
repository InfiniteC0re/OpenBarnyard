#pragma once

TOSHI_NAMESPACE_START

class TRenderInterface;

// Buffered on-screen debug text. Entries are added to the buffer and drawn (then cleared) by Render.
class TDebugText
{
public:
	enum ENTRYFLAGS_ : TUINT16
	{
		ENTRYFLAGS_NONE     = 0,
		ENTRYFLAGS_LOG      = BITFLAG( 0 ), // drawn at the left edge, one line (11 pixels) below the previous log entry
		ENTRYFLAGS_CENTERED = BITFLAG( 1 ), // centered on the position
	};

	// Followed by the text and its terminator
	struct Entry
	{
		TINT16  iX;          // 0x0
		TINT16  iY;          // 0x2
		TUINT16 uiLength;    // 0x4
		TUINT16 uiFlags;     // 0x6
		TUINT32 uiColour;    // 0x8
		TCHAR   szText[ 1 ]; // 0xC
	};

public:
	// $Barnyard: FUNCTION 006dd700
	TDebugText()
	    : m_bEnabled( TTRUE )
	{}

	// Clears the buffer
	virtual void Reset() = 0; // +0x00
	// Destroys the debug text and frees its memory
	virtual void Destroy() = 0; // +0x04
	// Draws every entry and clears the buffer
	virtual void Render() = 0; // +0x08
	// Returns space for an entry with a_uiLength characters, TNULL if the buffer is full
	virtual Entry* AllocateEntry( TUINT16 a_uiLength ) = 0; // +0x0C

	// $Barnyard: FUNCTION 006dd710
	// $Barnyard: FUNCTION 006dd730
	virtual ~TDebugText() {} // +0x10

	void  SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }
	TBOOL IsEnabled() const { return m_bEnabled; }

protected:
	TRenderInterface* m_pRenderer;   // 0x04
	TINT              m_iBufferSize; // 0x08
	TUINT8*           m_pCurrent;    // 0x0C, where the next entry goes
	TBOOL             m_bEnabled;    // 0x10
};

TOSHI_NAMESPACE_END
