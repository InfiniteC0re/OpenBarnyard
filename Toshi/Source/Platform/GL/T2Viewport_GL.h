#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class.
//-----------------------------------------------------------------------------
class T2Viewport
{
public:
	constexpr T2Viewport()
	    : m_fX( 0.0f )
	    , m_fY( 0.0f )
	    , m_fWidth( 1.0f )
	    , m_fHeight( 1.0f )
	{}

	constexpr T2Viewport(
	    TFLOAT a_fX,
	    TFLOAT a_fY,
	    TFLOAT a_fWidth,
	    TFLOAT a_fHeight
	)
	    : m_fX( a_fX )
	    , m_fY( a_fY )
	    , m_fWidth( a_fWidth )
	    , m_fHeight( a_fHeight )
	{}

	~T2Viewport() = default;

	TFLOAT GetX() const { return m_fX; }
	TFLOAT GetY() const { return m_fY; }
	TFLOAT GetWidth() const { return m_fWidth; }
	TFLOAT GetHeight() const { return m_fHeight; }

	void SetX( TFLOAT a_fValue ) { m_fX = a_fValue; }
	void SetY( TFLOAT a_fValue ) { m_fY = a_fValue; }
	void SetWidth( TFLOAT a_fValue ) { m_fWidth = a_fValue; }
	void SetHeight( TFLOAT a_fValue ) { m_fHeight = a_fValue; }

	void Begin();
	void End();

private:
	TFLOAT m_fX;
	TFLOAT m_fY;
	TFLOAT m_fWidth;
	TFLOAT m_fHeight;
};

TOSHI_NAMESPACE_END
