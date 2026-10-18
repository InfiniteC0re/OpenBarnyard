#include "ToshiPCH.h"
#include "TGUIFontGlyph.h"
#include "TGUIFont.h"
#include "TGUIManager.h"
#include "TGUITextureFactory.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $JPOG: FUNCTION TGuiInterface.dll+0000f010
TGUIFontGlyph::TGUIFontGlyph()
    : m_uiCharacter( 0 )
    , m_iBaseline( 0 )
    , m_iKerningLeft( 0 )
    , m_iKerningRight( 0 )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000efd0
TGUIFontGlyph::TGUIFontGlyph( TUINT16 a_uiCharacter )
    : m_uiCharacter( a_uiCharacter )
    , m_iBaseline( 0 )
    , m_iKerningLeft( 0 )
    , m_iKerningRight( 0 )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000eaa0
TGUIFontGlyph::TGUIFontGlyph( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TUINT16 a_uiCharacter, TINT16 a_iBaseline, TINT16 a_iKerningLeft, TINT16 a_iKerningRight )
    : TGUITextureSection( a_iTexture, a_rcName, a_iX, a_iY, a_iWidth, a_iHeight, 0, TFALSE, TFALSE, TFALSE )
    , m_uiCharacter( a_uiCharacter )
    , m_iBaseline( a_iBaseline )
    , m_iKerningLeft( a_iKerningLeft )
    , m_iKerningRight( a_iKerningRight )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000ef70
TGUIFontGlyph::TGUIFontGlyph( const TGUIFontGlyph& a_rcOther )
    : TGUITextureSection( a_rcOther )
    , m_uiCharacter( a_rcOther.m_uiCharacter )
    , m_iBaseline( a_rcOther.m_iBaseline )
    , m_iKerningLeft( a_rcOther.m_iKerningLeft )
    , m_iKerningRight( a_rcOther.m_iKerningRight )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000efc0
TGUIFontGlyph::~TGUIFontGlyph()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000e630
TGUIFontGlyph* TGUIFontGlyph::Clone() const
{
	return new TGUIFontGlyph( *this );
}

// Reads { name, texture, width, height, x, y[, baseline, kernleft, kernright] }, values of other types are ignored
// NOTE: JPOG gets the manager from the font (TGUIFont::GetInterface), it's the singleton here
// $JPOG: FUNCTION TGuiInterface.dll+0000eb10
TBOOL TGUIFontGlyph::Deserialise( TGUIFont* a_pFont, const PPropertyValueArray& a_rcArray )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	if ( a_rcArray.GetValue( 1 )->GetType() == PPropertyValue::TYPE_TPSTRING8 )
	{
		m_iTexture = pManager->FindTexture( a_rcArray.GetValue( 1 )->GetTPString8() );

		if ( m_iTexture == -1 )
		{
			TGUIManager::Error( TString8( "TGUIFont: Couldn't find texture named '" ) + a_rcArray.GetValue( 1 )->GetTPString8().GetString() + "'" );
			return TFALSE;
		}
	}

	if ( a_rcArray.GetValue( 0 )->GetType() == PPropertyValue::TYPE_TPSTRING8 )
		m_strName = a_rcArray.GetValue( 0 )->GetTPString8();

	if ( a_rcArray.GetValue( 2 )->GetType() == PPropertyValue::TYPE_INT )
		m_uiWidth = TUINT16( a_rcArray.GetValue( 2 )->GetInteger() );

	if ( a_rcArray.GetValue( 3 )->GetType() == PPropertyValue::TYPE_INT )
		m_uiHeight = TUINT16( a_rcArray.GetValue( 3 )->GetInteger() );

	if ( a_rcArray.GetValue( 4 )->GetType() == PPropertyValue::TYPE_INT )
		m_uiX = TUINT16( a_rcArray.GetValue( 4 )->GetInteger() );

	if ( a_rcArray.GetValue( 5 )->GetType() == PPropertyValue::TYPE_INT )
		m_uiY = TUINT16( a_rcArray.GetValue( 5 )->GetInteger() );

	if ( a_rcArray.GetSize() > 6 )
	{
		if ( a_rcArray.GetValue( 6 )->GetType() == PPropertyValue::TYPE_INT )
			m_iBaseline = TINT16( a_rcArray.GetValue( 6 )->GetInteger() );

		if ( a_rcArray.GetValue( 7 )->GetType() == PPropertyValue::TYPE_INT )
			m_iKerningLeft = TINT16( a_rcArray.GetValue( 7 )->GetInteger() );

		if ( a_rcArray.GetValue( 8 )->GetType() == PPropertyValue::TYPE_INT )
			m_iKerningRight = TINT16( a_rcArray.GetValue( 8 )->GetInteger() );
	}

	return TTRUE;
}

// NOTE: JPOG gets the manager from the font (TGUIFont::GetInterface), it's the singleton here
// $JPOG: FUNCTION TGuiInterface.dll+0000edb0
TBOOL TGUIFontGlyph::Serialise( TGUIFont* a_pFont, PPropertyValueArray& a_rArray )
{
	TGUITextureFactory* pTextureFactory = TGUIManager::GetSingleton()->GetTextureFactory();

	a_rArray.SetSize( 9 );
	*a_rArray.GetValue( 0 ) = PPropertyValue( m_strName );
	*a_rArray.GetValue( 1 ) = PPropertyValue( pTextureFactory->GetTextureName( m_iTexture ) );
	*a_rArray.GetValue( 2 ) = PPropertyValue( TINT( m_uiWidth ) );
	*a_rArray.GetValue( 3 ) = PPropertyValue( TINT( m_uiHeight ) );
	*a_rArray.GetValue( 4 ) = PPropertyValue( TINT( m_uiX ) );
	*a_rArray.GetValue( 5 ) = PPropertyValue( TINT( m_uiY ) );
	*a_rArray.GetValue( 6 ) = PPropertyValue( TINT( m_iBaseline ) );
	*a_rArray.GetValue( 7 ) = PPropertyValue( TINT( m_iKerningLeft ) );
	*a_rArray.GetValue( 8 ) = PPropertyValue( TINT( m_iKerningRight ) );

	return TTRUE;
}

TOSHI_NAMESPACE_END
