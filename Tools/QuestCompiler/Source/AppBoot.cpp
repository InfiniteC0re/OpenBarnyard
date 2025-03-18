#include "pch.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>
#include <T2Locale/T2Locale.h>
#include <Core/TMemoryInitialiser.h>
#include <ToshiTools/T2CommandLine.h>

#include <XML2PProperties.h>
#include <Plugins/PPropertyParser/PPropertiesWriter.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

//void TranslateDocument( tinyxml2::XMLElement* a_pCurrentElement )
//{
//	static TTRB*                    s_pLocaleTrb     = TNULL;
//	static T2Locale::LocaleStrings* s_pLocaleStrings = TNULL;
//
//	if ( s_pLocaleTrb == TNULL )
//	{
//		s_pLocaleTrb = new TTRB;
//
//		s_pLocaleTrb->Load( "C:\\Stuff\\Barnyard\\Game\\Data\\Locale\\eng-uk.trb" );
//		s_pLocaleStrings = s_pLocaleTrb->CastSymbol<T2Locale::LocaleStrings>( "LocaleStrings" );
//	}
//
//	static TPString8 s_strLocale = TPS8D( "locale" );
//
//	if ( TPString8( a_pCurrentElement->FindAttribute( "type" )->Value() ) == s_strLocale )
//	{
//		TINT iLocaleId = T2String8::StringToInt( a_pCurrentElement->Attribute( "value" ) );
//
//		static TCHAR s_Buffer[ 8192 ];
//		TStringManager::StringUnicodeToChar( s_Buffer, s_pLocaleStrings->Strings[ iLocaleId ], -1 );
//
//		a_pCurrentElement->SetAttribute( "value", s_Buffer );
//	}
//	else
//	{
//		auto pChildElement = a_pCurrentElement->FirstChildElement();
//
//		while ( pChildElement )
//		{
//			TranslateDocument( pChildElement );
//			pChildElement = pChildElement->NextSiblingElement();
//		}
//	}
//}

void JoinInteractionsRecurse( tinyxml2::XMLDocument* a_pInteractions, tinyxml2::XMLElement* a_pActivitySet, tinyxml2::XMLElement* a_pCurrentElement )
{
	if ( !T2String8::CompareNoCase( a_pCurrentElement->Name(), "activities" ) )
	{
		auto pElem        = a_pCurrentElement->FirstChildElement( "_elem" );
		auto pItemElement = pElem->FirstChildElement();

		while ( pItemElement )
		{
			TINT iActivityId = T2String8::StringToInt( pItemElement->Attribute( "value" ) );
			TASSERT( iActivityId < a_pActivitySet->ChildElementCount() );

			auto pActivityElement = a_pActivitySet->FirstChildElement();
			TINT iActivityCounter = 0;

			while ( pActivityElement )
			{
				if ( iActivityCounter == iActivityId ) break;

				pActivityElement = pActivityElement->NextSiblingElement();
				iActivityCounter++;
			}

			TASSERT( pActivityElement );
			a_pCurrentElement->InsertEndChild( pActivityElement->DeepClone( a_pInteractions ) );
			pItemElement = pItemElement->NextSiblingElement();
		}

		a_pCurrentElement->DeleteChild( pElem );
	}
	else
	{
		auto pChildElement = a_pCurrentElement->FirstChildElement();

		while ( pChildElement )
		{
			JoinInteractionsRecurse( a_pInteractions, a_pActivitySet, pChildElement );
			pChildElement = pChildElement->NextSiblingElement();
		}
	}
}

TINT iActivityId = 0;

void GetAllActivities( tinyxml2::XMLElement* a_pActivities, tinyxml2::XMLElement* a_pCurrentElement )
{
	if ( !T2String8::CompareNoCase( a_pCurrentElement->Name(), "activities" ) )
	{
		auto pElemArrayElement = a_pCurrentElement->GetDocument()->NewElement( "_elem" );
		pElemArrayElement->SetAttribute( "type", "array" );

		auto pItemElement = a_pCurrentElement->FirstChildElement();

		while ( pItemElement )
		{
			a_pActivities->InsertEndChild( pItemElement->DeepClone( a_pActivities->GetDocument() ) );
			pItemElement = pItemElement->NextSiblingElement();

			auto pItemElement = pElemArrayElement->InsertNewChildElement( "item" );
			pItemElement->SetAttribute( "type", "int" );
			pItemElement->SetAttribute( "value", iActivityId++ );
		}

		a_pCurrentElement->DeleteChildren();
		a_pCurrentElement->InsertEndChild( pElemArrayElement );
	}
	else
	{
		auto pChildElement = a_pCurrentElement->FirstChildElement();

		while ( pChildElement )
		{
			GetAllActivities( a_pActivities, pChildElement );
			pChildElement = pChildElement->NextSiblingElement();
		}
	}
}

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );
	TUtil::SetTPStringPool( new TPString8Pool( 1024, 0, GetGlobalAllocator(), TNULL ) );

	T2CommandLine commandLine( GetCommandLineA() );
	auto          strActivitySet  = commandLine.GetParameterValue( "-activityset" );
	auto          strInteractions = commandLine.GetParameterValue( "-interactions" );
	auto          strOutPath      = commandLine.GetParameterValue( "-o", "." );
	auto          bIsBtec         = commandLine.HasParameter( "-btec" );
	auto          bIsDecompile    = commandLine.HasParameter( "-d" );
	auto          bIsCompile      = commandLine.HasParameter( "-c" );

	if ( bIsCompile )
	{
		TString8 inInteractions = strInteractions;
		TString8 outFilepath = strOutPath;

		// Fix slashes for the path
		FixPathSlashes( inInteractions );

		// Make out directory from the input file if none is specified
		if ( !strOutPath )
			outFilepath.Copy( inInteractions, inInteractions.FindReverse( '\\', -1 ) + 1 );
		FixPathSlashes( outFilepath );

		// Read interaction list
		tinyxml2::XMLDocument interactionsDoc;
		{
			auto pXMLFile = TFile::Create( inInteractions );

			if ( pXMLFile )
			{
				auto  uiFileSize = pXMLFile->GetSize();
				char* pFileData  = new char[ uiFileSize + 1 ];
				pXMLFile->Read( pFileData, uiFileSize );
				pXMLFile->Destroy();
				pFileData[ uiFileSize ] = '\0';

				interactionsDoc.Parse( pFileData );
				delete[] pFileData;
			}
		}
		
		tinyxml2::XMLDocument outActivitySet;

		auto pOutActivitiesRoot     = XML2PProperties::CreatePropertiesXMLTemplate( outActivitySet, bIsBtec );
		auto pOutActivitySetElement = pOutActivitiesRoot->InsertNewChildElement( "activityset" );
		pOutActivitySetElement->SetAttribute( "type", "property" );

		GetAllActivities( pOutActivitySetElement, interactionsDoc.FirstChildElement() );

		PBProperties activitySetsProperties;
		XML2PProperties::XmlToProperties( outActivitySet.RootElement(), activitySetsProperties );

		PBProperties interactionsProperties;
		XML2PProperties::XmlToProperties( interactionsDoc.RootElement(), interactionsProperties );

		PPropertiesWriter::WriteTRB( TString8::VarArgs( "%s\\%s", outFilepath.GetString(), "AllInteractions.trz" ), interactionsProperties, bIsBtec );
		PPropertiesWriter::WriteTRB( TString8::VarArgs( "%s\\%s", outFilepath.GetString(), "ActivitySets1.trz" ), activitySetsProperties, bIsBtec );
	}
	else if ( bIsDecompile )
	{
		TString8 inActivitySet  = strActivitySet;
		TString8 inInteractions = strInteractions;
		TString8 outFilepath    = strOutPath;
		
		// Fix slashes for the paths
		FixPathSlashes( inActivitySet );
		FixPathSlashes( inInteractions );

		// Make out directory from the input file if none is specified
		if ( !strOutPath )
			outFilepath.Copy( inActivitySet, inActivitySet.FindReverse( '\\', -1 ) + 1 );
		FixPathSlashes( outFilepath );

		// Read activity set
		tinyxml2::XMLDocument activitySetDoc;
		XML2PProperties::PropertiesToXml( activitySetDoc, inActivitySet );

		// Read interaction list
		tinyxml2::XMLDocument interactionsDoc;
		XML2PProperties::PropertiesToXml( interactionsDoc, inInteractions );

		JoinInteractionsRecurse( &interactionsDoc, activitySetDoc.FirstChildElement()->FirstChildElement( "activityset" ), interactionsDoc.FirstChildElement() );
		interactionsDoc.SaveFile( "Interactions.xml" );
	}

	return 0;
}
