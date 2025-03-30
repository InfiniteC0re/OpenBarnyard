#include "pch.h"

#include <Core/TMemoryInitialiser.h>

#include <Toshi/Toshi.h>
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>
#include <Toshi/T2String.h>
#include <Toshi/TArray.h>

#include <ToshiTools/T2CommandLine.h>
#include <T2Locale/T2Locale.h>
#include <Plugins/PTRB.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );

	T2CommandLine commandLine( GetCommandLineA() );
	auto          strOutPath     = commandLine.GetParameterValue( "-o" );
	auto          strExportFile  = commandLine.GetParameterValue( "-e" );
	auto          strCompileFile = commandLine.GetParameterValue( "-c" );

	if ( !strOutPath )
	{
		TERROR( "No output path specified!\n" );
		return 1;
	}

	if ( strCompileFile )
	{
		auto pTextFile = TFile::Create( strCompileFile );

		if ( pTextFile )
		{
			TINFO( "Opened file for input!\n" );
			TINFO( "Reading input file...\n" );

			PTRB outTrbFile;
			auto pOutStack = outTrbFile.GetSections()->CreateStream();

			// Read file data into a buffer
			auto  uiFileSize  = pTextFile->GetSize();
			char* pFileBuffer = new char[ uiFileSize + 1 ];

			pTextFile->Read( pFileBuffer, uiFileSize );
			pFileBuffer[ uiFileSize ] = '\0';
			pTextFile->Destroy();

			TArray<TString16> foundStrings;

			T2FormatString2048 localeString;
			TCHAR*             pFileStart  = TREINTERPRETCAST( TCHAR*, pFileBuffer );
			TCHAR*             pFileEnd    = TREINTERPRETCAST( TCHAR*, pFileBuffer + uiFileSize );
			TCHAR*             pFileCursor = TREINTERPRETCAST( TCHAR*, pFileBuffer );

			while ( pFileCursor < pFileEnd )
			{
				TINT iLocaleStringLength;
				pFileCursor += localeString.ParseLine( pFileCursor, -1, &iLocaleStringLength, TFALSE, TTRUE );

				foundStrings.Push( Platform_UTF8ToUnicode( localeString.Get() ) );
			}

			delete[] pFileBuffer;

			TINFO( "Parsed %d locale strings.\n", foundStrings.Size() );

			auto pLocaleStrings = pOutStack->Alloc<T2Locale::LocaleStrings>();

			pLocaleStrings->m_numstrings = foundStrings.Size();
			pOutStack->Alloc<T2LocalisedString>( &pLocaleStrings->Strings, foundStrings.Size() );

			TARRAY_FOREACH( foundStrings, str )
			{
				auto iIndex = str.Index();
				pOutStack->Alloc<TWCHAR>( &pLocaleStrings->Strings[ iIndex ], str->Length() + 1 );

				T2String16::Copy( pLocaleStrings->Strings[ iIndex ], str.Get() );
			}

			outTrbFile.GetSymbols()->Add( pOutStack, "LocaleStrings", pLocaleStrings.get() );
			outTrbFile.WriteToFile( strOutPath.GetString(), commandLine.HasParameter( "-btec" ) );
		}
		else
		{
			TERROR( "Couldn't open file for input!\n" );
		}
	}
	else if ( strExportFile )
	{
		TTRB inTrb;

		if ( TTRB::ERROR_OK == inTrb.Load( strExportFile ) )
		{
			auto pLocaleStrings = inTrb.CastSymbol<T2Locale::LocaleStrings>( "LocaleStrings" );

			if ( pLocaleStrings )
			{
				auto pOutFile = TFile::Create( strOutPath, TFILEMODE_CREATENEW | TFILEMODE_WRITE );

				if ( pOutFile )
				{
					for ( TINT i = 0; i < pLocaleStrings->m_numstrings; i++ )
					{
						auto                pString = pLocaleStrings->Strings[ i ];
						T2FormatWString2048 finalString;
						TUINT               iPos = 0;

						while ( *pString != L'\0' )
						{
							if ( *pString == L'\n' )
							{
								finalString[ iPos++ ] = '\\';
								finalString[ iPos++ ] = 'n';
							}
							else
							{
								finalString[ iPos++ ] = *pString;
							}

							pString++;
						}

						finalString[ iPos ] = '\0';

						TString8 utf8String = Platform_UnicodeToUTF8( finalString.Get() );
						pOutFile->CPrintf( "%s\n", utf8String.GetString() );
					}

					pOutFile->Destroy();
				}
				else
				{
					TERROR( "Couldn't open file for output!\n" );
				}
			}
			else
			{
				TERROR( "The specified TRB file doesn't contain any locale strings!\n" );
			}
		}
		else
		{
			TERROR( "Couldn't open TRB file to export from!\n" );
		}
	}

	return 0;
}
