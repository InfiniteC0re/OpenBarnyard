#include "ToshiPCH.h"
#include "TGUITextureAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUITextureAnimation );

// $NicktoonsPS2: FUNCTION 00387ce0
// $JPOG: FUNCTION TGuiInterface.dll+0001a5b0
TGUITextureAnimation::TGUITextureAnimation()
    : m_fFrequency( 1.0f )
    , m_aFrames( 10, 0 )
{
}

// $NicktoonsPS2: FUNCTION 00387d38
// $JPOG: FUNCTION TGuiInterface.dll+0001a180
TGUITextureAnimation::TGUITextureAnimation( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iNumFrames, TFLOAT a_fFrequency, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY )
    : TGUITextureSection( a_iTexture, a_rcName, a_iX, a_iY, a_iWidth, a_iHeight, a_iRotation, a_bWrap, a_bMirrorX, a_bMirrorY )
    , m_fFrequency( a_fFrequency )
    , m_aFrames( 10, 0 )
{
	if ( a_iNumFrames > 0 )
		m_aFrames.Resize( a_iNumFrames );

	m_aFrames.SetSize( a_iNumFrames );

	for ( TINT i = 0; i < a_iNumFrames; i++ )
		m_aFrames[ i ] = Frame{ 0, 0, 0, 0 };
}

// $NicktoonsPS2: FUNCTION 00387ed8
// $JPOG: FUNCTION TGuiInterface.dll+0001a490
TGUITextureAnimation::TGUITextureAnimation( const TGUITextureAnimation& a_rcOther )
    : TGUITextureSection( a_rcOther )
    , m_fFrequency( a_rcOther.m_fFrequency )
    , m_aFrames( a_rcOther.m_aFrames.GetGrowSize(), a_rcOther.m_aFrames.Size() )
{
	m_aFrames.SetSize( a_rcOther.m_aFrames.Size() );

	for ( TINT i = 0; i < m_aFrames.Size(); i++ )
		m_aFrames[ i ] = a_rcOther.m_aFrames[ i ];
}

// $NicktoonsPS2: FUNCTION 00387ff0
// $JPOG: FUNCTION TGuiInterface.dll+0001a580
TGUITextureAnimation::~TGUITextureAnimation()
{
}

// $NicktoonsPS2: FUNCTION 00388048
// $JPOG: FUNCTION TGuiInterface.dll+0001a550
TGUITextureAnimation* TGUITextureAnimation::Clone() const
{
	return new TGUITextureAnimation( *this );
}

// $NicktoonsPS2: FUNCTION 00388098
// $JPOG: FUNCTION TGuiInterface.dll+0001a2c0
void TGUITextureAnimation::GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TFLOAT a_fTime ) const
{
	if ( m_aFrames.Size() != 0 )
	{
		const Frame& rcFrame = m_aFrames[ TINT( fmodf( a_fTime, m_fFrequency * m_aFrames.Size() ) / m_fFrequency ) ];

		a_rX      = rcFrame.uiX;
		a_rY      = rcFrame.uiY;
		a_rWidth  = rcFrame.uiWidth;
		a_rHeight = rcFrame.uiHeight;
	}
	else
	{
		a_rX      = m_uiX;
		a_rY      = m_uiY;
		a_rWidth  = m_uiWidth;
		a_rHeight = m_uiHeight;
	}
}

// $NicktoonsPS2: FUNCTION 00388170
// $JPOG: FUNCTION TGuiInterface.dll+0001a3b0
void TGUITextureAnimation::GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TINT a_iFrame ) const
{
	if ( a_iFrame != 0 )
	{
		const Frame& rcFrame = m_aFrames[ a_iFrame - 1 ];

		a_rX      = rcFrame.uiX;
		a_rY      = rcFrame.uiY;
		a_rWidth  = rcFrame.uiWidth;
		a_rHeight = rcFrame.uiHeight;
	}
	else
	{
		a_rX      = m_uiX;
		a_rY      = m_uiY;
		a_rWidth  = m_uiWidth;
		a_rHeight = m_uiHeight;
	}
}

// $NicktoonsPS2: FUNCTION 003881d0
// $JPOG: FUNCTION TGuiInterface.dll+0001a430
void TGUITextureAnimation::SetBBox( TUINT16 a_uiX, TUINT16 a_uiY, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TINT a_iFrame )
{
	if ( a_iFrame != 0 )
	{
		Frame& rFrame = m_aFrames[ a_iFrame - 1 ];

		rFrame.uiX      = a_uiX;
		rFrame.uiY      = a_uiY;
		rFrame.uiWidth  = a_uiWidth;
		rFrame.uiHeight = a_uiHeight;
	}
	else
	{
		m_uiX      = a_uiX;
		m_uiY      = a_uiY;
		m_uiWidth  = a_uiWidth;
		m_uiHeight = a_uiHeight;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0001a270
void TGUITextureAnimation::InsertFrame( TINT a_iFrame, Frame& a_rFrame )
{
	m_aFrames.InsertBefore( TArray<Frame>::Iterator( a_iFrame - 1, m_aFrames ), a_rFrame );
}

// $JPOG: FUNCTION TGuiInterface.dll+0001a210
void TGUITextureAnimation::RemoveFrame( TINT a_iFrame )
{
	TINT iLast = m_aFrames.Size() - 1;

	for ( TINT i = a_iFrame - 1; i < iLast; i++ )
		m_aFrames[ i ] = m_aFrames[ i + 1 ];

	m_aFrames.SetSize( iLast );
}

TOSHI_NAMESPACE_END
