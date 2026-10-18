#include "ToshiPCH.h"
#include "PGUITRFontFactory.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIFont.h>
#include <TGUI/TGUIFontGlyph.h>
#include <TGUI/TGUITextureFactory.h>
#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( PGUITRFontFactory );

// $JPOG: FUNCTION PGUIRenderer.dll+00006180
PGUITRFontFactory::PGUITRFontFactory()
    : m_aFonts( 10, 0 )
    , m_pDisplayContext( TNULL )
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006150
// $JPOG: FUNCTION PGUIRenderer.dll+00005d10
PGUITRFontFactory::~PGUITRFontFactory()
{
	// NOTE: the fonts aren't deleted, the array only frees its buffer
}

// Returns the ID of the font a_rcName, adds the font if it isn't known yet
// $JPOG: FUNCTION PGUIRenderer.dll+00005e30
TINT PGUITRFontFactory::GetFontID( const TPString8& a_rcName )
{
	TINT iID = FindFontID( a_rcName );

	if ( iID == -1 )
	{
		Font* pFont = new Font( a_rcName );
		m_aFonts.Push( pFont );

		return m_aFonts.Size() - 1;
	}

	return iID;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005fb0
TINT PGUITRFontFactory::FindFontID( const TPString8& a_rcName )
{
	TINT iID = ( m_aFonts.Size() != 0 ) ? 0 : -1;

	if ( iID == -1 )
		return -1;

	while ( a_rcName != m_aFonts[ iID ]->GetName() )
	{
		if ( ++iID >= m_aFonts.Size() || iID == 0 )
			iID = -1;

		if ( iID == -1 )
			return -1;
	}

	return iID;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006130
const TPString8& PGUITRFontFactory::GetFontName( TINT a_iID )
{
	return m_aFonts[ a_iID ]->GetName();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006110
TBOOL PGUITRFontFactory::IsFontIDValid( TINT a_iID ) const
{
	return a_iID >= 0 && a_iID < m_aFonts.Size();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006140
void PGUITRFontFactory::Create( PGUITRDisplayContext* a_pDisplayContext )
{
	m_pDisplayContext = a_pDisplayContext;
}

// NOTE: Barnyard route added on top. The metrics follow T2GUIFont::DrawTextSingleLine: the line height is the font
// height, the y offset of a character its baseline, the paddings its kerning (the advance is the same, the left
// padding doesn't move the image of the character, the label path of the display context has no such offset).
TGUIFont* PGUITRFontFactory::CreateFontFromT2GUIFont( TGUIManager* a_pManager, const TPString8& a_rcName )
{
	TGUITextureFactory* pTextureFactory = a_pManager->GetTextureFactory();
	T2GUIFont*          pT2GUIFont      = pTextureFactory ? T2GUIFontManager::FindFont( a_rcName.GetString() ) : TNULL;

	if ( !pT2GUIFont )
		return TNULL;

	T2GUIFontDef* pFontDef = pT2GUIFont->GetFontDef();

	TGUIFont* pFont = new TGUIFont();
	pFont->Create( a_rcName, pFontDef->uiLineHeight, pFontDef->iLetterSpacing );

	// The textures of the materials of the font (the high byte of a character selects one if there are several)
	TINT16       aTextures[ 256 ];
	const TCHAR* szTextureName = pFontDef->szTextureNames;

	for ( TINT i = 0; i < pFontDef->uiNumMaterials; i++ )
	{
		aTextures[ i ] = pTextureFactory->GetTextureID( TPString8( szTextureName ) );
		szTextureName += TStringManager::String8Length( szTextureName ) + 1;
	}

	for ( TUINT uiChar = pFontDef->ui16MinCharacter; uiChar <= pFontDef->ui16MaxCharacter; uiChar++ )
	{
		TINT iCharIndex = pFontDef->pCharactersMap[ uiChar - pFontDef->ui16MinCharacter ];
		TINT iMaterial  = ( pFontDef->uiNumMaterials > 1 ) ? TINT( uiChar >> 8 ) : 0;

		if ( iCharIndex < 0 || iMaterial >= pFontDef->uiNumMaterials )
			continue;

		const T2GUIFontDefCharacterData& rCharData = pFontDef->pCharactersData[ iCharIndex ];

		pFont->AddGlyph( new TGUIFontGlyph(
		    aTextures[ iMaterial ],
		    TPString8(),
		    rCharData.ui16PosX,
		    rCharData.ui16PosY,
		    rCharData.ui8CharWidth,
		    rCharData.ui8CharHeight,
		    TUINT16( uiChar ),
		    rCharData.iYOffset,
		    rCharData.iLeftPadding,
		    rCharData.iRightPadding
		) );
	}

	T2GUIFontManager::ReleaseFont( pT2GUIFont );
	a_pManager->AddFont( pFont );

	return pFont;
}
