#include "pch.h"
#include "AModLoader.h"
#include "AModEvents.h"
#include "ImGui/AImGui.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AModLoader );

static TString8 s_strModsBinariesDirectory = "Mods\\Native\\";

AModLoader::AModLoader()
{
	m_oCommandLine.Create( GetCommandLineA() );

	// Development: "-modsdir <dir>" loads the native mods from another folder (e.g. a build output)
	if ( m_oCommandLine.HasParameter( "-modsdir" ) )
	{
		s_strModsBinariesDirectory = m_oCommandLine.GetParameterValue( "-modsdir" );

		if ( s_strModsBinariesDirectory.Length() > 0 && s_strModsBinariesDirectory[ s_strModsBinariesDirectory.Length() - 1 ] != '\\' )
			s_strModsBinariesDirectory += "\\";
	}
}

AModLoader::~AModLoader()
{
	TASSERT( m_oMods.IsEmpty() );
}

TBOOL AModLoader::OnCreate()
{
#if defined( TRENDERINTERFACE_DX8 ) || defined( TRENDERINTERFACE_DX11 )
	// The overlay is created once the renderer is
	AImGui::Subscribe();
#endif

	LoadMods();

	return TTask::OnCreate();
}

TBOOL AModLoader::OnUpdate( TFLOAT a_fDeltaTime )
{
	T2_FOREACH( m_oMods, it )
	{
		TBOOL bResult = it->OnUpdate( a_fDeltaTime );
		TASSERT( bResult != TFALSE );
	}

	return TTRUE;
}

void AModLoader::OnDestroy()
{
	UnloadMods();

	AModEvents::Reset();

	TTask::OnDestroy();
}

AModInstance* AModLoader::FindMod( const TCHAR* a_szName )
{
	T2_FOREACH( m_oMods, it )
	{
		if ( TStringManager::String8Compare( it->GetName(), a_szName ) == 0 )
			return it;
	}

	return TNULL;
}

const TCHAR* AModLoader::GetModsDirectory()
{
	return "Mods\\";
}

const TCHAR* AModLoader::GetModsBinariesDirectory()
{
	return s_strModsBinariesDirectory;
}

void AModLoader::LoadMods()
{
	TINFO( "Loading mods from '%s'...\n", GetModsBinariesDirectory() );

	TString8         strSearchPath = TString8::VarArgs( "%s*.dll", GetModsBinariesDirectory() );
	WIN32_FIND_DATAA oFindData;
	HANDLE           hFind = FindFirstFileA( strSearchPath, &oFindData );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( !( oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
				LoadMod( TString8::VarArgs( "%s%s", GetModsBinariesDirectory(), oFindData.cFileName ) );

		} while ( FindNextFileA( hFind, &oFindData ) );

		FindClose( hFind );
	}

	TINFO( "Loaded %d mod(s)\n", m_oMods.Size() );

	T2_FOREACH( m_oMods, it )
	{
		it->OnAllModsLoaded();
	}
}

void AModLoader::LoadMod( const TCHAR* a_szPath )
{
	HMODULE hModule = LoadLibraryA( a_szPath );

	if ( !hModule )
	{
		TERROR( "%s: Couldn't load the library (error %u)\n", a_szPath, GetLastError() );
		return;
	}

	// NOTE: the library is never freed, even if the mod fails to initialise. It might have
	// installed hooks or registered callbacks before failing, and those would point to unloaded code.
	auto fnCreateModInstance = TREINTERPRETCAST( t_CreateModInstance, GetProcAddress( hModule, "CreateModInstance" ) );
	auto fnGetModName        = TREINTERPRETCAST( t_GetModName, GetProcAddress( hModule, "GetModName" ) );
	auto fnGetModVersion     = TREINTERPRETCAST( t_GetModVersion, GetProcAddress( hModule, "GetModVersion" ) );

	if ( !fnCreateModInstance )
	{
		TERROR( "%s: CreateModInstance is not exported\n", a_szPath );
		return;
	}

	AModInstance* pModInstance = fnCreateModInstance( &m_oCommandLine );

	if ( !pModInstance )
	{
		TERROR( "%s: CreateModInstance returned TNULL\n", a_szPath );
		return;
	}

	pModInstance->m_strName   = fnGetModName ? fnGetModName() : a_szPath;
	pModInstance->m_uiVersion = fnGetModVersion ? fnGetModVersion() : TVERSION( 1, 0 );

	TINFO( "Initialising '%s' (version %u.%u)\n", pModInstance->GetName(), pModInstance->m_uiVersion.Parts.Major, pModInstance->m_uiVersion.Parts.Minor );
	TLOGUP();

	if ( pModInstance->OnLoad() )
	{
		TINFO( "Successfully initialised\n" );
		m_oMods.PushBack( pModInstance );
	}
	else
	{
		TERROR( "Couldn't initialise\n" );
		delete pModInstance;
	}

	TLOGDOWN();
}

void AModLoader::UnloadMods()
{
	// NOTE: the libraries stay loaded, see AModLoader::LoadMod
	while ( !m_oMods.IsEmpty() )
	{
		AModInstance* pModInstance = m_oMods.PopBack();

		pModInstance->OnUnload();
		delete pModInstance;
	}
}

