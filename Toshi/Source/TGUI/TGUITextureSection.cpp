#include "ToshiPCH.h"
#include "TGUITextureSection.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TGUITextureSection );

// $NicktoonsPS2: FUNCTION 00388360
// $JPOG: FUNCTION TGuiInterface.dll+0001ab70
TGUITextureSection::TGUITextureSection()
    : m_iTexture( -1 )
    , m_uiX( 0 )
    , m_uiY( 0 )
    , m_uiWidth( 0 )
    , m_uiHeight( 0 )
    , m_uiRotation( 0 )
    , m_bWrap( TFALSE )
    , m_bMirrorX( TFALSE )
    , m_bMirrorY( TFALSE )
{
}

// $NicktoonsPS2: FUNCTION 003883e8
// $JPOG: FUNCTION TGuiInterface.dll+0001ac00
TGUITextureSection::TGUITextureSection( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY )
    : m_iTexture( a_iTexture )
    , m_strName( a_rcName )
    , m_uiX( TUINT16( a_iX ) )
    , m_uiY( TUINT16( a_iY ) )
    , m_uiWidth( TUINT16( a_iWidth ) )
    , m_uiHeight( TUINT16( a_iHeight ) )
    , m_uiRotation( a_iRotation & 3 )
    , m_bWrap( a_bWrap )
    , m_bMirrorX( a_bMirrorX )
    , m_bMirrorY( a_bMirrorY )
{
}

// NOTE: the node isn't copied
// $NicktoonsPS2: FUNCTION 00388518
// $JPOG: FUNCTION TGuiInterface.dll+0001abc0
TGUITextureSection::TGUITextureSection( const TGUITextureSection& a_rcOther )
    : TObject()
    , TNodeList<TGUITextureSection>::TNode()
{
	Assign( a_rcOther );
}

// $NicktoonsPS2: FUNCTION 00388578
// $JPOG: FUNCTION TGuiInterface.dll+0001afc0
TGUITextureSection::~TGUITextureSection()
{
}

// $NicktoonsPS2: FUNCTION 003885e0
// $JPOG: FUNCTION TGuiInterface.dll+0001af90
TGUITextureSection* TGUITextureSection::Clone() const
{
	return new TGUITextureSection( *this );
}

// $NicktoonsPS2: FUNCTION 003888f8
// $JPOG: FUNCTION TGuiInterface.dll+00009eb0
void TGUITextureSection::GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TFLOAT a_fTime ) const
{
	a_rX      = m_uiX;
	a_rY      = m_uiY;
	a_rWidth  = m_uiWidth;
	a_rHeight = m_uiHeight;
}

// $NicktoonsPS2: FUNCTION 00388920
// $JPOG: FUNCTION TGuiInterface.dll+00009e80
void TGUITextureSection::GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TINT a_iFrame ) const
{
	a_rX      = m_uiX;
	a_rY      = m_uiY;
	a_rWidth  = m_uiWidth;
	a_rHeight = m_uiHeight;
}

// $NicktoonsPS2: FUNCTION 00388948
// $JPOG: FUNCTION TGuiInterface.dll+00009e50
void TGUITextureSection::SetBBox( TUINT16 a_uiX, TUINT16 a_uiY, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TINT a_iFrame )
{
	m_uiX      = a_uiX;
	m_uiY      = a_uiY;
	m_uiWidth  = a_uiWidth;
	m_uiHeight = a_uiHeight;
}

// $NicktoonsPS2: FUNCTION 00388740
// $JPOG: FUNCTION TGuiInterface.dll+0001ad70
void TGUITextureSection::Set( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY )
{
	m_iTexture   = a_iTexture;
	m_strName    = a_rcName;
	m_uiX        = TUINT16( a_iX );
	m_uiY        = TUINT16( a_iY );
	m_uiWidth    = TUINT16( a_iWidth );
	m_uiHeight   = TUINT16( a_iHeight );
	m_uiRotation = a_iRotation & 3;
	m_bWrap      = a_bWrap;
	m_bMirrorX   = a_bMirrorX;
	m_bMirrorY   = a_bMirrorY;
}

// $NicktoonsPS2: FUNCTION 00388628
// $JPOG: FUNCTION TGuiInterface.dll+0001ae30
void TGUITextureSection::Assign( const TGUITextureSection& a_rcOther )
{
	m_iTexture   = a_rcOther.m_iTexture;
	m_strName    = a_rcOther.m_strName;
	m_uiX        = a_rcOther.m_uiX;
	m_uiY        = a_rcOther.m_uiY;
	m_uiWidth    = a_rcOther.m_uiWidth;
	m_uiHeight   = a_rcOther.m_uiHeight;
	m_uiRotation = a_rcOther.m_uiRotation;
	m_bWrap      = a_rcOther.m_bWrap;
	m_bMirrorX   = a_rcOther.m_bMirrorX;
	m_bMirrorY   = a_rcOther.m_bMirrorY;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001af70
TGUITextureSection& TGUITextureSection::operator=( const TGUITextureSection& a_rcOther )
{
	Assign( a_rcOther );
	return *this;
}

// $JPOG: FUNCTION TGuiInterface.dll+0001acc0
TBOOL TGUITextureSection::operator==( const TGUITextureSection& a_rcOther ) const
{
	return m_iTexture == a_rcOther.m_iTexture &&
	    m_strName == a_rcOther.m_strName &&
	    m_uiX == a_rcOther.m_uiX &&
	    m_uiY == a_rcOther.m_uiY &&
	    m_uiWidth == a_rcOther.m_uiWidth &&
	    m_uiHeight == a_rcOther.m_uiHeight &&
	    m_uiRotation == a_rcOther.m_uiRotation &&
	    m_bWrap == a_rcOther.m_bWrap &&
	    m_bMirrorX == a_rcOther.m_bMirrorX &&
	    m_bMirrorY == a_rcOther.m_bMirrorY;
}

TOSHI_NAMESPACE_END
