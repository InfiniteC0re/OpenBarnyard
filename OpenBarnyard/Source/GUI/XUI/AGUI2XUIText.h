#pragma once
#ifdef BARNYARD_LATEST

#  include "AGUI2XUIElement.h"
#  include "AGUI2XUITextBox.h"

#  include <GUI/T2GUIFont.h>
#  include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// RTTI: AGUI2XUIText : AGUI2XUIElement
//-----------------------------------------------------------------------------
class AGUI2XUIText
    : public AGUI2XUIElement
{
public:
	TDECLARE_CLASS( AGUI2XUIText, AGUI2XUIElement );

	// Bits of the XUI TextStyle property
	using TEXTSTYLE = TUINT32;
	enum TEXTSTYLE_ : TEXTSTYLE
	{
		TEXTSTYLE_NONE       = 0,
		TEXTSTYLE_DROPSHADOW = BITFLAG( 0 ),
		TEXTSTYLE_NOWRAP     = BITFLAG( 4 ),
		TEXTSTYLE_LEFT       = BITFLAG( 8 ),
		TEXTSTYLE_RIGHT      = BITFLAG( 9 ),
		TEXTSTYLE_CENTER     = BITFLAG( 10 ),
	};

public:
	AGUI2XUIText();
	~AGUI2XUIText();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;
	virtual void SetWidth( TFLOAT a_fWidth ) override;

	//-----------------------------------------------------------------------------
	// AGUI2XUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override;

	static const TCHAR*  GetPCFont( TINT a_iPointSize, TFLOAT& a_rOutScale );
	static const TWCHAR* FindLocaleString( const TWCHAR* a_wszText );

	// Callbacks of the text box that draw the controller button icons of the text (characters 0xB1 to 0xFE)
	static TBOOL  RenderIcon( Toshi::T2GUIRenderer* a_pRenderer, TINT a_iChar, TFLOAT* a_pOutAdvance, TFLOAT a_fLineHeight, TFLOAT a_fX, TFLOAT a_fY );
	static TFLOAT GetIconWidth( TINT a_iChar );

public:
	// A controller button icon of wii_icons128.tga
	struct Icon
	{
		const TCHAR*    szName;
		TUINT16         uiChar;
		TFLOAT          fWidth;
		TFLOAT          fHeight;
		TFLOAT          fScaleX;
		TFLOAT          fScaleY;
		Toshi::TVector2 vecUV1;
		Toshi::TVector2 vecUV2;
	};

	// Font of the texts that don't set one, "Arial_14" (set by AGUI2::OnCreate)
	static TCHAR ms_szDefaultFont[ 48 ];

	// Material of wii_icons128.tga (created by the loading code, destroyed by AGUI2::UnloadXUIMaterialLibraries)
	inline static Toshi::T2GUIMaterial* ms_pIconMaterial = TNULL;

	static Icon ms_aIcons[ 11 ];

private:
	Toshi::T2GUIFont* m_pFont;    // 0x98
	AGUI2XUITextBox   m_oTextBox; // 0x9C
	TBOOL             m_bWrap;    // 0x120
};

#endif // BARNYARD_LATEST
