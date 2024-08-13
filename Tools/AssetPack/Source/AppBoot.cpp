#include "pch.h"
#include "AAssetBuilder.h"
#include "AAssetUnpacker.h"

#include <Toshi/Toshi.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>
#include <ToshiTools/T2CommandLine.h>

#include <filesystem>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );
	TUtil::SetTPStringPool( new TPString8Pool( 1024, 0, &T2Allocator::s_GlobalAllocator, TNULL ) );

	T2CommandLine commandLine( GetCommandLineA() );
	auto		  strPackPath	= commandLine.GetParameterValue( "-p" );
	auto		  strUnpackFile = commandLine.GetParameterValue( "-u" );
	auto		  strOutPath	= commandLine.GetParameterValue( "-o" );
	auto		  strAssetName	= commandLine.GetParameterValue( "-asset", "pack" );
	auto		  bIsBtec		= commandLine.HasParameter( "-btec" );

	if ( strUnpackFile )
	{
		TString8 filepath = strUnpackFile;

		for ( TINT i = 0; i < filepath.Length(); i++ )
			if ( filepath[ i ] == '/' ) filepath[ i ] = '\\';

		TString8 inputFileName = filepath.GetString( filepath.FindReverse( '\\', -1 ) + 1 );
		inputFileName.Truncate( inputFileName.FindReverse( '.', -1 ) );

		TString8 outputDir = filepath;
		outputDir.Truncate( outputDir.FindReverse( '\\', -1 ) );
		outputDir += "\\";
		outputDir += inputFileName;
		CreateDirectoryA( outputDir, NULL );

		AAssetPack assetPack;

		if ( assetPack.Load( filepath ) )
		{
			AAssetUnpacker::Unpack( assetPack, outputDir, bIsBtec );
		}
	}
	else if ( strPackPath )
	{
		TString8 filepath = strPackPath;

		for ( TINT i = 0; i < filepath.Length(); i++ )
			if ( filepath[ i ] == '/' ) filepath[ i ] = '\\';

		TString8 inputFileName = filepath.GetString( filepath.FindReverse( '\\', -1 ) + 1 );

		TINT iDotPos = inputFileName.FindReverse( '.', -1 );
		if ( iDotPos > 0 ) inputFileName.Truncate( inputFileName.FindReverse( '.', -1 ) );

		AAssetBuilder assetBuilder;

		for ( const auto& entry : std::filesystem::directory_iterator( filepath.GetString() ) )
		{
			auto wcsPath = entry.path().native().c_str();

			char szPath[ MAX_PATH ];
			TStringManager::StringUnicodeToChar( szPath, entry.path().native().c_str(), -1 );

			assetBuilder.Add( szPath );
		}

		TString8 outPath;
		TString8 assetName;

		if ( strOutPath )
		{
			outPath = TString8::VarArgs( "%s\\%s.trb", strOutPath, strAssetName );
		}
		else
		{
			outPath = TString8::VarArgs( "%s.trb", strAssetName );
		}

		assetBuilder.Save( outPath, bIsBtec );
	}

	return 0;
}
