#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: integer 2D point of the GUI.
//-----------------------------------------------------------------------------
class TGUIPoint
{
public:
	// NOTE: doesn't initialise the point
	// $JPOG: FUNCTION TGuiInterface.dll+00008530
	TGUIPoint() {}

	// $JPOG: FUNCTION TGuiInterface.dll+00008540
	TGUIPoint( TINT a_iX, TINT a_iY )
	    : m_iX( a_iX ), m_iY( a_iY )
	{}

	// $JPOG: FUNCTION TGuiInterface.dll+00008480
	TGUIPoint& operator=( const TGUIPoint& a_rcOther )
	{
		m_iX = a_rcOther.m_iX;
		m_iY = a_rcOther.m_iY;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000084c0
	TGUIPoint& operator+=( const TGUIPoint& a_rcOther )
	{
		m_iX += a_rcOther.m_iX;
		m_iY += a_rcOther.m_iY;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000084a0
	TGUIPoint& operator-=( const TGUIPoint& a_rcOther )
	{
		m_iX -= a_rcOther.m_iX;
		m_iY -= a_rcOther.m_iY;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000084e0
	void Set( TINT a_iX, TINT a_iY )
	{
		m_iX = a_iX;
		m_iY = a_iY;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00008500
	void SetX( TINT a_iX ) { m_iX = a_iX; }

	// $JPOG: FUNCTION TGuiInterface.dll+000084f0
	void SetY( TINT a_iY ) { m_iY = a_iY; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008520
	TINT GetX() const { return m_iX; }

	// $JPOG: FUNCTION TGuiInterface.dll+00008510
	TINT GetY() const { return m_iY; }

private:
	TINT m_iX; // 0x0
	TINT m_iY; // 0x4
};

TOSHI_NAMESPACE_END
