#include "pch.h"

#include <Core/TMemoryInitialiser.h>

#include <Toshi/Toshi.h>
#include <Toshi/T2String.h>
#include <Toshi/TArray.h>
#include <ToshiTools/T2CommandLine.h>
#include <File/TTRB.h>

#include <Plugins/PPropertyParser/PBProperties.h>

TOSHI_NAMESPACE_USING

static TMemoryInitialiser s_MemoryInitialiser;

int main( int argc, char** argv )
{
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = GetCommandLineA();

	TUtil::ToshiCreate( toshiParams );

	T2CommandLine commandLine( GetCommandLineA() );

	// Look for soundbank files
	TFileSystem* pFileSystem         = TFileManager::GetSingleton()->FindFileSystem( "local" );
	TString8     strLookForDirectory = commandLine.GetParameterValue( "-i", "D:\\dev\\OpenBarnyard\\Game\\Data\\Sound\\" );
	TString8     strOutDir           = commandLine.GetParameterValue( "-o", "D:\\dev\\OpenBarnyard\\OpenBarnyard\\Source\\SoundBank" );

	TString8 strFileName;
	TBOOL    bHasFile = pFileSystem->GetFirstFile( strLookForDirectory, strFileName );

	while ( bHasFile )
	{
		if ( strFileName.EndsWithNoCase( ".trb" ) &&
		     strFileName.StartsWithNoCase( "soundbank" ) )
		{
			TTRACE( "Trying to open '%s' soundbank file\n", strFileName.GetString() );

			TString8 strFullFileName = strLookForDirectory;
			strFullFileName += strFileName;

			TTRB trb;
			if ( trb.Load( strFullFileName ) == TTRB::ERROR_OK )
			{
				const PBProperties* pProperties = PBProperties::LoadFromTRB( trb );

				if ( pProperties != TNULL )
				{
					const PBProperties* pSounds;
					if ( pProperties->GetOptionalPropertyValue( pSounds, "Sounds" ) )
					{
						TString8 strSoundBankName;
						pSounds->GetOptionalPropertyValue( strSoundBankName, "name" );

						static T2StringBuffer<1024 * 1024> s_SoundBankHeaderCode;
						s_SoundBankHeaderCode.Clear();

						s_SoundBankHeaderCode += "//-----------------------------------------------------------------------------\n";
						s_SoundBankHeaderCode.AppendFormat( "// Contains indices of various sounds stored in %s soundbank.\n", strSoundBankName.GetString() );
						s_SoundBankHeaderCode += "// Note: Automatically generated header file.\n";
						s_SoundBankHeaderCode += "//-----------------------------------------------------------------------------\n\n";

						s_SoundBankHeaderCode += "namespace soundbank {\n\n";
						s_SoundBankHeaderCode.AppendFormat( "enum %s {\n\n", strSoundBankName.GetString() );

						for ( auto it = pSounds->Begin(); it != pSounds->End(); it++ )
						{
							if ( it->GetName() != "Sound" )
								continue;

							auto pSound = it->GetValue()->GetProperties();

							TString8 strSoundName;
							pSound->GetOptionalPropertyValue( strSoundName, "name" );

							TString8 strCategoryName;
							pSound->GetOptionalPropertyValue( strCategoryName, "category" );

							TINT iSoundId;
							pSound->GetOptionalPropertyValue( iSoundId, "id" );

							s_SoundBankHeaderCode.AppendFormat( "\t// Category: %s\n", strCategoryName.GetString(), iSoundId );
							s_SoundBankHeaderCode.AppendFormat( "\t%s = %d,\n\n", strSoundName.GetString(), iSoundId );
						}

						s_SoundBankHeaderCode.AppendFormat( "}; // enum %s \n\n", strSoundBankName.GetString() );
						s_SoundBankHeaderCode += "} // namespace soundbank\n";

						TString8 strOutFullPath = TString8::VarArgs( "%s\\%s.h", strOutDir.GetString(), strSoundBankName.GetString() );
						TFile*   pOutFile       = TFile::Create( strOutFullPath, TFILEMODE_CREATENEW );

						pOutFile->Write( s_SoundBankHeaderCode.Get(), s_SoundBankHeaderCode.Length() );
						pOutFile->Destroy();
					}
					else
					{
						TTRACE( "Couldn't find Sounds property\n" );
					}
				}
				else
				{
					TTRACE( "Invalid file format - couldn't find PProperties\n" );
				}
			}
			else
			{
				TTRACE( "Couldn't load TRB file\n" );
			}
		}

		bHasFile = pFileSystem->GetNextFile( strFileName );
	}

	return 0;
}
