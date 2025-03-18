#include "pch.h"

#include <Toshi/Toshi.h>
#include <Core/TMemoryInitialiser.h>

#include <ToshiTools/T2CommandLine.h>

#include <XML2PProperties.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );
	TUtil::SetTPStringPool( new TPString8Pool( 1024, 0, GetGlobalAllocator(), TNULL ) );

	T2CommandLine commandLine( GetCommandLineA() );
	auto          strCompileFile   = commandLine.GetParameterValue( "-c" );
	auto          strDecompileFile = commandLine.GetParameterValue( "-d" );
	auto          strOutPath       = commandLine.GetParameterValue( "-o" );
	auto          bIsBtec          = commandLine.HasParameter( "-btec" );

	if ( strCompileFile )
	{
		auto pXMLFile = TFile::Create( strCompileFile );

		if ( pXMLFile )
		{
			auto  uiFileSize = pXMLFile->GetSize();
			char* pFileData  = new char[ uiFileSize + 1 ];
			pXMLFile->Read( pFileData, uiFileSize );
			pXMLFile->Destroy();
			pFileData[ uiFileSize ] = '\0';

			tinyxml2::XMLDocument doc;
			doc.Parse( pFileData );
			delete[] pFileData;

			PBProperties properties;
			XML2PProperties::XmlToProperties( doc.GetDocument()->FirstChildElement( "PProperties" ), properties );

			TString8 inFilepath = strCompileFile;
			TString8 outFilepath = strOutPath;

			for ( TINT i = 0; i < inFilepath.Length(); i++ )
			{
				if ( inFilepath[ i ] == '/' ) inFilepath[ i ] = '\\';
			}

			if ( !outFilepath )
			{
				TString8 inputFileDirName;
				inputFileDirName.Copy( inFilepath, inFilepath.FindReverse( '\\', -1 ) + 1 );

				TString8 inputFileName = inFilepath.GetString( inFilepath.FindReverse( '\\', -1 ) + 1 );
				inputFileName.Truncate( inputFileName.FindReverse( '.', -1 ) );

				if ( inputFileDirName.Length() != 0 )
				{
					outFilepath = TString8::VarArgs( "%s%s.trb", inputFileDirName.GetString(), inputFileName.GetString() );
				}
				else
				{
					outFilepath = TString8::VarArgs( "%s.trb", inputFileName.GetString() );
				}
			}
			else
			{
				outFilepath = outFilepath;
			}

			PPropertiesWriter::WriteTRB( outFilepath, properties, bIsBtec );
		}
	}
	else if ( strDecompileFile )
	{
		TString8 inFilepath = strDecompileFile;
		TString8 outFilepath = strOutPath;

		for ( TINT i = 0; i < inFilepath.Length(); i++ )
		{
			if ( inFilepath[ i ] == '/' ) inFilepath[ i ] = '\\';
		}

		if ( !strOutPath )
		{
			TString8 inputFileDirName;
			inputFileDirName.Copy( inFilepath, inFilepath.FindReverse( '\\', -1 ) + 1 );

			TString8 inputFileName = inFilepath.GetString( inFilepath.FindReverse( '\\', -1 ) + 1 );
			inputFileName.Truncate( inputFileName.FindReverse( '.', -1 ) );

			if ( inputFileDirName.Length() != 0 )
			{
				outFilepath = TString8::VarArgs( "%s%s.xml", inputFileDirName.GetString(), inputFileName.GetString() );
			}
			else
			{
				outFilepath = TString8::VarArgs( "%s.xml", inputFileName.GetString() );
			}
		}
		else
		{
			outFilepath = strOutPath;
		}

		tinyxml2::XMLDocument doc;
		TBOOL bResult = XML2PProperties::PropertiesToXml( doc, inFilepath );

		if ( bResult )
			doc.SaveFile( outFilepath );
	}

	return 0;
}
