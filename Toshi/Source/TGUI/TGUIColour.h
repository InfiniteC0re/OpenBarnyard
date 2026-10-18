#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: 32-bit colour of the GUI (stored as A, R, G, B bytes).
//-----------------------------------------------------------------------------
class TGUIColour
{
public:
	// Tags selecting the order of the components of a packed colour
	enum FormatARGB
	{
		ARGB
	};

	enum FormatABGR
	{
		ABGR
	};

public:
	// $JPOG: FUNCTION TGuiInterface.dll+00008c30
	TGUIColour()
	    : m_uiAlpha( 255 ), m_uiRed( 255 ), m_uiGreen( 255 ), m_uiBlue( 255 )
	{}

	// $JPOG: FUNCTION TGuiInterface.dll+00008c10
	TGUIColour( TUINT8 a_uiRed, TUINT8 a_uiGreen, TUINT8 a_uiBlue, TUINT8 a_uiAlpha )
	    : m_uiAlpha( a_uiAlpha ), m_uiRed( a_uiRed ), m_uiGreen( a_uiGreen ), m_uiBlue( a_uiBlue )
	{}

	// $JPOG: FUNCTION TGuiInterface.dll+00008ac0
	TGUIColour( const TGUIColour& a_rcOther )
	    : m_uiAlpha( a_rcOther.m_uiAlpha ), m_uiRed( a_rcOther.m_uiRed ), m_uiGreen( a_rcOther.m_uiGreen ), m_uiBlue( a_rcOther.m_uiBlue )
	{}

	TGUIColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha );
	TGUIColour( TUINT a_uiColour, FormatARGB a_eFormat );
	TGUIColour( TUINT a_uiColour, FormatABGR a_eFormat );

	// $JPOG: FUNCTION TGuiInterface.dll+00008aa0
	TGUIColour& operator=( const TGUIColour& a_rcOther )
	{
		m_uiRed   = a_rcOther.m_uiRed;
		m_uiGreen = a_rcOther.m_uiGreen;
		m_uiBlue  = a_rcOther.m_uiBlue;
		m_uiAlpha = a_rcOther.m_uiAlpha;
		return *this;
	}

	TGUIColour& operator*=( const TGUIColour& a_rcOther );

	// $JPOG: FUNCTION TGuiInterface.dll+00008980
	TBOOL operator==( const TGUIColour& a_rcOther ) const { return GetPacked() == a_rcOther.GetPacked(); }

	// $JPOG: FUNCTION TGuiInterface.dll+00008960
	TBOOL operator!=( const TGUIColour& a_rcOther ) const { return GetPacked() != a_rcOther.GetPacked(); }

	void SetAlpha( TFLOAT a_fAlpha );

	// NOTE: the first argument receives the alpha
	void Get( TFLOAT& a_rfAlpha, TFLOAT& a_rfRed, TFLOAT& a_rfGreen, TFLOAT& a_rfBlue ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+000089e0
	TUINT GetABGR() const { return m_uiRed | ( m_uiGreen << 8 ) | ( m_uiBlue << 16 ) | ( m_uiAlpha << 24 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00008a00
	TUINT GetARGB() const { return m_uiBlue | ( m_uiGreen << 8 ) | ( m_uiRed << 16 ) | ( m_uiAlpha << 24 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00008a20
	TUINT GetPlatformColour() const { return GetARGB(); }

	TUINT8 GetAlpha() const { return m_uiAlpha; }
	TUINT8 GetRed() const { return m_uiRed; }
	TUINT8 GetGreen() const { return m_uiGreen; }
	TUINT8 GetBlue() const { return m_uiBlue; }

private:
	// The 4 bytes as they are in memory
	TUINT32 GetPacked() const { return *TREINTERPRETCAST( const TUINT32*, this ); }

private:
	TUINT8 m_uiAlpha; // 0x0
	TUINT8 m_uiRed;   // 0x1
	TUINT8 m_uiGreen; // 0x2
	TUINT8 m_uiBlue;  // 0x3
};

TOSHI_NAMESPACE_END
