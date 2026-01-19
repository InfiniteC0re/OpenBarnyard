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
#include "ClassLink.h"
#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "GUI/AGUI2.h"
#include "CVar/AVarType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Very important objects!!!
static Toshi::TMemoryInitialiser s_oMemoryInitialiser;

// Handle to the BARNYARD mutex
static HANDLE s_hBarnyardHandle;

AApplication g_oTheApp;

TOSHI_NAMESPACE_USING

#ifdef TOSHI_FINAL
#  define TOSHI_ENTRY int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, INT cmdShow )
#else // TOSHI_FINAL
#  define TOSHI_ENTRY int main( int argc, TCHAR** argv )
#endif // !TOSHI_FINAL

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

static void EngineQuitCallback();
static void CreateEngine( const TCHAR* a_pchCommandLine )
{
	// Create core engine systems
	// NOTE: this is not exactly how it works in the original, I changed it a bit for more customization
	TUtil::TOSHIParams toshiParams;
	toshiParams.szCommandLine = a_pchCommandLine;
	toshiParams.szLogFileName = "barnyard";

	TUtil::ToshiCreate( toshiParams );
	g_oSystemManager.SetQuitCallback( EngineQuitCallback );

	AMemory::CreatePool( AMemory::POOL_StringPool );
	TUtil::Log( "Build Version %s\n", "0.28" );

#ifdef BARNYARD_COMMUNITY_PATCH
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

	TUtil::Log( "Command Line: %s\n", a_pchCommandLine );
	TUtil::Log( "OS Name: %s\n", osName );
	TUtil::Log( "OS Version: %d.%d Build:%d %s\n", osVersionInfo.dwMajorVersion, osVersionInfo.dwMinorVersion, osVersionInfo.dwBuildNumber, osVersionInfo.szCSDVersion );
#endif // BARNYARD_COMMUNITY_PATCH
}

// $Barnyard: FUNCTION 004042c0
static void DestroyEngine()
{
	ReleaseMutex( s_hBarnyardHandle );

	ALocaleManager::DestroySingleton();
	AVarType::Destroy();

	TUtil::ToshiDestroy();
}

// $Barnyard: FUNCTION 00404360
static void EngineQuitCallback()
{
	DestroyEngine();
	exit( 0 );
}

TOSHI_ENTRY
{
	const TCHAR* pchCommandLine = GetCommandLineA();

	// Create core systems
	CreateEngine( pchCommandLine );

	// Create main handle to make sure the game is not running
	s_hBarnyardHandle = CreateMutexA( NULL, TTRUE, "BARNYARD" );

	if ( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		MessageBoxA( NULL, "Game is already running", "Barnyard", MB_OK );
		return 0;
	}

	ClassLink_Win::ReferenceClasses();
	if ( g_oTheApp.Create( "The Barnyard - (c) Blue Tongue Entertainment", 0, 0 ) )
		g_oTheApp.Execute();

	DestroyEngine();
	return 0;
}

// $Barnyard: FUNCTION 00404390
static TBOOL CreateOptionalSystems()
{
	// Create pool for strings
	TPString8Pool* pStringPool = new TPString8Pool( 1024, 0, AMemory::GetAllocator( AMemory::POOL_StringPool ), TNULL );
	TUtil::SetTPStringPool( pStringPool );
	
	// Initialise CVar system, it uses the string pool
	AVarType::Create( pStringPool );

	return TTRUE;
}

// $Barnyard: FUNCTION 00404370
// $Barnyard: FUNCTION 00404010
AApplication::~AApplication()
{
}

// $Barnyard: FUNCTION 004045a0
TBOOL AApplication::OnCreate( int argc, TCHAR** argv )
{
	TINFO( "Starting Barnyard...\n" );

	// Initialise important systems
	if ( !CreateOptionalSystems() )
		return TFALSE;

	ALocaleManager* pLocaleManager = T2Locale::CreateSingleton<ALocaleManager>();

	// Intialise free lists for rendering objects
	TPooledString8::SetupFreeList( 0, 0 ); // prevent the freelist from being used
	TTextureResourceHAL::SetupFreeList( 690, 0 );
	TVertexBlockResource::SetupFreeList( 500, 0 );
	TIndexBlockResource::SetupFreeList( 350, 0 );

	TTODO( "Create some instances of some classes" );

	TModelManager::Initialise();
	pLocaleManager->SetLanguage(
#ifdef E3_2006_COMPATIBILITY
		ALocaleManager::Lang_English
#else
		ALocaleManager::Lang_EnglishUK
#endif
	);

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
