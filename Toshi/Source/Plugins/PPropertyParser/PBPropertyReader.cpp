#include "ToshiPCH.h"
#include "Toshi/TPString8.h"
#include "PBProperties.h"
#include "PPropertyName.h"
#include "PPropertyValue.h"
#include "PPropertyValueArray.h"
#include "PProperty.h"

#include "T2Locale/T2Locale.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006b2360
PPropertyName* PBPropertyReader::CreatePropertyName( const PBPropertyName* a_pName )
{
	return new PPropertyName( TPString8( a_pName->GetString() ), TPString8( "" ) );
}

// $Barnyard: FUNCTION 006b2800
TBOOL PBPropertyReader::FillProperties( const PBProperties* a_pBProperties, PProperties* a_pProperties )
{
	for ( TUINT32 i = 0; i < a_pBProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pBProperty = a_pBProperties->GetPropertyByIndex( i );

		PPropertyValue* pValue = ToValue( pBProperty->GetValue() );
		PPropertyName*  pName  = CreatePropertyName( &pBProperty->GetName() );

		a_pProperties->AddPropertyNoCount( new PProperty( *pName, *pValue ) );

		if ( pValue )
			delete pValue;

		if ( pName )
			delete pName;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006b1ca0
PProperties* PBPropertyReader::CreateProperties( const PBProperties* a_pBProperties )
{
	PProperties* pProperties = new PProperties();
	FillProperties( a_pBProperties, pProperties );

	return pProperties;
}

// $Barnyard: FUNCTION 006b25d0
PPropertyValue* PBPropertyReader::ToValue( const PBPropertyValue* a_pValue )
{

	switch ( a_pValue->m_eType )
	{
		case PBPropertyValue::Type::Int:
			return new PPropertyValue( a_pValue->m_uValue.Int );
		case PBPropertyValue::Type::UInt32:
			return new PPropertyValue( a_pValue->m_uValue.UInt32 );
		case PBPropertyValue::Type::Float:
			return new PPropertyValue( a_pValue->m_uValue.Float );
		case PBPropertyValue::Type::Bool:
			// NOTE: retail tests the whole dword
			return new PPropertyValue( TBOOL( a_pValue->m_uValue.UInt32 != 0 ) );
		case PBPropertyValue::Type::Properties:
			return new PPropertyValue( CreateProperties( a_pValue->m_uValue.Properties ) );
		case PBPropertyValue::Type::PropertyName:
		{
			PPropertyName*  pName  = CreatePropertyName( TSTATICCAST( const PBPropertyName, a_pValue->m_uValue.Pointer ) );
			PPropertyValue* pValue = new PPropertyValue( *pName );

			if ( pName )
				delete pName;

			return pValue;
		}
		case PBPropertyValue::Type::Array:
			return new PPropertyValue( ToValueArray( a_pValue->m_uValue.Array ) );
		case PBPropertyValue::Type::String:
			return new PPropertyValue( a_pValue->GetTPString8() );
		case PBPropertyValue::Type::LocaleString:
			return new PPropertyValue( T2Locale::GetSingleton()->GetString( a_pValue->m_uValue.Int ), TFALSE );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006b2510
PPropertyValueArray* PBPropertyReader::ToValueArray( const PBPropertyValueArray* a_pArray )
{
	PPropertyValueArray* pArray = new PPropertyValueArray( a_pArray->m_iSize );

	for ( TUINT32 i = 0; i < a_pArray->m_iSize; i++ )
	{
		PPropertyValue* pValue = ToValue( &a_pArray->m_pValues[ i ] );
		pArray->Push( *pValue );

		if ( pValue )
			delete pValue;
	}

	return pArray;
}

// $Barnyard: FUNCTION 006b1ce0
const TWCHAR* PBPropertyValue::GetLocalisedString() const
{
	return T2Locale::GetSingleton()->GetString( m_uValue.UInt32 );
}

// $Barnyard: FUNCTION 006b21e0
TBOOL PBProperties::GetOptionalLocalisedString( const TWCHAR*& a_rOutValue, const TCHAR* a_szName ) const
{
	const PBPropertyValue* pValue = GetProperty( a_szName );

	if ( pValue && pValue->GetType() == PBPropertyValue::Type::LocaleString )
	{
		a_rOutValue = T2Locale::GetSingleton()->GetString( pValue->GetLocaleString().uiIndex );
		return TTRUE;
	}

	return TFALSE;
}
