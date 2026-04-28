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
	    , m_vClearColor( TVector4::VEC_ZERO )
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
	    , m_vClearColor( TVector4::VEC_ZERO )
	{}

	~T2Viewport() = default;

	TFLOAT          GetX() const { return m_fX; }
	TFLOAT          GetY() const { return m_fY; }
	TFLOAT          GetWidth() const { return m_fWidth; }
	TFLOAT          GetHeight() const { return m_fHeight; }
	const TVector4& GetClearColor() const { return m_vClearColor; }

	void SetX( TFLOAT a_fValue ) { m_fX = a_fValue; }
	void SetY( TFLOAT a_fValue ) { m_fY = a_fValue; }
	void SetWidth( TFLOAT a_fValue ) { m_fWidth = a_fValue; }
	void SetHeight( TFLOAT a_fValue ) { m_fHeight = a_fValue; }
	void SetClearColor( const TVector4& a_vValue ) { m_vClearColor = a_vValue; }

	void Begin();
	void End();

private:
	TFLOAT m_fX;
	TFLOAT m_fY;
	TFLOAT m_fWidth;
	TFLOAT m_fHeight;

	TVector4 m_vClearColor;
};

TOSHI_NAMESPACE_END
