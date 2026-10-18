#include "ToshiPCH.h"
#include "TGUIFont.h"
#include "TGUIManager.h"
#include "TGUITextureFactory.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PProperty.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_INIT( TGUIFont );

TGUIDeserialisable::PropertySet* TGUIFont::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUIFont::ms_oPropertySetOrdered = TNULL;
TFLOAT                           TGUIFont::ms_fHorizScale         = 1.0f;
TFLOAT                           TGUIFont::ms_fVertScale          = 1.0f;
TGUIFont::BreakMode              TGUIFont::ms_eBreakMode          = TGUIFont::BreakMode_European;

TPString8 TGUIFont::str_name;
TPString8 TGUIFont::str_texture;
TPString8 TGUIFont::str_glyph;
TPString8 TGUIFont::str_xpos;
TPString8 TGUIFont::str_ypos;
TPString8 TGUIFont::str_width;
TPString8 TGUIFont::str_height;
TPString8 TGUIFont::str_kernleft;
TPString8 TGUIFont::str_kernright;
TPString8 TGUIFont::str_spacing;

// NOTE: JPOG fills two THashMap<TUINT16, TBOOL> (ms_oJpnNoPreBreakTable and ms_oJpnNoPostBreakTable)
// with these characters in InitialiseStatic, they're searched directly here
static constexpr TUINT16 s_aJpnNoPreBreakChars[] = {
	0x0021, 0x0029, 0x002c, 0x002e, 0x003a, 0x003b, 0x003f, 0x005d, 0x007d, 0xff0c, 0xff0e, 0xff1a, 0xff1b, 0xff1f, 0xff01, 0xff09,
	0x3015, 0xff3d, 0xff5d, 0x3009, 0x300b, 0x300d, 0x300f, 0x3011, 0xff61, 0xff63, 0xff64, 0xff65, 0xff66, 0xff67, 0xff68, 0xff69,
	0xff6a, 0xff6b, 0xff6c, 0xff6d, 0xff6e, 0xff6f, 0xff9e, 0xff9f, 0x3000, 0x3001, 0x3002, 0x30fb, 0x309b, 0x309c, 0x30fd, 0x30fe,
	0x309d, 0x309e, 0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308e, 0x3092, 0x30a1, 0x30a3, 0x30a5,
	0x30a7, 0x30a9, 0x30c3, 0x30e3, 0x30e5, 0x30e7, 0x30ee, 0x30f2, 0x30f5, 0x30f6
};

static constexpr TUINT16 s_aJpnNoPostBreakChars[] = {
	0x0028, 0x005b, 0x007b, 0xff08, 0x3014, 0xff3b, 0xff5b, 0x3008, 0x300a, 0x300c, 0x300e, 0x3010
};

TSTATICASSERT( TARRAYSIZE( s_aJpnNoPreBreakChars ) == 74 );
TSTATICASSERT( TARRAYSIZE( s_aJpnNoPostBreakChars ) == 12 );

// $JPOG: FUNCTION TGuiInterface.dll+0000b590
void TGUIFont::InitialiseClass()
{
	ms_oPropertySet        = new PropertySet( 4, 0 );
	ms_oPropertySetOrdered = new PropertySet( 4, 0 );

	str_name      = TPString8( "name" );
	str_texture   = TPString8( "texture" );
	str_glyph     = TPString8( "glyph" );
	str_xpos      = TPString8( "xpos" );
	str_ypos      = TPString8( "ypos" );
	str_width     = TPString8( "width" );
	str_height    = TPString8( "height" );
	str_kernleft  = TPString8( "kernleft" );
	str_kernright = TPString8( "kernright" );
	str_spacing   = TPString8( "spacing" );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000baa0
void TGUIFont::DeinitialiseClass()
{
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );
}

TBOOL TGUIFont::IsJapaneseNoPreBreak( TUINT16 a_uiCharacter )
{
	for ( TUINT16 uiChar : s_aJpnNoPreBreakChars )
		if ( uiChar == a_uiCharacter ) return TTRUE;

	return TFALSE;
}

TBOOL TGUIFont::IsJapaneseNoPostBreak( TUINT16 a_uiCharacter )
{
	for ( TUINT16 uiChar : s_aJpnNoPostBreakChars )
		if ( uiChar == a_uiCharacter ) return TTRUE;

	return TFALSE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000bb50
TGUIFont::TGUIFont()
    : m_pManager( TNULL )
    , m_iFontID( 0 )
    , m_aTextures( 10, 0 )
{
	CreateGlyphHash();
	m_iFontHeight = 0;
	m_iSpacing    = 1;
}

// NOTE: the font ID and the font height aren't copied
// $JPOG: FUNCTION TGuiInterface.dll+0000d980
TGUIFont::TGUIFont( const TGUIFont& a_rcOther )
    : TGUIDeserialisable()
    , TNodeList<TGUIFont>::TNode()
    , m_pManager( a_rcOther.m_pManager )
    , m_strName( a_rcOther.m_strName )
    , m_iFontID( 0 )
    , m_aTextures( a_rcOther.m_aTextures.GetGrowSize(), a_rcOther.m_aTextures.Size() )
{
	m_aTextures.SetSize( a_rcOther.m_aTextures.Size() );

	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
		m_aTextures[ i ] = a_rcOther.m_aTextures[ i ];

	CreateGlyphHash();
	m_iSpacing = a_rcOther.m_iSpacing;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original leaves the font height uninitialised
	m_iFontHeight = a_rcOther.m_iFontHeight;
#endif

	for ( auto it = a_rcOther.m_oGlyphs.Begin(); it != a_rcOther.m_oGlyphs.End(); it++ )
	{
		TGUIFontGlyph* pGlyph = TSTATICCAST( TGUIFontGlyph, it->Clone() );
		InsertGlyphHash( pGlyph->GetCharacter(), pGlyph );
		LinkGlyph( pGlyph );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000bbf0
TGUIFont::~TGUIFont()
{
	DeleteGlyphHash();
	m_oGlyphs.DeleteAll();
	m_aTextures.FreeData();
}

// $JPOG: FUNCTION TGuiInterface.dll+0000bd20
TBOOL TGUIFont::Deserialise( const PProperties* a_pProperties )
{
	if ( !TGUIDeserialisable::Deserialise( a_pProperties ) )
		return TFALSE;

	const PPropertyValue* pName = GetRequiredProperty( a_pProperties, str_name, PPropertyValue::TYPE_TPSTRING8 );
	if ( !pName ) return TFALSE;

	TPString8 strName = pName->GetTPString8();

	const PPropertyValue* pHeight = GetRequiredProperty( a_pProperties, str_height, PPropertyValue::TYPE_INT );
	if ( !pHeight ) return TFALSE;

	TINT iHeight = pHeight->GetInteger();

	const PPropertyValue* pSpacing = GetRequiredProperty( a_pProperties, str_spacing, PPropertyValue::TYPE_INT );
	if ( !pSpacing ) return TFALSE;

	TINT iSpacing = pSpacing->GetInteger();

	m_strName     = strName;
	m_iFontHeight = iHeight;
	m_iSpacing    = iSpacing;

	TGUIManager* pManager = TGUIManager::GetSingleton();
	auto&        rList    = const_cast<PProperties*>( a_pProperties )->GetProperties();

	// Textures
	auto             it        = rList.Begin();
	const PProperty* pProperty = a_pProperties->FindProperty( str_texture, it );

	while ( pProperty )
	{
		const PPropertyValue& rcValue = pProperty->GetValue();

		if ( rcValue.GetType() != PPropertyValue::TYPE_TPSTRING8 )
			TGUIManager::Error( "TGUIFont: Incorrect type of 'texture' property" );

		AddTexture( pManager->GetTextureFactory()->GetTextureID( rcValue.GetTPString8() ) );
		pProperty = a_pProperties->FindProperty( str_texture, it );
	}

	// Glyphs
	it        = rList.Begin();
	pProperty = a_pProperties->FindProperty( str_glyph, it );

	while ( pProperty )
	{
		const PPropertyValue& rcValue = pProperty->GetValue();
		TPString8             strChar = pProperty->GetName().GetSubName();
		TUINT16               uiChar;

		if ( !StringToChar( TString8( strChar.GetString() ), uiChar ) )
		{
			TGUIManager::Error( TString8( TString8( "TGUIFont: Unknown format for UNICODE character '" ) + strChar.GetString() ) + "'" );
		}
		else
		{
			if ( rcValue.GetType() != PPropertyValue::TYPE_ARRAY )
			{
				TGUIManager::Error( "TGUIFont: Incorrect type of 'glyph' property" );

#ifdef BARNYARD_COMMUNITY_PATCH
				// NOTE: the original reads the array anyway (a crash for other types)
				pProperty = a_pProperties->FindProperty( str_glyph, it );
				continue;
#endif
			}

			PPropertyValueArray* pArray = rcValue.GetArray();
			TGUIFontGlyph*       pGlyph = new TGUIFontGlyph( uiChar );

			if ( pGlyph->Deserialise( this, *pArray ) )
				AddGlyph( pGlyph );
			else
				delete pGlyph;
		}

		pProperty = a_pProperties->FindProperty( str_glyph, it );
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c150
TBOOL TGUIFont::Serialise( PProperties* a_pProperties )
{
	if ( !TGUIDeserialisable::Serialise( a_pProperties ) )
		return TFALSE;

	a_pProperties->PutProperty( str_name, PPropertyValue( m_strName ) );
	a_pProperties->PutProperty( str_height, PPropertyValue( m_iFontHeight ) );
	a_pProperties->PutProperty( str_spacing, PPropertyValue( m_iSpacing ) );

	TGUITextureFactory* pTextureFactory = TGUIManager::GetSingleton()->GetTextureFactory();

	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
	{
		TString8 strIndex;
		strIndex.Format( "%d", i + 1 );

		a_pProperties->PutProperty( str_texture, TPString8( strIndex.GetString() ), PPropertyValue( pTextureFactory->GetTextureName( m_aTextures[ i ] ) ) );
	}

	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); it++ )
	{
		TGUIFontGlyph*       pGlyph = TSTATICCAST( TGUIFontGlyph, it.Get() );
		PPropertyValueArray* pArray = new PPropertyValueArray();

		if ( pGlyph->Serialise( this, *pArray ) )
		{
			TString8 strChar;
			CharToString( pGlyph->GetCharacter(), strChar );

			a_pProperties->PutProperty( str_glyph, TPString8( strChar.GetString() ), PPropertyValue( pArray ) );
		}
		else
		{
			delete pArray;
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e260
TGUIDeserialisable::PropertySet* TGUIFont::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e250
TGUIDeserialisable::PropertySet* TGUIFont::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e220
TGUIFont* TGUIFont::Clone() const
{
	return new TGUIFont( *this );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e1c0
TBOOL TGUIFont::Create( const TPString8& a_rcName, TINT a_iFontHeight, TINT a_iSpacing )
{
	m_strName     = a_rcName;
	m_iFontHeight = a_iFontHeight;
	m_iSpacing    = a_iSpacing;
	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c410
void TGUIFont::AddGlyph( TGUIFontGlyph* a_pGlyph )
{
	if ( GetGlyph( a_pGlyph->GetCharacter() ) == TNULL )
	{
		AddTexture( a_pGlyph->GetTexture() );
		InsertGlyphHash( a_pGlyph->GetCharacter(), a_pGlyph );
		LinkGlyph( a_pGlyph );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c490
void TGUIFont::RemoveGlyph( TGUIFontGlyph* a_pGlyph )
{
	RemoveGlyphHash( a_pGlyph->GetCharacter() );
	m_oGlyphs.Remove( a_pGlyph );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c4e0
void TGUIFont::AddTexture( TINT16 a_iTexture )
{
	if ( FindTexture( a_iTexture ) == -1 )
		m_aTextures.Push( a_iTexture );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c650
void TGUIFont::RemoveTexture( TINT16 a_iTexture )
{
	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); )
	{
		TGUIFontGlyph* pGlyph = TSTATICCAST( TGUIFontGlyph, it.Get() );
		it++;

		if ( pGlyph->GetTexture() == a_iTexture )
		{
			RemoveGlyph( pGlyph );
			delete pGlyph;
		}
	}

	TINT iIndex = FindTexture( a_iTexture );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original shifts the array from index -1 when the texture isn't in it
	if ( iIndex == -1 )
		return;
#endif

	TINT iLast = m_aTextures.Size() - 1;

	for ( TINT i = iIndex; i < iLast; i++ )
		m_aTextures.At( i ) = m_aTextures.At( i + 1 );

	m_aTextures.SetSize( iLast );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c6f0
void TGUIFont::CleanupTextures()
{
	m_aTextures.FreeData();

	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); it++ )
	{
		TINT16 iTexture = it->GetTexture();

		if ( FindTexture( iTexture ) == -1 )
			m_aTextures.Push( iTexture );
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e180
TGUIFontGlyph* TGUIFont::GetGlyph( TUINT16 a_uiCharacter ) const
{
	GlyphHashNode* pNode = FindGlyphHashNode( a_uiCharacter );
	return pNode ? pNode->pGlyph : TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c9d0
TGUIFontGlyph* TGUIFont::GetGlyph( const TPString8& a_rcName ) const
{
	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); it++ )
	{
		if ( it->GetName() == a_rcName )
			return TSTATICCAST( TGUIFontGlyph, it.Get() );
	}

	return TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e130
TBOOL TGUIFont::IsCharacterDefined( TUINT16 a_uiCharacter ) const
{
	return FindGlyphHashNode( a_uiCharacter ) != TNULL;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c910
TUINT16 TGUIFont::GetLowestCharacter() const
{
	TUINT16 uiLowest = TUINT16( 0xFFFF );

	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); it++ )
		uiLowest = TMath::Min( uiLowest, TSTATICCAST( TGUIFontGlyph, it.Get() )->GetCharacter() );

	return uiLowest;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000c970
TUINT16 TGUIFont::GetHighestCharacter() const
{
	TUINT16 uiHighest = 0;

	for ( auto it = m_oGlyphs.Begin(); it != m_oGlyphs.End(); it++ )
		uiHighest = TMath::Max( uiHighest, TSTATICCAST( TGUIFontGlyph, it.Get() )->GetCharacter() );

	return uiHighest;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000ca00
void TGUIFont::GetStringDimensions( const TString16& a_rcString, TINT& a_rWidth, TINT& a_rHeight ) const
{
	a_rWidth  = 0;
	a_rHeight = m_iFontHeight;

	for ( TINT i = 0; i < a_rcString.Length(); i++ )
	{
		TINT iCharWidth  = 0;
		TINT iCharHeight = 0;
		GetCharacterDimensions( a_rcString[ i ], iCharWidth, iCharHeight );

		a_rWidth += m_iSpacing + iCharWidth;
	}
}

// Characters without a glyph use the glyph of U+2400 (symbol for null)
// $JPOG: FUNCTION TGuiInterface.dll+0000ca80
void TGUIFont::GetCharacterDimensions( TUINT16 a_uiCharacter, TINT& a_rWidth, TINT& a_rHeight ) const
{
	TGUIFontGlyph* pGlyph = GetGlyph( a_uiCharacter );

	if ( !pGlyph )
		pGlyph = GetGlyph( TUINT16( 0x2400 ) );

	if ( pGlyph )
	{
		a_rWidth  = pGlyph->GetKerningRight() + pGlyph->GetKerningLeft() + pGlyph->GetWidth();
		a_rHeight = pGlyph->GetHeight();
	}
	else
	{
		a_rWidth  = 0;
		a_rHeight = 0;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+0000cb40
void TGUIFont::CharToString( TUINT16 a_uiCharacter, TString8& a_rOutString )
{
	if ( a_uiCharacter <= 0x20 || a_uiCharacter >= 0x7F )
		a_rOutString.Format( "U+%04x", a_uiCharacter );
	else
		a_rOutString.Format( "%c", a_uiCharacter );
}

// $JPOG: FUNCTION TGuiInterface.dll+0000cb90
TBOOL TGUIFont::StringToChar( const TString8& a_rcString, TUINT16& a_rOutCharacter )
{
	if ( a_rcString.IsEmpty() )
		return TFALSE;

	if ( ( a_rcString[ 0 ] == 'U' || a_rcString[ 0 ] == 'u' ) && a_rcString[ 1 ] == '+' )
	{
		TUINT uiValue;

		if ( sscanf( a_rcString.GetString() + 2, "%x", &uiValue ) != 1 )
			return TFALSE;

		a_rOutCharacter = TUINT16( uiValue );
		return TTRUE;
	}

	if ( a_rcString[ 1 ] == '\0' )
	{
		// NOTE: the character is sign extended
		a_rOutCharacter = TUINT16( TINT16( a_rcString[ 0 ] ) );
		return TTRUE;
	}

	return TFALSE;
}

// NOTE: inlined in the constructors
void TGUIFont::CreateGlyphHash()
{
	m_iNumHashed  = 0;
	m_iNumBuckets = 17;
	m_ppBuckets   = new GlyphHashNode*[ m_iNumBuckets ];

	for ( TINT i = 0; i < m_iNumBuckets; i++ )
		m_ppBuckets[ i ] = TNULL;
}

// NOTE: inlined in the destructor
void TGUIFont::DeleteGlyphHash()
{
	if ( m_ppBuckets )
	{
		for ( TINT i = 0; i < m_iNumBuckets; i++ )
		{
			GlyphHashNode* pNode = m_ppBuckets[ i ];

			while ( pNode )
			{
				GlyphHashNode* pNext = pNode->pNext;
				delete pNode;
				pNode = pNext;
			}

			m_ppBuckets[ i ] = TNULL;
		}

		m_iNumHashed = 0;
		delete[] m_ppBuckets;
		m_ppBuckets = TNULL;
	}

	m_iNumBuckets = 0;
}

// NOTE: inlined in GetGlyph, IsCharacterDefined and GetCharacterDimensions
TGUIFont::GlyphHashNode* TGUIFont::FindGlyphHashNode( TUINT16 a_uiCharacter ) const
{
	for ( GlyphHashNode* pNode = m_ppBuckets[ a_uiCharacter % m_iNumBuckets ]; pNode; pNode = pNode->pNext )
	{
		if ( pNode->uiCharacter == a_uiCharacter )
			return pNode;
	}

	return TNULL;
}

// Adds the glyph at the end of its bucket or replaces the glyph of the character
// $JPOG: FUNCTION TGuiInterface.dll+0000e6f0
void TGUIFont::InsertGlyphHash( TUINT16 a_uiCharacter, TGUIFontGlyph* a_pGlyph )
{
	TINT           iBucket = a_uiCharacter % m_iNumBuckets;
	GlyphHashNode* pPrev   = TNULL;

	for ( GlyphHashNode* pNode = m_ppBuckets[ iBucket ]; pNode; pNode = pNode->pNext )
	{
		if ( pNode->uiCharacter == a_uiCharacter )
		{
			pNode->pGlyph = a_pGlyph;
			return;
		}

		pPrev = pNode;
	}

	GlyphHashNode* pNode = new GlyphHashNode;
	pNode->uiCharacter   = a_uiCharacter;
	pNode->pGlyph        = a_pGlyph;
	pNode->pNext         = TNULL;

	if ( pPrev )
		pPrev->pNext = pNode;
	else
		m_ppBuckets[ iBucket ] = pNode;

	m_iNumHashed++;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e670
TBOOL TGUIFont::RemoveGlyphHash( TUINT16 a_uiCharacter )
{
	TINT           iBucket = a_uiCharacter % m_iNumBuckets;
	GlyphHashNode* pPrev   = TNULL;

	for ( GlyphHashNode* pNode = m_ppBuckets[ iBucket ]; pNode; pNode = pNode->pNext )
	{
		if ( pNode->uiCharacter == a_uiCharacter )
		{
			if ( pPrev )
				pPrev->pNext = pNode->pNext;
			else
				m_ppBuckets[ iBucket ] = pNode->pNext;

			delete pNode;
			m_iNumHashed--;
			return TTRUE;
		}

		pPrev = pNode;
	}

	return TFALSE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e790
TINT TGUIFont::FindTexture( TINT16 a_iTexture ) const
{
	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
	{
		if ( m_aTextures[ i ] == a_iTexture )
			return i;
	}

	return -1;
}

// NOTE: inlined in AddGlyph and the copy constructor
void TGUIFont::LinkGlyph( TGUIFontGlyph* a_pGlyph )
{
	m_oGlyphs.InsertTail( a_pGlyph );
}

// $NicktoonsPS2: FUNCTION 003648f8
// $JPOG: FUNCTION TGuiInterface.dll+0000cc60
void TGUIFont::BreakString( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const
{
	// NOTE: Nicktoons only has the European line breaking
	if ( ms_eBreakMode == BreakMode_European )
		BreakStringEuropean( a_rcString, a_pText, a_eAlignment, a_rcColour, a_rWidth, a_rHeight, a_iMaxWidth, a_iMaxHeight );
	else if ( ms_eBreakMode == BreakMode_Japanese )
		BreakStringJapanese( a_rcString, a_pText, a_eAlignment, a_rcColour, a_rWidth, a_rHeight, a_iMaxWidth, a_iMaxHeight );
}

// NOTE: inlined in the BreakString functions
static void AddLineSegment( TGUIFormattedText* a_pText, const TString16& a_rcString, TINT a_iFirst, TINT a_iCount, TINT a_iWidth, TINT a_iHeight, const TGUIColour& a_rcColour, const TGUIFont* a_pFont, TGUIAlignment a_eAlignment )
{
	a_pText->InsertTail( new TGUIFormattedText::Segment( TUINT16( a_iWidth ), TUINT16( a_iHeight ), a_rcColour, a_pFont, a_rcString.Mid( a_iFirst, a_iCount ), a_eAlignment ) );
}

// Breaks the lines at the spaces and at the new lines. A line ends before the spaces of the word that doesn't fit, a
// word longer than the line isn't broken.
// NOTE: the lines after a new line start with the new line character (as in the original)
// NOTE: Nicktoons' font adds its line spacing to the height of each line
// $NicktoonsPS2: FUNCTION 00364918
// $JPOG: FUNCTION TGuiInterface.dll+0000ccf0
void TGUIFont::BreakStringEuropean( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const
{
	const TWCHAR* wszString = a_rcString.GetString();
	const TINT    iLength   = a_rcString.Length();

	TINT iLineHeight  = m_iFontHeight;
	TINT iLineStart   = 0;
	TINT iLineWidth   = 0;
	TINT iMaxWidth    = 0;
	TINT iTotalHeight = 0;
	TINT iPos         = 0;

	TGUIFontGlyph* pSpaceGlyph = GetGlyph( TUINT16( ' ' ) );

#ifdef BARNYARD_COMMUNITY_PATCH
	// The original reads the width of the space glyph without checking it exists
	const TINT iSpaceWidth = pSpaceGlyph ? pSpaceGlyph->GetWidth() : 0;
#else
	const TINT iSpaceWidth = pSpaceGlyph->GetWidth();
#endif

	if ( iLength > 0 )
	{
		do
		{
			// Skip the spaces before the word
			TINT iNumSpaces = 0;

			for ( ;; iNumSpaces++ )
			{
				const TWCHAR wcChar = wszString[ iPos ];

				if ( !iswspace( wcChar ) || wcChar == L'\n' || iPos == iLength )
					break;

				iPos++;
			}

			// Measure the word
			const TINT iWordStart = iPos;
			TINT       iWordChars = 0;
			TINT       iWordWidth = 0;
			TWCHAR     wcChar;

			while ( TTRUE )
			{
				wcChar = wszString[ iPos ];

				if ( iswspace( wcChar ) || wcChar == L'\n' || iPos == iLength )
					break;

				TINT iCharWidth  = 0;
				TINT iCharHeight = 0;
				GetCharacterDimensions( wcChar, iCharWidth, iCharHeight );

				iWordChars++;
				iWordWidth += m_iSpacing + iCharWidth;
				iPos++;
			}

			TINT iNewLineWidth = iWordWidth + iLineWidth + iNumSpaces * iSpaceWidth;

			if ( iLineWidth > 0 && iNewLineWidth > a_iMaxWidth && a_iMaxWidth >= 0 )
			{
				// The word doesn't fit, it starts a new line
				if ( a_pText )
					AddLineSegment( a_pText, a_rcString, iLineStart, iWordStart - iLineStart - iNumSpaces, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

				iTotalHeight += iLineHeight;
				iMaxWidth   = TMath::Max( iLineWidth, iMaxWidth );
				iLineHeight = m_iFontHeight;
				iLineWidth  = iWordWidth;
				iLineStart  = iWordStart;
				continue;
			}

			iLineWidth = iNewLineWidth;

			if ( wcChar == L'\n' )
			{
				if ( a_pText )
					AddLineSegment( a_pText, a_rcString, iLineStart, iWordChars + iWordStart - iLineStart, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

				iTotalHeight += iLineHeight;
				iMaxWidth   = TMath::Max( iLineWidth, iMaxWidth );
				iLineHeight = m_iFontHeight;
				iPos++;
				iLineWidth = 0;
				iLineStart = iWordChars + iWordStart;
			}
		} while ( iPos < iLength );
	}

	// The last line
	if ( iLineStart < iPos )
	{
		if ( a_pText )
			AddLineSegment( a_pText, a_rcString, iLineStart, iPos - iLineStart, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

		iTotalHeight += iLineHeight;
		iMaxWidth = TMath::Max( iLineWidth, iMaxWidth );
	}

	a_rWidth  = iMaxWidth;
	a_rHeight = iTotalHeight;
}

// Breaks the lines between any two characters (and at the new lines), but not before the characters that can't start a
// line nor after the characters that can't end one
// $JPOG: FUNCTION TGuiInterface.dll+0000d350
void TGUIFont::BreakStringJapanese( const TString16& a_rcString, TGUIFormattedText* a_pText, TGUIAlignment a_eAlignment, const TGUIColour& a_rcColour, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const
{
	const TWCHAR* wszString = a_rcString.GetString();
	const TINT    iLength   = a_rcString.Length();

	TINT iLineHeight  = m_iFontHeight;
	TINT iLineStart   = 0;
	TINT iLineWidth   = 0;
	TINT iMaxWidth    = 0;
	TINT iTotalHeight = 0;
	TINT iPos         = 0;
	TINT iCharWidth   = 0;
	TINT iCharHeight  = 0;

	if ( iLength > 0 )
	{
		while ( TTRUE )
		{
			TWCHAR wcChar = wszString[ iPos ];
			GetCharacterDimensions( wcChar, iCharWidth, iCharHeight );

			TINT iNewLineWidth = m_iSpacing + iLineWidth + iCharWidth;
			TINT iAdvance      = m_iSpacing + iCharWidth;

			if ( iNewLineWidth < a_iMaxWidth || a_iMaxWidth < iAdvance )
			{
				// The character fits (or doesn't fit even in an empty line)
				iLineWidth += iAdvance;

				if ( wcChar == L'\n' )
				{
					if ( a_pText )
						AddLineSegment( a_pText, a_rcString, iLineStart, iPos - iLineStart, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

					iTotalHeight += iLineHeight;
					iMaxWidth = TMath::Max( iLineWidth, iMaxWidth );
					iPos++;
					iLineHeight = m_iFontHeight;
					iLineWidth  = 0;
					iLineStart  = iPos;
				}
				else
				{
					iPos++;
				}
			}
			else if ( iPos <= iLineStart )
			{
				// The first character of the line is always kept
				iLineWidth += iAdvance;
				iPos++;
			}
			else
			{
				// Move the break back while the character can't start a line or the previous one can't end a line
				TBOOL bBreak = TTRUE;

				while ( TTRUE )
				{
					const TINT iPrev = iPos - 1;
					iLineWidth       = iNewLineWidth - iAdvance;

					if ( !( IsJapaneseNoPreBreak( wcChar ) && iPrev > iLineStart ) )
					{
						if ( iPrev <= iLineStart || !IsJapaneseNoPostBreak( wszString[ iPrev ] ) )
							break;
					}

					iPos--;
					wcChar = wszString[ iPrev ];
					GetCharacterDimensions( wcChar, iCharWidth, iCharHeight );

					if ( iPrev <= iLineStart )
					{
						// NOTE: the line can't be broken, the width of the character is lost (as in the original)
						iPos   = iPrev + 1;
						bBreak = TFALSE;
						break;
					}

					iNewLineWidth = iLineWidth;
					iAdvance      = m_iSpacing + iCharWidth;
				}

				if ( bBreak )
				{
					if ( a_pText )
						AddLineSegment( a_pText, a_rcString, iLineStart, iPos - iLineStart, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

					iTotalHeight += iLineHeight;
					iMaxWidth   = TMath::Max( iLineWidth, iMaxWidth );
					iLineHeight = m_iFontHeight;
					iLineWidth  = m_iSpacing + iCharWidth;
					iLineStart  = iPos++;
				}
			}

			if ( iPos >= iLength )
				break;
		}
	}

	// The last line
	if ( iLineStart < iPos )
	{
		if ( a_pText )
			AddLineSegment( a_pText, a_rcString, iLineStart, iPos - iLineStart, iLineWidth, iLineHeight, a_rcColour, this, a_eAlignment );

		iTotalHeight += iLineHeight;
		iMaxWidth = TMath::Max( iLineWidth, iMaxWidth );
	}

	a_rWidth  = iMaxWidth;
	a_rHeight = iTotalHeight;
}

TOSHI_NAMESPACE_END
