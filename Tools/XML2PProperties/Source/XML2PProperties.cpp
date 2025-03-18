#include "XML2PProperties.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

using json = nlohmann::json;

void XML2PProperties::JsonToProperties( json& a_rJSON, PBProperties& a_rProperties )
{
	for ( auto it = a_rJSON.begin(); it != a_rJSON.end(); it++ )
	{
		if ( it->is_string() )
		{
			a_rProperties.AddProperty( it.key().c_str(), it.value().get<std::string>().c_str() );
		}
		else if ( it->is_number_integer() )
		{
			a_rProperties.AddProperty( it.key().c_str(), it.value().get<TINT>() );
		}
		else if ( it->is_number_unsigned() )
		{
			a_rProperties.AddProperty( it.key().c_str(), it.value().get<TUINT>() );
		}
		else if ( it->is_number_float() )
		{
			a_rProperties.AddProperty( it.key().c_str(), it.value().get<TFLOAT>() );
		}
		else if ( it->is_array() )
		{
			auto propertyArray = a_rProperties.AddPropertyArray( it.key().c_str() )->GetValue()->GetArray();

			for ( auto arrIt = it->begin(); arrIt != it->end(); arrIt++ )
			{
				if ( arrIt->is_string() )
				{
					propertyArray->Add( arrIt.value().get<std::string>().c_str() );
				}
				else if ( arrIt->is_number_integer() )
				{
					propertyArray->Add( arrIt.value().get<TINT>() );
				}
				else if ( arrIt->is_number_unsigned() )
				{
					propertyArray->Add( arrIt.value().get<TUINT>() );
				}
				else if ( arrIt->is_number_float() )
				{
					propertyArray->Add( arrIt.value().get<TFLOAT>() );
				}
			}
		}
		else if ( it->is_object() )
		{
			auto pProperties = a_rProperties.AddProperties( it.key().c_str() )->GetValue()->GetProperties();
			JsonToProperties( *it, *pProperties );
		}
	}
}

static void PropertyValueToXml( tinyxml2::XMLElement* a_pRootElement, const PBPropertyValue* a_pValue )
{
	switch ( a_pValue->GetType() )
	{
		case PBPropertyValue::Type::String:
			a_pRootElement->SetAttribute( "type", "string" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetString() );
			break;
		case PBPropertyValue::Type::Float:
			a_pRootElement->SetAttribute( "type", "float" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetFloat() );
			break;
		case PBPropertyValue::Type::Bool:
			a_pRootElement->SetAttribute( "type", "bool" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetBoolean() );
			break;
		case PBPropertyValue::Type::Int:
			a_pRootElement->SetAttribute( "type", "int" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetInteger() );
			break;
		case PBPropertyValue::Type::UInt32:
			a_pRootElement->SetAttribute( "type", "uint" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetUINT32() );
			break;
		case PBPropertyValue::Type::LocaleString:
			a_pRootElement->SetAttribute( "type", "locale" );
			a_pRootElement->SetAttribute( "value", a_pValue->GetLocaleString().uiIndex );
			break;
		case PBPropertyValue::Type::Properties:
			a_pRootElement->SetAttribute( "type", "property" );
			XML2PProperties::PropertiesToXml( a_pRootElement, a_pValue->GetProperties() );
			break;
		case PBPropertyValue::Type::Array:
			a_pRootElement->SetAttribute( "type", "array" );
			auto pArray = a_pValue->GetArray();

			for ( TUINT i = 0; i < pArray->GetSize(); i++ )
			{
				auto pArrayElement = a_pRootElement->InsertNewChildElement( "item" );
				auto pArrayValue   = pArray->GetValue( i );

				PropertyValueToXml( pArrayElement, pArrayValue );
			}
			break;
	}
}

void XML2PProperties::PropertiesToXml( tinyxml2::XMLElement* a_pRootElement, const PBProperties* a_pProperties )
{
	T2_FOREACH( ( *a_pProperties ), it )
	{
		auto pName  = &it->GetName();
		auto pValue = it->GetValue();

		auto pElement = a_pRootElement->InsertNewChildElement( pName->GetString() );
		PropertyValueToXml( pElement, pValue );
	}
}

TBOOL XML2PProperties::PropertiesToXml( tinyxml2::XMLDocument& a_rOutDoc, const TCHAR* a_pchFileName )
{
	TTRB inTrb;
	if ( inTrb.Load( a_pchFileName ) == TTRB::ERROR_OK )
	{
		auto pProperties = PBProperties::LoadFromTRB( inTrb );
		auto pRootElement = CreatePropertiesXMLTemplate( a_rOutDoc, TFALSE );

		PropertiesToXml( pRootElement, pProperties );
		
		return TTRUE;
	}

	return TFALSE;
}

void XML2PProperties::XmlToProperties( tinyxml2::XMLElement* a_pRootElement, PBProperties& a_rProperties )
{
	static TPString8 s_strString   = TPS8D( "string" );
	static TPString8 s_strInt      = TPS8D( "int" );
	static TPString8 s_strUint     = TPS8D( "uint" );
	static TPString8 s_strBool     = TPS8D( "bool" );
	static TPString8 s_strFloat    = TPS8D( "float" );
	static TPString8 s_strProperty = TPS8D( "property" );
	static TPString8 s_strArray    = TPS8D( "array" );
	static TPString8 s_strLocale   = TPS8D( "locale" );

	for ( auto it = a_pRootElement->FirstChildElement(); it != TNULL; it = it->NextSiblingElement() )
	{
		auto typeAttr  = it->FindAttribute( "type" );
		auto valueAttr = it->FindAttribute( "value" );
		TVALIDPTR( typeAttr );

		TPString8 strType = TPString8( typeAttr->Value() );

		if ( strType == s_strString )
			a_rProperties.AddProperty( it->Name(), valueAttr->Value() );
		else if ( strType == s_strInt )
			a_rProperties.AddProperty( it->Name(), valueAttr->IntValue() );
		else if ( strType == s_strUint )
			a_rProperties.AddProperty( it->Name(), valueAttr->UnsignedValue() );
		else if ( strType == s_strLocale )
			a_rProperties.AddProperty( it->Name(), PBPropertyLocaleString( valueAttr->UnsignedValue() ) );
		else if ( strType == s_strFloat )
			a_rProperties.AddProperty( it->Name(), valueAttr->FloatValue() );
		else if ( strType == s_strBool )
			a_rProperties.AddPropertyBool( it->Name(), valueAttr->BoolValue() );
		else if ( strType == s_strArray )
		{
			auto propertyArray = a_rProperties.AddPropertyArray( it->Name() )->GetValue()->GetArray();

			for ( auto arrIt = it->FirstChildElement( "item" ); arrIt != TNULL; arrIt = arrIt->NextSiblingElement( "item" ) )
			{
				auto arrTypeAttr  = arrIt->FindAttribute( "type" );
				auto arrValueAttr = arrIt->FindAttribute( "value" );
				TVALIDPTR( arrTypeAttr );
				TVALIDPTR( arrValueAttr );

				TPString8 strItemType = TPString8( arrTypeAttr->Value() );

				if ( strItemType == s_strString )
					propertyArray->Add( arrValueAttr->Value() );
				else if ( strItemType == s_strInt )
					propertyArray->Add( arrValueAttr->IntValue() );
				else if ( strItemType == s_strUint )
					propertyArray->Add( arrValueAttr->UnsignedValue() );
				else if ( strItemType == s_strLocale )
					propertyArray->Add( PBPropertyLocaleString( arrValueAttr->UnsignedValue() ) );
				else if ( strItemType == s_strFloat )
					propertyArray->Add( arrValueAttr->FloatValue() );
				else if ( strItemType == s_strBool )
					propertyArray->Add( arrValueAttr->BoolValue() );
			}
		}
		else if ( strType == s_strProperty )
		{
			auto pProperties = a_rProperties.AddProperties( it->Name() )->GetValue()->GetProperties();
			XmlToProperties( it, *pProperties );
		}
	}
}

tinyxml2::XMLElement* XML2PProperties::CreatePropertiesXMLTemplate( tinyxml2::XMLDocument& a_rOutDoc, TBOOL a_bCompressed )
{
	a_rOutDoc.InsertEndChild( a_rOutDoc.NewDeclaration() );
	a_rOutDoc.InsertEndChild( a_rOutDoc.NewComment( "Decompiled by OpenToshi's XML2PProperties" ) );

	auto pRootElement = a_rOutDoc.NewElement( "PProperties" );
	pRootElement->SetAttribute( "Compressed", a_bCompressed );

	a_rOutDoc.InsertEndChild( pRootElement );

	return pRootElement;
}
