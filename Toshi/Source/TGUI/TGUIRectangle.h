#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: integer rectangle of the GUI (position and size).
//-----------------------------------------------------------------------------
class TGUIRectangle
{
public:
	TGUIRectangle() {}

	TGUIRectangle( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
	    : m_iX( a_iX ), m_iY( a_iY ), m_iWidth( a_iWidth ), m_iHeight( a_iHeight )
	{}

	void Set( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
	{
		m_iX      = a_iX;
		m_iY      = a_iY;
		m_iWidth  = a_iWidth;
		m_iHeight = a_iHeight;
	}

	TINT GetX() const { return m_iX; }
	TINT GetY() const { return m_iY; }
	TINT GetWidth() const { return m_iWidth; }
	TINT GetHeight() const { return m_iHeight; }
	TINT GetRight() const { return m_iX + m_iWidth; }
	TINT GetBottom() const { return m_iY + m_iHeight; }

public:
	TINT m_iX;      // 0x0
	TINT m_iY;      // 0x4
	TINT m_iWidth;  // 0x8
	TINT m_iHeight; // 0xC
};

TOSHI_NAMESPACE_END
