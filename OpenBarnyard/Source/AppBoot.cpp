#include "pch.h"
#include <Toshi/Toshi.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TRenderInterface.h>
#include <Render/TModelManager.h>

#include <Input/TInputInterface.h>

#ifdef TOSHI_SKU_WINDOWS
#  include <Platform/DX8/TTextureResourceHAL_DX8.h>
#  include <Platform/DX8/TVertexBlockResource_DX8.h>
#  include <Platform/DX8/TIndexBlockResource_DX8.h>
#endif // TOSHI_SKU_WINDOWS

#include "AppBoot.h"
#include "AToshiClassReferenceHelper.h"
#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AApplication g_oTheApp;

TOSHI_NAMESPACE_USING

#ifndef TOSHI_DIST
#  define TOSHI_ENTRY int main( int argc, TCHAR** argv )
#else
#  define TOSHI_ENTRY int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow )
#endif

static const TCHAR* GetOSName( OSVERSIONINFOEX& osVersionInfo )
{
	TBOOL isWorkstation = osVersionInfo.wProductType == VER_NT_WORKSTATION;

	if ( osVersionInfo.dwMajorVersion == 10 )
		return isWorkstation ? "Windows 10" : "Windows Server 2016";

	if ( osVersionInfo.dwMajorVersion == 6 )
	{
		if ( osVersionInfo.dwMinorVersion == 0 )
			return isWorkstation ? "Windows Vista" : "Windows Server 2008";

		if ( osVersionInfo.dwMinorVersion == 1 )
			return isWorkstation ? "Windows 7" : "Windows Server 2008 R2";

		if ( osVersionInfo.dwMinorVersion == 2 )
			return isWorkstation ? "Windows 8" : "Windows Server 2012";

		if ( osVersionInfo.dwMinorVersion == 3 )
			return isWorkstation ? "Windows 8.1" : "Windows Server 2012 R2";
	}

	return "unknown";
}

TOSHI_ENTRY
{
	static AToshiClassReferenceHelper s_ToshiClassReferencer;
	static TMemoryInitialiser         s_MemoryInitialiser;

	TCHAR* szCommandLine = GetCommandLineA();

	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = szCommandLine;
	toshiParams.szLogFileName = "barnyard";

	TUtil::ToshiCreate( toshiParams );

	g_oSystemManager.SetQuitCallback( []() {
		TUtil::ToshiDestroy();
		exit( 0 );
	} );

	AMemory::CreatePool( AMemory::POOL_StringPool );
	TUtil::Log( "Build Version %s\n", "0.28" );

	OSVERSIONINFOEX osVersionInfo = { sizeof( osVersionInfo ) };

	const TCHAR* osName = "unknown";
	HMODULE      ntdll  = GetModuleHandleW( L"ntdll.dll" );

	if ( ntdll != NULL )
	{
		typedef void( WINAPI * t_RtlGetVersion )( OSVERSIONINFOEX* );
		auto RtlGetVersion = TREINTERPRETCAST( t_RtlGetVersion, GetProcAddress( ntdll, "RtlGetVersion" ) );

		if ( RtlGetVersion != NULL )
		{
			RtlGetVersion( &osVersionInfo );
			osName = GetOSName( osVersionInfo );
		}
	}

	TUtil::Log( "Command Line: %s\n", szCommandLine );
	TUtil::Log( "OS Name: %s\n", osName );
	TUtil::Log( "OS Version: %d.%d Build:%d %s\n", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion, osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion );

	HANDLE hBarnyardMutex = CreateMutexA( NULL, TTRUE, "BARNYARD" );
	TUtil::SetGlobalMutex( hBarnyardMutex );

	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		MessageBoxA( NULL, "Game is already running", "Barnyard", MB_OK );
		return 0;
	}

	if ( g_oTheApp.Create( "The Barnyard - (c) Blue Tongue Entertainment", 0, 0 ) )
	{
		g_oTheApp.Execute();
	}

	TUtil::ToshiDestroy();

	return 0;
}

static TBOOL CreateStringPool()
{
	auto pStringPool = new TPString8Pool( 1024, 0, AMemory::GetAllocator( AMemory::POOL_StringPool ), TNULL );

	TUtil::SetTPStringPool( pStringPool );
	pStringPool->InitialiseStatic();

	return TTRUE;
}

TBOOL AApplication::OnCreate( int argc, TCHAR** argv )
{
	TINFO( "Starting Barnyard...\n" );

	// Create string pool before anything else
	if ( !CreateStringPool() )
		return TFALSE;

	ALocaleManager* pLocaleManager = T2Locale::CreateSingleton<ALocaleManager>();

	// Intialise free lists for rendering objects
	TPooledString8::SetupFreeList( 0, 0 ); // prevent the freelist from being used
	TTextureResourceHAL::SetupFreeList( 690, 0 );
	TVertexBlockResource::SetupFreeList( 500, 0 );
	TIndexBlockResource::SetupFreeList( 350, 0 );

	TTODO( "Create some instances of some classes" );

	TModelManager::Initialise();
	pLocaleManager->SetLanguage( ALocaleManager::Lang_EnglishUK );

	auto pScheduler = g_oSystemManager.GetScheduler();

	m_pRenderTask = pScheduler->CreateTask<ADummyTask>();
	m_pRenderTask->Create();
	m_pRenderTask->Activate( TTRUE );
	m_pRenderTask->SetName( "RenderTask" );

	m_pUpdate3Task = pScheduler->CreateTask<ADummyTask>();
	m_pUpdate3Task->Create();
	m_pUpdate3Task->Activate( TTRUE );
	m_pUpdate3Task->SetName( "Update3" );

	m_pUpdate2Task = pScheduler->CreateTask<ADummyTask>();
	m_pUpdate2Task->Create();
	m_pUpdate2Task->Activate( TTRUE );
	m_pUpdate2Task->SetName( "Update2" );

	m_pUpdate1Task = pScheduler->CreateTask<ADummyTask>();
	m_pUpdate1Task->Create();
	m_pUpdate1Task->Activate( TTRUE );
	m_pUpdate1Task->SetName( "Update1" );

	m_pInputTask = pScheduler->CreateTask<ADummyTask>();
	m_pInputTask->Create();
	m_pInputTask->Activate( TTRUE );
	m_pInputTask->SetName( "InputTask" );

	m_pRootTask = pScheduler->CreateTask<ARootTask>();

	if ( m_pRootTask != TNULL )
	{
		if ( TFALSE == m_pRootTask->Create() )
			return TFALSE;

		m_pRootTask->Activate( TTRUE );
	}

	return TApplication::OnCreate( argc, argv );
}

TBOOL AApplication::OnUpdate( TFLOAT deltaTime )
{
	return TTRUE;
}
