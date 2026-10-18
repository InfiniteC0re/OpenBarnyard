#pragma once
#include <Toshi/TNodeList.h>
#include <Toshi/TPString8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: named rectangle of a GUI texture ("[texturesections]" blocks):
// texture ID (see TGUITextureFactory), position and size in the texture, a
// rotation (0-3) and wrap/mirror flags.
//-----------------------------------------------------------------------------
class TGUITextureSection
    : public TObject
    , public TNodeList<TGUITextureSection>::TNode
{
public:
	TDECLARE_CLASS( TGUITextureSection, TObject );

public:
	TGUITextureSection();
	TGUITextureSection( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY );
	TGUITextureSection( const TGUITextureSection& a_rcOther );
	~TGUITextureSection();

	virtual TGUITextureSection* Clone() const;

	// Bounding box at the time a_fTime (animations)
	// NOTE: declared before the frame version, Nicktoons has them in this order in the vtable
	virtual void GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TFLOAT a_fTime ) const;

	// Bounding box of the frame a_iFrame (0 is the texture section itself, animation frames start at 1)
	virtual void GetBBox( TUINT16& a_rX, TUINT16& a_rY, TUINT16& a_rWidth, TUINT16& a_rHeight, TINT a_iFrame ) const;

	virtual void SetBBox( TUINT16 a_uiX, TUINT16 a_uiY, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TINT a_iFrame );

	void Set( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TINT a_iRotation, TBOOL a_bWrap, TBOOL a_bMirrorX, TBOOL a_bMirrorY );
	void Assign( const TGUITextureSection& a_rcOther );

	TGUITextureSection& operator=( const TGUITextureSection& a_rcOther );
	TBOOL               operator==( const TGUITextureSection& a_rcOther ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00009f90
	TINT16 GetTexture() const { return m_iTexture; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009e40
	void SetTexture( TINT16 a_iTexture ) { m_iTexture = a_iTexture; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f80
	const TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009e00
	void SetName( const TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f70
	TINT GetX() const { return m_uiX; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f60
	TINT GetY() const { return m_uiY; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f50
	TINT GetWidth() const { return m_uiWidth; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f40
	TINT GetHeight() const { return m_uiHeight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009df0
	void SetX( TUINT16 a_uiX ) { m_uiX = a_uiX; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009de0
	void SetY( TUINT16 a_uiY ) { m_uiY = a_uiY; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009dd0
	void SetWidth( TUINT16 a_uiWidth ) { m_uiWidth = a_uiWidth; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009dc0
	void SetHeight( TUINT16 a_uiHeight ) { m_uiHeight = a_uiHeight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f30
	TINT GetRotation() const { return m_uiRotation; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009da0
	void SetRotation( TUINT16 a_uiRotation ) { m_uiRotation = a_uiRotation & 3; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f20
	TBOOL GetWrap() const { return m_bWrap; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009d80
	void SetWrap( TBOOL a_bWrap ) { m_bWrap = a_bWrap; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009f00
	TBOOL GetMirrorX() const { return m_bMirrorX; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009d60
	void SetMirrorX( TBOOL a_bMirrorX ) { m_bMirrorX = a_bMirrorX; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009ee0
	TBOOL GetMirrorY() const { return m_bMirrorY; }

	// $JPOG: FUNCTION TGuiInterface.dll+00009d40
	void SetMirrorY( TBOOL a_bMirrorY ) { m_bMirrorY = a_bMirrorY; }

protected:
	TINT16    m_iTexture; // 0x10 -1 if not set
	TPString8 m_strName;  // 0x14
	TUINT16   m_uiX;      // 0x18
	TUINT16   m_uiY;      // 0x1A
	TUINT16   m_uiWidth;  // 0x1C
	TUINT16   m_uiHeight; // 0x1E

	// 0x20
	TUINT32 m_uiRotation : 2;
	TUINT32 m_bWrap : 1;
	TUINT32 m_bMirrorX : 1;
	TUINT32 m_bMirrorY : 1;
};

TOSHI_NAMESPACE_END
