#include "ToshiPCH.h"
#include "TGUIElement.h"
#include "TGUIManager.h"
#include "TGUITextureSection.h"
#include "TGUIFont.h"
#include "TGUIDisplayContext.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TPString8 TGUIElement::str_type;
TPString8 TGUIElement::str_texture;
TPString8 TGUIElement::str_label;
TPString8 TGUIElement::str_solid;
TPString8 TGUIElement::str_name;
TPString8 TGUIElement::str_font;
TPString8 TGUIElement::str_colour;
TPString8 TGUIElement::str_alignx;
TPString8 TGUIElement::str_aligny;
TPString8 TGUIElement::str_fillx;
TPString8 TGUIElement::str_filly;

TPString8 TGUIElement::ms_saAlignXName[ TGUIAlignment_NUMOF ];
TPString8 TGUIElement::ms_saAlignYName[ TGUIAlignment_NUMOF ];
TPString8 TGUIElement::ms_saFillName[ Fill_NUMOF ];
TPString8 TGUIElement::ms_saElementTypeName[ ElementType_NUMOF ];

const TCHAR* const TGUIElement::ms_csaAlignXNameStr[ TGUIAlignment_NUMOF ]    = { "center", "left", "right" };
const TCHAR* const TGUIElement::ms_csaAlignYNameStr[ TGUIAlignment_NUMOF ]    = { "center", "top", "bottom" };
const TCHAR* const TGUIElement::ms_csaFillNameStr[ Fill_NUMOF ]               = { "none", "repeat", "stretch" };
const TCHAR* const TGUIElement::ms_csaElementTypeNameStr[ ElementType_NUMOF ] = { "undefined", "texture", "label", "solid" };

// $NicktoonsPS2: FUNCTION 0037cc68
// $JPOG: FUNCTION TGuiInterface.dll+000262a0
void TGUIElement::InitialiseStatic()
{
	str_type    = TPString8( "type" );
	str_texture = TPString8( "texture" );
	str_label   = TPString8( "label" );
	str_solid   = TPString8( "solid" );
	str_name    = TPString8( "name" );
	str_font    = TPString8( "font" );
	str_colour  = TPString8( "colour" );
	str_alignx  = TPString8( "alignx" );
	str_aligny  = TPString8( "aligny" );
	str_fillx   = TPString8( "fillx" );
	str_filly   = TPString8( "filly" );

	for ( TINT i = 0; i < TGUIAlignment_NUMOF; i++ )
		ms_saAlignXName[ i ] = TPString8( ms_csaAlignXNameStr[ i ] );

	for ( TINT i = 0; i < TGUIAlignment_NUMOF; i++ )
		ms_saAlignYName[ i ] = TPString8( ms_csaAlignYNameStr[ i ] );

	for ( TINT i = 0; i < Fill_NUMOF; i++ )
		ms_saFillName[ i ] = TPString8( ms_csaFillNameStr[ i ] );

	for ( TINT i = 0; i < ElementType_NUMOF; i++ )
		ms_saElementTypeName[ i ] = TPString8( ms_csaElementTypeNameStr[ i ] );
}

// $JPOG: FUNCTION TGuiInterface.dll+00028610
void TGUIElement::DeinitialiseStatic()
{
	for ( TINT i = 0; i < ElementType_NUMOF; i++ )
		ms_saElementTypeName[ i ] = TPString8();

	for ( TINT i = 0; i < Fill_NUMOF; i++ )
		ms_saFillName[ i ] = TPString8();

	for ( TINT i = 0; i < TGUIAlignment_NUMOF; i++ )
		ms_saAlignYName[ i ] = TPString8();

	for ( TINT i = 0; i < TGUIAlignment_NUMOF; i++ )
		ms_saAlignXName[ i ] = TPString8();

	str_filly   = TPString8();
	str_fillx   = TPString8();
	str_aligny  = TPString8();
	str_alignx  = TPString8();
	str_colour  = TPString8();
	str_font    = TPString8();
	str_name    = TPString8();
	str_solid   = TPString8();
	str_label   = TPString8();
	str_texture = TPString8();
	str_type    = TPString8();
}

// NOTE: the texture section/font and the label aren't initialised
// $NicktoonsPS2: FUNCTION 0037b4f8
// $JPOG: FUNCTION TGuiInterface.dll+00026270
TGUIElement::TGUIElement()
    : m_eType( ElementType_Undefined )
    , m_iParameter( 0 )
    , m_eAlignX( TGUIAlignment_Center )
    , m_eFillX( Fill_Repeat )
    , m_eAlignY( TGUIAlignment_Center )
    , m_eFillY( Fill_Repeat )
    , m_oColour()
    , m_pTextureSection( TNULL )
    , m_wszLabel( TNULL )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00028660
TGUIElement::TGUIElement( const TGUIElement& a_rcOther )
    : m_eType( ElementType_Undefined )
    , m_oColour()
    , m_pTextureSection( TNULL )
    , m_wszLabel( TNULL )
{
	Assign( a_rcOther );
}

// $NicktoonsPS2: FUNCTION 0037b538
// $JPOG: FUNCTION TGuiInterface.dll+00028630
TGUIElement::~TGUIElement()
{
	ChangeType( ElementType_Undefined );
}

// $NicktoonsPS2: FUNCTION 0037b590
// $JPOG: FUNCTION TGuiInterface.dll+00028640
TGUIElement& TGUIElement::operator=( const TGUIElement& a_rcOther )
{
	Assign( a_rcOther );
	return *this;
}

// $NicktoonsPS2: FUNCTION 0037b5b8
// $JPOG: FUNCTION TGuiInterface.dll+00027ea0
void TGUIElement::Assign( const TGUIElement& a_rcOther )
{
	ChangeType( a_rcOther.m_eType );

	m_iParameter = a_rcOther.m_iParameter;
	m_oColour    = a_rcOther.m_oColour;
	m_eAlignX    = a_rcOther.m_eAlignX;
	m_eAlignY    = a_rcOther.m_eAlignY;
	m_eFillX     = a_rcOther.m_eFillX;
	m_eFillY     = a_rcOther.m_eFillY;

	if ( m_eType == ElementType_Texture )
	{
		m_pTextureSection = a_rcOther.m_pTextureSection;
	}
	else if ( m_eType == ElementType_Label )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		SetLabel( a_rcOther.m_wszLabel );
#else
		// NOTE: Nicktoons sets the address of the label pointer of a_rcOther as the label
		SetLabel( TREINTERPRETCAST( const TWCHAR*, &a_rcOther.m_wszLabel ) );
#endif
		m_pFont = a_rcOther.m_pFont;
	}
}

// $JPOG: FUNCTION TGuiInterface.dll+00026c30
TGUIElement& TGUIElement::Reset()
{
	ChangeType( ElementType_Undefined );

	m_iParameter = 0;
	m_oColour    = TGUIColour();
	m_eAlignX    = TGUIAlignment_Center;
	m_eAlignY    = TGUIAlignment_Center;
	m_eFillX     = Fill_Repeat;
	m_eFillY     = Fill_Repeat;

	return *this;
}

// $JPOG: FUNCTION TGuiInterface.dll+00027490
TBOOL TGUIElement::IsDefined() const
{
	if ( m_eType == ElementType_Texture )
		return m_iParameter > 0 || m_pTextureSection != TNULL;

	if ( m_eType == ElementType_Label )
		return m_iParameter > 0 || m_pFont != TNULL;

	return TFALSE;
}

// NOTE: JPOG releases its label (a TLString) when the type changes from label, Nicktoons doesn't own it
// $NicktoonsPS2: FUNCTION 0037b718
// $JPOG: FUNCTION TGuiInterface.dll+00026c70
void TGUIElement::ChangeType( ElementType a_eType )
{
	if ( a_eType == m_eType )
		return;

	if ( a_eType == ElementType_Texture || a_eType == ElementType_Label )
		m_pTextureSection = TNULL;

	m_eType = a_eType;
}

// $NicktoonsPS2: FUNCTION 0037b748
// $JPOG: FUNCTION TGuiInterface.dll+000284d0
const TClass* TGUIElement::GetParameterType() const
{
	if ( m_eType == ElementType_Texture )
		return &TGetClass( TGUITextureSection );

	if ( m_eType == ElementType_Label )
		return PPropertyValue::TYPE_STRING16;

	return PPropertyValue::TYPE_UNDEF;
}

// $NicktoonsPS2: FUNCTION 0037b6e8
// $JPOG: FUNCTION TGuiInterface.dll+00028540
TGUIElement& TGUIElement::SetParameter( TINT a_iIndex )
{
	m_iParameter = a_iIndex + 1;
	return *this;
}

// $NicktoonsPS2: FUNCTION 0037b688
// $JPOG: FUNCTION TGuiInterface.dll+000285c0
TGUIElement& TGUIElement::SetLabel( const TWCHAR* a_wszLabel )
{
	ChangeType( ElementType_Label );
	m_wszLabel = a_wszLabel;
	return *this;
}

// NOTE: inlined in the functions that use it
const TGUITextureSection* TGUIElement::GetTextureSection( PPropertyValue* a_pParameters ) const
{
	if ( m_iParameter > 0 && a_pParameters[ m_iParameter - 1 ].IsDefined() )
		return TSTATICCAST( TGUITextureSection, a_pParameters[ m_iParameter - 1 ].GetTObject() );

	return m_pTextureSection;
}

// NOTE: inlined in the functions that use it
const TWCHAR* TGUIElement::GetLabel( PPropertyValue* a_pParameters ) const
{
	if ( m_iParameter > 0 && a_pParameters[ m_iParameter - 1 ].IsDefined() )
		return a_pParameters[ m_iParameter - 1 ].GetString16();

#ifdef BARNYARD_COMMUNITY_PATCH
	return m_wszLabel;
#else
	// NOTE: Nicktoons uses the address of the label pointer as the text (the labels of its styles come from parameters)
	return TREINTERPRETCAST( const TWCHAR*, &m_wszLabel );
#endif
}

// $NicktoonsPS2: FUNCTION 0037b790
// $JPOG: FUNCTION TGuiInterface.dll+000274d0
TBOOL TGUIElement::Deserialise( const PProperties* a_pProperties )
{
	const PPropertyValue* pValue = a_pProperties->GetProperty( str_colour );

	if ( pValue )
	{
		if ( pValue->IsType( PPropertyValue::TYPE_UINT32 ) )
			m_oColour = TGUIColour( pValue->GetUINT32(), TGUIColour::ARGB );
		else
			TGUIManager::Error( "TGUIElement: Property 'colour' has incorrect type" );
	}

	pValue = a_pProperties->GetProperty( str_alignx );

	if ( pValue )
	{
		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TINT i = 0;

			for ( ; i < TGUIAlignment_NUMOF; i++ )
			{
				if ( pValue->GetTPString8() == ms_saAlignXName[ i ] )
				{
					m_eAlignX = TGUIAlignment( i );
					break;
				}
			}

			if ( i == TGUIAlignment_NUMOF )
				TGUIManager::Error( "TGUIElement: Property 'alignx' has illegal value" );
		}
		else
		{
			TGUIManager::Error( "TGUIElement: Property 'alignx' has incorrect type" );
		}
	}

	pValue = a_pProperties->GetProperty( str_aligny );

	if ( pValue )
	{
		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TINT i = 0;

			for ( ; i < TGUIAlignment_NUMOF; i++ )
			{
				if ( pValue->GetTPString8() == ms_saAlignYName[ i ] )
				{
					m_eAlignY = TGUIAlignment( i );
					break;
				}
			}

			if ( i == TGUIAlignment_NUMOF )
				TGUIManager::Error( "TGUIElement: Property 'aligny' has illegal value" );
		}
		else
		{
			TGUIManager::Error( "TGUIElement: Property 'aligny' has incorrect type" );
		}
	}

	pValue = a_pProperties->GetProperty( str_fillx );

	if ( pValue )
	{
		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TINT i = 0;

			for ( ; i < Fill_NUMOF; i++ )
			{
				if ( pValue->GetTPString8() == ms_saFillName[ i ] )
				{
					m_eFillX = Fill( i );
					break;
				}
			}

			if ( i == Fill_NUMOF )
				TGUIManager::Error( "TGUIElement: Property 'fillx' has illegal value" );
		}
		else
		{
			TGUIManager::Error( "TGUIElement: Property 'fillx' has incorrect type" );
		}
	}

	pValue = a_pProperties->GetProperty( str_filly );

	if ( pValue )
	{
		if ( pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
		{
			TINT i = 0;

			for ( ; i < Fill_NUMOF; i++ )
			{
				if ( pValue->GetTPString8() == ms_saFillName[ i ] )
				{
					m_eFillY = Fill( i );
					break;
				}
			}

			if ( i == Fill_NUMOF )
				TGUIManager::Error( "TGUIElement: Property 'filly' has illegal value" );
		}
		else
		{
			TGUIManager::Error( "TGUIElement: Property 'filly' has incorrect type" );
		}
	}

	// The type block, an empty block keeps the defaults
	if ( const PPropertyValue* pTexture = a_pProperties->GetOptionalProperty( str_texture, PPropertyValue::TYPE_PROPS ) )
	{
		ChangeType( ElementType_Texture );

		if ( pTexture->GetProperties()->GetProperties().IsEmpty() )
			return TTRUE;

		return DeserialiseTexture( pTexture->GetProperties() );
	}

	if ( const PPropertyValue* pLabel = a_pProperties->GetOptionalProperty( str_label, PPropertyValue::TYPE_PROPS ) )
	{
		ChangeType( ElementType_Label );

		if ( pLabel->GetProperties()->GetProperties().IsEmpty() )
			return TTRUE;

		return DeserialiseLabel( pLabel->GetProperties() );
	}

	if ( const PPropertyValue* pSolid = a_pProperties->GetOptionalProperty( str_solid, PPropertyValue::TYPE_PROPS ) )
	{
		ChangeType( ElementType_Solid );

		if ( pSolid->GetProperties()->GetProperties().IsEmpty() )
			return TTRUE;

		return DeserialiseSolid( pSolid->GetProperties() );
	}

	TGUIManager::Error( "TGUIElement: Invalid element type!" );
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0037bcf8
// $JPOG: FUNCTION TGuiInterface.dll+00027990
TBOOL TGUIElement::DeserialiseTexture( const PProperties* a_pProperties )
{
	const PPropertyValue* pName = a_pProperties->GetProperty( str_name );

	if ( !pName )
	{
		TGUIManager::Error( "TGUIElement(Texture): Property 'name' not specified" );
		return TFALSE;
	}

	if ( !pName->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
	{
		TGUIManager::Error( "TGUIElement(Texture): Property 'name' has incorrect type" );
		return TFALSE;
	}

	TGUITextureSection* pTextureSection = TGUIManager::GetSingleton()->FindTextureSection( pName->GetTPString8() );

	if ( !pTextureSection )
	{
		TGUIManager::Error( TString8( "TGUIElement(Texture): Couldn't find texture section '" ) + pName->GetTPString8().GetString() + "'" );
		return TFALSE;
	}

	m_pTextureSection = pTextureSection;
	return TTRUE;
}

// NOTE: errors are reported but the element is accepted
// $NicktoonsPS2: FUNCTION 0037be38
// $JPOG: FUNCTION TGuiInterface.dll+00027ac0
TBOOL TGUIElement::DeserialiseLabel( const PProperties* a_pProperties )
{
	const PPropertyValue* pFont = a_pProperties->GetProperty( str_font );

	if ( !pFont )
	{
		TGUIManager::Error( "TGUIElement(Label): Property 'font' not specified" );
	}
	else if ( !pFont->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
	{
		TGUIManager::Error( "TGUIElement(Label): Property 'font' has incorrect type" );
	}
	else
	{
		TGUIFont* pFoundFont = TGUIManager::GetSingleton()->FindFont( pFont->GetTPString8() );

		// NOTE: Barnyard route added on top: the fonts no [font] block defines (Barnyard's fonts.trb)
		if ( !pFoundFont )
			pFoundFont = TGUIManager::GetSingleton()->FindFallbackFont( pFont->GetTPString8() );

		if ( pFoundFont )
		{
			m_pFont = pFoundFont;
		}
		else
		{
			TString8 strError;
			strError.Format( "TGUIElement(Label): Couldn't find font '%s'", pFont->GetTPString8().GetString() );
			TGUIManager::Error( strError );

			m_pFont = TNULL;
		}
	}

	const PPropertyValue* pLabel = a_pProperties->GetProperty( str_label );

	if ( pLabel )
	{
		if ( !pLabel->IsType( PPropertyValue::TYPE_STRING16 ) )
			TGUIManager::Error( "TGUIElement(Label): Property 'label' has incorrect type" );

		m_wszLabel = pLabel->GetString16();
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0037bfb8
// $JPOG: FUNCTION TGuiInterface.dll+000284c0
TBOOL TGUIElement::DeserialiseSolid( const PProperties* a_pProperties )
{
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0037bfc0
// $JPOG: FUNCTION TGuiInterface.dll+00027c60
TBOOL TGUIElement::Serialise( PProperties* a_pProperties )
{
	if ( m_oColour.GetARGB() != TUINT32( -1 ) )
		a_pProperties->PutProperty( str_colour, PPropertyValue( TUINT32( m_oColour.GetARGB() ) ) );

	if ( m_eAlignX != TGUIAlignment_Center )
		a_pProperties->PutProperty( str_alignx, PPropertyValue( ms_saAlignXName[ m_eAlignX ] ) );

	if ( m_eAlignY != TGUIAlignment_Center )
		a_pProperties->PutProperty( str_aligny, PPropertyValue( ms_saAlignYName[ m_eAlignY ] ) );

	if ( m_eFillX != Fill_Repeat )
		a_pProperties->PutProperty( str_fillx, PPropertyValue( ms_saFillName[ m_eFillX ] ) );

	if ( m_eFillY != Fill_Repeat )
		a_pProperties->PutProperty( str_filly, PPropertyValue( ms_saFillName[ m_eFillY ] ) );

	// The type block
	PProperties* pTypeProperties = new PProperties();
	a_pProperties->PutProperty( ms_saElementTypeName[ m_eType ], PPropertyValue( pTypeProperties ) );

	if ( m_eType == ElementType_Texture )
	{
		if ( m_iParameter == 0 && m_pTextureSection )
			pTypeProperties->PutProperty( str_name, PPropertyValue( m_pTextureSection->GetName() ) );
	}
	else if ( m_eType == ElementType_Label )
	{
		if ( m_pFont )
			pTypeProperties->PutProperty( str_font, PPropertyValue( m_pFont->GetName() ) );

		if ( m_iParameter == 0 )
			pTypeProperties->PutProperty( str_label, PPropertyValue( const_cast<TWCHAR*>( m_wszLabel ), TFALSE ) );
	}

	return TTRUE;
}

// NOTE: the size is unchanged for solid elements
// $NicktoonsPS2: FUNCTION 0037c2f0
// $JPOG: FUNCTION TGuiInterface.dll+00026cf0
void TGUIElement::GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight ) const
{
	if ( m_eType == ElementType_Texture )
	{
		const TGUITextureSection* pTextureSection = GetTextureSection( a_pParameters );

		if ( pTextureSection )
		{
			a_rWidth  = pTextureSection->GetWidth();
			a_rHeight = pTextureSection->GetHeight();
		}
		else
		{
			a_rWidth  = 0;
			a_rHeight = 0;
		}
	}
	else if ( m_eType == ElementType_Label )
	{
		if ( m_pFont )
		{
			// NOTE: Nicktoons' font measures the text with no maximum width (-1)
			const TWCHAR* wszLabel = GetLabel( a_pParameters );
			m_pFont->GetStringDimensions( TString16( wszLabel ? wszLabel : L"" ), a_rWidth, a_rHeight );
		}
		else
		{
			a_rWidth  = 0;
			a_rHeight = 0;
		}
	}
	else if ( m_eType == ElementType_Undefined )
	{
		a_rWidth  = 0;
		a_rHeight = 0;
	}
}

// $NicktoonsPS2: FUNCTION 0037c438
// $JPOG: FUNCTION TGuiInterface.dll+00026e40
void TGUIElement::FormatText( PPropertyValue* a_pParameters, TGUIFormattedText* a_pText, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const
{
	if ( m_eType != ElementType_Label )
		return;

	const TWCHAR* wszLabel = GetLabel( a_pParameters );

	if ( m_pFont )
	{
		m_pFont->BreakString( TString16( wszLabel ? wszLabel : L"" ), a_pText, m_eAlignX, m_oColour, a_rWidth, a_rHeight, a_iMaxWidth, a_iMaxHeight );

		// NOTE: JPOG sets it if the localised string is temporary, the labels aren't localised strings here
		if ( a_pText )
			a_pText->SetTemporaryString( TFALSE );
	}
}

// NOTE: solid elements aren't painted
// $NicktoonsPS2: FUNCTION 0037c550
// $JPOG: FUNCTION TGuiInterface.dll+00026f30
void TGUIElement::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle* a_pRect ) const
{
	if ( m_eType == ElementType_Texture )
	{
		const TGUITextureSection* pTextureSection = GetTextureSection( a_pParameters );

		if ( pTextureSection )
			PaintTextureSection( a_pDisplayContext, pTextureSection, a_pRect, m_oColour );
	}
	else if ( m_eType == ElementType_Label )
	{
		TGUIRectangle oRect    = *a_pRect;
		const TWCHAR* wszLabel = GetLabel( a_pParameters );

		if ( m_pFont )
		{
			m_pFont->GetStringDimensions( TString16( wszLabel ? wszLabel : L"" ), oRect.m_iWidth, oRect.m_iHeight );

			if ( m_eAlignX == TGUIAlignment_Right )
				oRect.m_iX = a_pRect->m_iX + a_pRect->m_iWidth - oRect.m_iWidth;
			else if ( m_eAlignX == TGUIAlignment_Left )
				oRect.m_iX = a_pRect->m_iX;
			else
				oRect.m_iX = a_pRect->m_iX + ( a_pRect->m_iWidth - oRect.m_iWidth ) / 2;

			if ( m_eAlignY == TGUIAlignment_Bottom )
				oRect.m_iY = a_pRect->m_iY + a_pRect->m_iHeight - oRect.m_iHeight;
			else if ( m_eAlignY == TGUIAlignment_Top )
				oRect.m_iY = a_pRect->m_iY;
			else
				oRect.m_iY = a_pRect->m_iY + ( a_pRect->m_iHeight - oRect.m_iHeight ) / 2;

			a_pDisplayContext->DrawLabel( &oRect, m_pFont, m_oColour, wszLabel );
		}
	}
}

// The texture sections are painted in the rectangle grown by a_pOffset, the labels get it as a clip rectangle
// $NicktoonsPS2: FUNCTION 0037c770
// $JPOG: FUNCTION TGuiInterface.dll+00027150
void TGUIElement::Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle* a_pRect, const TGUIRectangle* a_pOffset, const TGUIColour& a_rcColour ) const
{
	// Modulates the colour of the element
	TGUIColour oColour(
	    TUINT8( ( m_oColour.GetRed() * a_rcColour.GetRed() ) >> 8 ),
	    TUINT8( ( m_oColour.GetGreen() * a_rcColour.GetGreen() ) >> 8 ),
	    TUINT8( ( m_oColour.GetBlue() * a_rcColour.GetBlue() ) >> 8 ),
	    TUINT8( ( m_oColour.GetAlpha() * a_rcColour.GetAlpha() ) >> 8 )
	);

	if ( m_eType == ElementType_Texture )
	{
		const TGUITextureSection* pTextureSection = GetTextureSection( a_pParameters );

		if ( pTextureSection )
		{
			TGUIRectangle oRect(
			    a_pRect->m_iX + a_pOffset->m_iX,
			    a_pRect->m_iY + a_pOffset->m_iY,
			    a_pRect->m_iWidth + a_pOffset->m_iWidth,
			    a_pRect->m_iHeight + a_pOffset->m_iHeight
			);

			PaintTextureSection( a_pDisplayContext, pTextureSection, &oRect, oColour );
		}
	}
	else if ( m_eType == ElementType_Label )
	{
		TGUIRectangle oRect    = *a_pRect;
		const TWCHAR* wszLabel = GetLabel( a_pParameters );

		if ( m_pFont )
		{
			m_pFont->GetStringDimensions( TString16( wszLabel ? wszLabel : L"" ), oRect.m_iWidth, oRect.m_iHeight );

			if ( m_eAlignX == TGUIAlignment_Right )
				oRect.m_iX = a_pRect->m_iX + a_pRect->m_iWidth - oRect.m_iWidth;
			else if ( m_eAlignX == TGUIAlignment_Left )
				oRect.m_iX = a_pRect->m_iX;
			else
				oRect.m_iX = a_pRect->m_iX + ( a_pRect->m_iWidth - oRect.m_iWidth ) / 2;

			if ( m_eAlignY == TGUIAlignment_Bottom )
				oRect.m_iY = a_pRect->m_iY + a_pRect->m_iHeight - oRect.m_iHeight;
			else if ( m_eAlignY == TGUIAlignment_Top )
				oRect.m_iY = a_pRect->m_iY;
			else
				oRect.m_iY = a_pRect->m_iY + ( a_pRect->m_iHeight - oRect.m_iHeight ) / 2;

			a_pDisplayContext->DrawLabel( &oRect, m_pFont, oColour, wszLabel, a_pOffset );
		}
	}
}

// Fill none: the size of the texture section at the alignment of the element; repeat: the texture section is
// repeated from the alignment of the element; stretch: the texture section is stretched (TGUIAlignment_NUMOF)
// $NicktoonsPS2: FUNCTION 0037cad0
// $JPOG: FUNCTION TGuiInterface.dll+00027f50
void TGUIElement::PaintTextureSection( TGUIDisplayContext* a_pDisplayContext, const TGUITextureSection* a_pTextureSection, const TGUIRectangle* a_pRect, const TGUIColour& a_rcColour ) const
{
	TGUIRectangle oRect = *a_pRect;

	// Rotated by 90 or 270 degrees, the width and the height are swapped
	TINT iWidth;
	TINT iHeight;

	if ( a_pTextureSection->GetRotation() == 0 || a_pTextureSection->GetRotation() == 2 )
	{
		iWidth  = a_pTextureSection->GetWidth();
		iHeight = a_pTextureSection->GetHeight();
	}
	else
	{
		iWidth  = a_pTextureSection->GetHeight();
		iHeight = a_pTextureSection->GetWidth();
	}

	TGUIAlignment eAlignX;
	TGUIAlignment eAlignY;

	if ( m_eFillX == Fill_None )
	{
		eAlignX        = TGUIAlignment_Left;
		oRect.m_iWidth = iWidth;

		if ( m_eAlignX == TGUIAlignment_Right )
			oRect.m_iX = a_pRect->m_iX + a_pRect->m_iWidth - iWidth;
		else if ( m_eAlignX == TGUIAlignment_Left )
			oRect.m_iX = a_pRect->m_iX;
		else
			oRect.m_iX = a_pRect->m_iX + ( a_pRect->m_iWidth - iWidth ) / 2;
	}
	else
	{
		eAlignX = ( m_eFillX == Fill_Repeat ) ? m_eAlignX : TGUIAlignment_NUMOF;
	}

	if ( m_eFillY == Fill_None )
	{
		eAlignY         = TGUIAlignment_Left;
		oRect.m_iHeight = iHeight;

		if ( m_eAlignY == TGUIAlignment_Bottom )
			oRect.m_iY = a_pRect->m_iY + a_pRect->m_iHeight - iHeight;
		else if ( m_eAlignY == TGUIAlignment_Top )
			oRect.m_iY = a_pRect->m_iY;
		else
			oRect.m_iY = a_pRect->m_iY + ( a_pRect->m_iHeight - iHeight ) / 2;
	}
	else
	{
		eAlignY = ( m_eFillY == Fill_Repeat ) ? m_eAlignY : TGUIAlignment_NUMOF;
	}

	a_pDisplayContext->Imp_DrawTextureSection( a_pTextureSection, a_rcColour, oRect.m_iX, oRect.m_iY, oRect.m_iWidth, oRect.m_iHeight, eAlignX, eAlignY );
}

TOSHI_NAMESPACE_END
