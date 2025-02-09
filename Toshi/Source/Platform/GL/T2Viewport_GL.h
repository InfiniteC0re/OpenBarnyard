#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class.
//-----------------------------------------------------------------------------
class T2Viewport
{
public:
	constexpr T2Viewport(
	    TFLOAT a_fX,
	    TFLOAT a_fY,
	    TFLOAT a_fWidth,
	    TFLOAT a_fHeight,
	    TFLOAT a_fMinZ,
	    TFLOAT a_fMaxZ
	)
	    : m_fX( a_fX )
	    , m_fY( a_fY )
	    , m_fWidth( a_fWidth )
	    , m_fHeight( a_fHeight )
	    , m_fMinZ( a_fMinZ )
	    , m_fMaxZ( a_fMaxZ )
	{}

	~T2Viewport() = default;

	TFLOAT GetX() const { return m_fX; }
	TFLOAT GetY() const { return m_fY; }
	TFLOAT GetWidth() const { return m_fWidth; }
	TFLOAT GetHeight() const { return m_fHeight; }
	TFLOAT GetMinZ() const { return m_fMinZ; }
	TFLOAT GetMaxZ() const { return m_fMaxZ; }

	void SetX( TFLOAT a_fValue ) { m_fX = a_fValue; }
	void SetY( TFLOAT a_fValue ) { m_fY = a_fValue; }
	void SetWidth( TFLOAT a_fValue ) { m_fWidth = a_fValue; }
	void SetHeight( TFLOAT a_fValue ) { m_fHeight = a_fValue; }
	void SetMinZ( TFLOAT a_fValue ) { m_fMinZ = a_fValue; }
	void SetMaxZ( TFLOAT a_fValue ) { m_fMaxZ = a_fValue; }

private:
	TFLOAT m_fX;
	TFLOAT m_fY;
	TFLOAT m_fWidth;
	TFLOAT m_fHeight;
	TFLOAT m_fMinZ;
	TFLOAT m_fMaxZ;
};

TOSHI_NAMESPACE_END
