#pragma once
#include "PBProperties.h"
#include "Plugins/PTRB.h"
#include "File/TTRB.h"
#include "Toshi/TPString8.h"
#include "Toshi/T2Map.h"
#include "Toshi/T2String.h"

class PPropertiesWriter
{
private:
	inline static Toshi::T2Map<const TCHAR*, PTRBSections::MemoryStream::Ptr<TCHAR>, Toshi::T2String8Comparator> s_aStringMap;

private:
	static PTRBSections::MemoryStream::Ptr<TCHAR> GetStringPtr( const TCHAR* a_szString )
	{
		return *s_aStringMap.Find( a_szString );
	}

	static void DumpValue( PTRBSections::MemoryStream::Ptr<PBPropertyValue> a_ptrValue, const PBPropertyValue& a_OrigValue, PTRBSections::MemoryStream* a_pStack, PTRBSections::MemoryStream::Ptr<PBProperties>* a_pParentProperties )
	{
		a_ptrValue->m_eType = a_OrigValue.GetType();

		switch ( a_ptrValue->m_eType )
		{
			case PBPropertyValue::Type::Bool:
				a_ptrValue->m_uValue.Bool = a_OrigValue.GetBoolean();
				break;
			case PBPropertyValue::Type::Int:
				a_ptrValue->m_uValue.Int = a_OrigValue.GetInteger();
				break;
			case PBPropertyValue::Type::LocaleString:
				a_ptrValue->m_uValue.UInt32 = a_OrigValue.GetUINT32();
				break;
			case PBPropertyValue::Type::Float:
				a_ptrValue->m_uValue.Float = a_OrigValue.GetFloat();
				break;
			case PBPropertyValue::Type::String:
				a_pStack->WritePointer( &a_ptrValue->m_uValue.String, GetStringPtr( a_OrigValue.GetString() ) );
				break;
			case PBPropertyValue::Type::Properties:
			{
				auto properties = DumpProperties( *a_OrigValue.GetProperties(), a_pStack, a_pParentProperties );
				a_pStack->WritePointer( &a_ptrValue->m_uValue.Properties, properties );
				break;
			}
			case PBPropertyValue::Type::Array:
			{
				auto arraySize  = a_OrigValue.GetArray()->GetSize();
				auto pArray     = a_pStack->Alloc( &a_ptrValue->m_uValue.Array );
				auto pValues    = a_pStack->Alloc( &pArray->m_pValues, arraySize );
				pArray->m_iSize = arraySize;

				for ( size_t i = 0; i < arraySize; i++ )
				{
					DumpValue( pValues + i, *a_OrigValue.GetArray()->GetValue( i ), a_pStack, a_pParentProperties );
				}

				break;
			}
		}
	}

	static PTRBSections::MemoryStream::Ptr<PBProperties> DumpProperties( const PBProperties& a_oProperties, PTRBSections::MemoryStream* a_pStack, PTRBSections::MemoryStream::Ptr<PBProperties>* a_pParentProperties = TNULL )
	{
		auto properties = a_pStack->Alloc<PBProperties>();

		properties->m_iCount = a_oProperties.GetPropertyCount();
		auto props           = a_pStack->Alloc<PBProperties::PBProperty>( &properties->m_pProperties, properties->m_iCount );

		const TBOOL bIsRoot = a_oProperties.GetParentProperties() == TNULL;

		if ( bIsRoot )
		{
			// Write string table
			for ( auto it = s_aStringMap.Begin(); it != s_aStringMap.End(); it++ )
			{
				auto buffer = a_pStack->AllocBytes( Toshi::TStringManager::String8Length( it->GetFirst() ) + 1 );
				Toshi::TStringManager::String8Copy( buffer.get(), it->GetFirst() );

				it->GetSecond() = buffer;
			}
		}
		else
		{
			TASSERT( a_pParentProperties != TNULL );
			a_pStack->WritePointer( &properties->m_pParent, *a_pParentProperties );
		}

		for ( size_t i = 0; i < properties->m_iCount; i++ )
		{
			auto& origProp = a_oProperties.Begin()[ i ];

			auto prop      = props + i;
			auto propName  = a_pStack->Alloc<PBPropertyName>( &prop->m_pName );
			auto propValue = a_pStack->Alloc<PBPropertyValue>( &prop->m_pValue );

			a_pStack->WritePointer( &propName->m_szName, GetStringPtr( origProp.GetName().GetString() ) );
			DumpValue( propValue, *origProp.GetValue(), a_pStack, &properties );
		}

		return properties;
	}

	static void AddStringToMap( const TCHAR* a_szString )
	{
		if ( s_aStringMap.Find( a_szString ) == &s_aStringMap.End()->GetSecond() )
		{
			s_aStringMap.Insert( a_szString, {} );
		}
	}

	static void FillStringMap( const PBProperties& a_oProperties )
	{
		for ( auto it = a_oProperties.Begin(); it != a_oProperties.End(); it++ )
		{
			auto propName  = it->GetName().GetString();
			auto propValue = it->GetValue();
			auto propType  = propValue->GetType();

			AddStringToMap( propName );

			switch ( propType )
			{
				case PBPropertyValue::Type::Properties:
					FillStringMap( *propValue->GetProperties() );
					break;
				case PBPropertyValue::Type::String:
					AddStringToMap( propValue->GetString() );
					break;
				case PBPropertyValue::Type::Array:
					for ( size_t i = 0; i < propValue->GetArray()->GetSize(); i++ )
					{
						auto arrayValue = propValue->GetArray()->GetValue( i );

						if ( arrayValue->GetType() == PBPropertyValue::Type::String )
						{
							AddStringToMap( arrayValue->GetString() );
						}
					}
					break;
			}
		}
	}

public:
	static TBOOL WriteTRB( const Toshi::TString8& a_sFilePath, const PBProperties& a_oProperties, TBOOL a_bCompress = TFALSE )
	{
		TASSERT( a_oProperties.GetParentProperties() == TNULL );

		PTRBWriter trb;
		auto       sect = trb.GetSections();
		auto       symb = trb.GetSymbols();

		auto stack = sect->CreateStream();

		FillStringMap( a_oProperties );

		symb->Add( stack, "Main", DumpProperties( a_oProperties, stack ).get() );
		trb.WriteToFile( a_sFilePath.GetString(), a_bCompress );

		s_aStringMap.Clear();

		return TTRUE;
	}
};
