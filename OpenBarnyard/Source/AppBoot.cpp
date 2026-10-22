#include "pch.h"
#include <Toshi/Toshi.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TPString8.h>

#include <Core/TMemoryInitialiser.h>

#include <Render/TRenderInterface.h>
#include <Render/TModelManager.h>
#include <Render/TOrderTable.h>

#include <Input/TInputInterface.h>
#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PProperty.h>

#include "Render/TBackend.h"

#include "AppBoot.h"
#include "ClassLink.h"
#include "Tasks/ADummyTask.h"
#include "Locale/ALocaleManager.h"
#include "Locale/ALocale.h"
#include "GUI/AGUI2.h"
#include "CVar/AVarType.h"
#include "Tasks/ARootTask.h"
#include "Mod/AModLoader.h"
#include "Assets/AAssetLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Very important objects!!!
// NOTE: in the original, the memory isn't a static object. It's initialised by a C initialiser (.CRT$XI*, 006bfca0),
// before any C++ static object is constructed, and deinitialised by a terminator (.CRT$XT*, 006bfcb0) which only
// runs after all the static objects are destroyed (the DLL CRT never runs it). Static objects that free memory in their
// destructors (TClass, free lists of the engine...) rely on the memory being still there.
static int __cdecl InitialiseMemory()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: twice the 128 MB of retail, big levels (the merged world of the remaster) run out of it
	Toshi::TMemory::Initialise( 256 * 1024 * 1024, 0 );
#else
	Toshi::TMemory::Initialise( 0, 0 );
#endif
	return 0;
}

static void __cdecl DeinitialiseMemory()
{
	Toshi::TMemory::Deinitialise();
}

enum SYSTEMMESSAGE
{
	SYSTEMMESSAGE_GraphicsInitFailed,
	SYSTEMMESSAGE_NoVirtualMemory,
	SYSTEMMESSAGE_NotEnoughVirtualMemory,
	SYSTEMMESSAGE_ErrorCaption,
	SYSTEMMESSAGE_GraphicsInitFailed16Colours,
	SYSTEMMESSAGE_NUMOF
};

// 00772510: per language, only the first 7 languages have them
static const TCHAR* s_aSystemMessages[][ SYSTEMMESSAGE_NUMOF ] = {
	// English
	{
	    "Unable to initialize graphics system.\nConsult documentation.",
	    "Barnyard requires virtual memory to be enabled.\nEnable virtual memory and restart the program.",
	    "Barnyard requires 256MB of virtual memory.\nIncrease the amount of virtual memory and restart the program.",
	    "Error",
	    "Unable to initialize graphics system.\nCheck that the desktop colour depth is not set to 16 colors.",
	},
	// EnglishUK
	{
	    "Unable to initialize graphics system.\nConsult documentation.",
	    "Barnyard requires virtual memory to be enabled.\nEnable virtual memory and restart the program.",
	    "Barnyard requires 256MB of virtual memory.\nIncrease the amount of virtual memory and restart the program.",
	    "Error",
	    "Unable to initialize graphics system.\nCheck that the desktop colour depth is not set to 16 colors.",
	},
	// Japanese
	{
	    "Unable to initialize graphics system.\nConsult documentation.",
	    "Barnyard requires virtual memory to be enabled.\nEnable virtual memory and restart the program.",
	    "Barnyard requires 256MB of virtual memory.\nIncrease the amount of virtual memory and restart the program.",
	    "Error",
	    "Unable to initialize graphics system.\nCheck that the desktop colour depth is not set to 16 colors.",
	},
	// German
	{
	    "Das Grafiksystem konnte nicht initialisiert werden.\nBitte konsultieren Sie die entsprechende Dokumentation.",
	    "Barnyard ben\xF6"
	    "tigt virtuellen Speicher. \nAktivieren Sie virtuellen Speicher und starten Sie das Programm erneut.",
	    "Barnyard ben\xF6"
	    "tigt 256MB virtuellen Speicher.\nSorgen Sie f\xFC"
	    "r ausreichend virtuellen Speicher und starten Sie das Programm erneut.",
	    "Fehler",
	    "Die Grafikkarte kann nicht initialisiert werden.\nBitte stellen Sie sicher, dass die Farbtiefe nicht auf 16 Farben eingestellt ist.",
	},
	// Italian
	{
	    "Impossibile inizializzare il sistema di grafica.\nConsulta la documentazione.",
	    "Per installare Barnyard devi attivare la memoria virtuale.\nAttiva la memoria virtuale e riavvia il programma.",
	    "Barnyard richiede 256 MB di memoria virtuale.\nAumenta la memoria virtuale e riavvia il programma.",
	    "Errore",
	    "Impossibile inizializzare il sistema di grafica.\nControlla che la profondit\xE0"
	    " di colori sul desktop non sia impostata a 16 colori.",
	},
	// Spanish
	{
	    "Imposible iniciar sistema de gr\xE1"
	    "ficos.\nConsultar documentaci\xF3"
	    "n.",
	    "Barnyard necesita memoria virtual para activarse.\nActivar memoria virtual y reiniciar el programa.",
	    "Barnyard necesita 256 MB de memoria virtual.\nAumenta la cantidad de memoria virtual y reinicia el programa.",
	    "Error",
	    "Imposible iniciar sistema de gr\xE1"
	    "ficos.\nComprueba que la profundidad de color del escritorio no est\xE1"
	    " configurada para 16 colores.",
	},
	// French
	{
	    "Impossible d'initialiser le syst\xE8"
	    "me d'affichage.\nConsultez la documentation.",
	    "Vous devez activer la m\xE9"
	    "moire virtuelle pour installer Le P\xF4"
	    "le Express.\nActivez la m\xE9"
	    "moire virtuelle et red\xE9"
	    "marrez le programme.",
	    "Le P\xF4"
	    "le Express n\xE9"
	    "cessite 256 Mo de m\xE9"
	    "moire virtuelle.\nLib\xE9"
	    "rez de la m\xE9"
	    "moire virtuelle et red\xE9"
	    "marrez le programme.",
	    "Erreur",
	    "Impossible d'initialiser le syst\xE8"
	    "me d'affichage.\nAssurez-vous que la profondeur de couleur du bureau n'est pas r\xE9"
	    "gl\xE9"
	    "e sur 16 couleurs.",
	},
};

static TBOOL s_bShownSystemMessage = TFALSE;

// $Barnyard: FUNCTION 00403f80
static void ShowSystemMessage( SYSTEMMESSAGE a_eText, SYSTEMMESSAGE a_eCaption )
{
	if ( !s_bShownSystemMessage )
	{
		s_bShownSystemMessage = TTRUE;

		Toshi::T2Locale::Lang eLanguage = ALocaleManager::GetDefaultLanguage();
		MessageBoxA( NULL, s_aSystemMessages[ eLanguage ][ a_eText ], s_aSystemMessages[ eLanguage ][ a_eCaption ], MB_ICONWARNING );
	}
}

// NOTE: in the original, this is a C initialiser too (.CRT$XI*), run before the memory gets initialised
// $Barnyard: FUNCTION 00404240
static int __cdecl CheckVirtualMemory()
{
	MEMORYSTATUS oMemoryStatus;
	GlobalMemoryStatus( &oMemoryStatus );

	if ( oMemoryStatus.dwTotalPageFile == 0 )
	{
		ShowSystemMessage( SYSTEMMESSAGE_NoVirtualMemory, SYSTEMMESSAGE_ErrorCaption );
		exit( 0 );
	}

	if ( oMemoryStatus.dwAvailPageFile < 256 * 1024 * 1024 )
	{
		ShowSystemMessage( SYSTEMMESSAGE_NotEnoughVirtualMemory, SYSTEMMESSAGE_ErrorCaption );
		exit( 0 );
	}

	return 0;
}

#pragma section( ".CRT$XIT", long, read )
extern "C" __declspec( allocate( ".CRT$XIT" ) ) int( __cdecl* g_pfnCheckVirtualMemory )() = CheckVirtualMemory;
#pragma comment( linker, "/include:_g_pfnCheckVirtualMemory" )

#pragma section( ".CRT$XIU", long, read )
#pragma section( ".CRT$XTU", long, read )

extern "C" __declspec( allocate( ".CRT$XIU" ) ) int( __cdecl* g_pfnInitialiseMemory )()    = InitialiseMemory;
extern "C" __declspec( allocate( ".CRT$XTU" ) ) void( __cdecl* g_pfnDeinitialiseMemory )() = DeinitialiseMemory;

#pragma comment( linker, "/include:_g_pfnInitialiseMemory" )
#pragma comment( linker, "/include:_g_pfnDeinitialiseMemory" )

// Handle to the BARNYARD mutex
static HANDLE s_hBarnyardHandle;

MODAPI AApplication g_oTheApp;

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
	// NOTE: slightly different from the original, for more customization
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

// $Barnyard: FUNCTION 00403fd0
static TBOOL DeinitialiseClass( TClass* a_pClass, void* a_pUserData )
{
	if ( a_pClass->IsInitialized() )
		a_pClass->Deinitialize();

	return TTRUE;
}

// $Barnyard: FUNCTION 004027c0
static void DestroyLocaleManager()
{
	// The buffer is allocated by CreateLocaleManager, the locale manager doesn't own it
	void* pBuffer = ALocaleManager::Interface()->GetBuffer();
	ALocaleManager::DestroySingleton();
	TFree( pBuffer );
}

// $Barnyard: FUNCTION 004042c0
static void DestroyEngine()
{
	ReleaseMutex( s_hBarnyardHandle );

	if ( ARootTask::IsSingletonCreated() )
		ARootTask::GetSingleton()->Shutdown();

	// Kill every task and destroy them
	g_oSystemManager.GetScheduler()->DestroyAllTasks();

	// Deinitialise every class now, while the memory and the string pool still exist.
	// Otherwise TClass destructors do this during the static destruction (e.g. AFunctionTable::Unregister)
	TGetClass( TObject ).RecurseTree( TNULL, TNULL, DeinitialiseClass, TNULL );

	TModelManager::Uninitialise();

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the free list of the order tables is a static object whose destructor frees every node of it, but the nodes
	// are elements of one array. Release the static data now, so the list is empty when it gets destroyed
	TOrderTable::DestroyStaticData();
#endif // BARNYARD_COMMUNITY_PATCH

	AAssetLoader::CloseAll();
	g_oLocaleUnknownData.Clear();
	DestroyLocaleManager();
	AVarType::Destroy();

	// Destroy the string pool (releases the TPString8Initialiser strings and frees every pooled string)
	if ( TPString8Pool* pStringPool = TUtil::GetTPStringPool() )
		delete pStringPool;

	TUtil::SetTPStringPool( TNULL );

	TFreeList::DeinitialiseAll();
	AMemory::DestroyPool( AMemory::POOL_StringPool );

	TUtil::ToshiDestroy();
}

// $Barnyard: FUNCTION 00404360
static void EngineQuitCallback()
{
	DestroyEngine();
	exit( 0 );
}

// $Barnyard: FUNCTION 004044a0
TOSHI_ENTRY
{
	const TCHAR* pchCommandLine = GetCommandLineA();

	// Create core systems
	CreateEngine( pchCommandLine );

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( !T2String8::FindString( pchCommandLine, "-multiinstance" ) )
#endif
	{
		// Create main handle to make sure the game is not running
		s_hBarnyardHandle = CreateMutexA( NULL, TTRUE, "BARNYARD" );

		if ( GetLastError() == ERROR_ALREADY_EXISTS )
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			MessageBoxA( NULL, "Game is already running", "Barnyard", MB_OK );
#endif
			return 0;
		}
	}

	// NOTE: TFreeList::DumpAll (006c19c0) walks a list until its end and does nothing with it (a cut out debug check)
	TMemory::TestMemIntegrity( TNULL );
	ClassLink_Win::ReferenceClasses();

	// Split the command line into arguments
	TCHAR* aArgs[ 128 ];
	TINT   iNumArgs = 1;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original splits the buffer returned by GetCommandLineA in place, but the engine keeps a pointer to it
	// (the command line of TUtil), so split a copy of it instead
	TSIZE  uiCommandLineLength = T2String8::Length( pchCommandLine );
	TCHAR* pchArgs             = new TCHAR[ uiCommandLineLength + 1 ];
	T2String8::Copy( pchArgs, pchCommandLine, uiCommandLineLength + 1 );
#else
	TCHAR* pchArgs = GetCommandLineA();
#endif

	aArgs[ 0 ]     = pchArgs;
	TBOOL bInQuote = TFALSE;

	for ( TINT i = 0; pchArgs[ i ] != '\0'; i++ )
	{
		if ( pchArgs[ i ] == '"' )
			bInQuote = !bInQuote;

		if ( pchArgs[ i ] == ' ' && !bInQuote )
		{
			pchArgs[ i ]        = '\0';
			aArgs[ iNumArgs++ ] = &pchArgs[ i + 1 ];
		}
	}

	if ( g_oTheApp.Create( "The Barnyard - (c) Blue Tongue Entertainment", iNumArgs, aArgs ) )
		g_oTheApp.Execute();

#ifdef BARNYARD_COMMUNITY_PATCH
	delete[] pchArgs;
#endif

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

// $Barnyard: FUNCTION 00402740
static TBOOL CreateLocaleManager()
{
	T2Locale::CreateSingleton<ALocaleManager>();
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

	CreateLocaleManager();
	ALocaleManager* pLocaleManager = ALocaleManager::Interface();

	// Intialise free lists for rendering objects
	PProperties::SetupFreeList( 100, 0 );
	PProperty::SetupFreeList( 100, 0 );
	TPooledString8::SetupFreeList( 0, 0 ); // prevent the freelist from being used
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail can't go past these counts, big levels (the merged world of the remaster) need more
	TTextureResourceHAL::SetupFreeList( 690, 64 );
	TVertexBlockResource::SetupFreeList( 500, 64 );
	TIndexBlockResource::SetupFreeList( 350, 64 );
#else
	TTextureResourceHAL::SetupFreeList( 690, 0 );
	TVertexBlockResource::SetupFreeList( 500, 0 );
	TIndexBlockResource::SetupFreeList( 350, 0 );
#endif

	TModelManager::Initialise();
	pLocaleManager->SetLanguage(
#ifdef E3_2006_COMPATIBILITY
	    ALocaleManager::Lang_English
#else
	    ALocaleManager::Lang_EnglishUK
#endif
	);
	ALocaleUnknownData_SetUnk1( 0 );

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

#ifdef BARNYARD_COMMUNITY_PATCH
	// Load mods before the game systems are created, so they can hook them
	AModLoader* pModLoader = pScheduler->CreateTask<AModLoader>();
	pModLoader->Create();
	pModLoader->Activate( TTRUE );
#endif

	m_pRootTask = pScheduler->CreateTask<ARootTask>();

	if ( m_pRootTask != TNULL )
	{
		if ( TFALSE == m_pRootTask->Create() )
			return TFALSE;

		m_pRootTask->Activate( TTRUE );
	}

	return TApplication::OnCreate( argc, argv );
}

#ifdef BARNYARD_COMMUNITY_PATCH

//-----------------------------------------------------------------------------
// Frame rate limiter
//-----------------------------------------------------------------------------
// The original has no frame limiter: fullscreen Present waits for the vertical blank, but windowed mode runs
// uncapped, which changes everything that is tuned per frame.
// "-fpslimit <N>" caps the frame rate to N FPS (off by default, like retail).
//-----------------------------------------------------------------------------

static constexpr TFLOAT DEFAULT_FPS_LIMIT = 0.0f; // no limit

static TBOOL         s_bFrameLimiterInitialised = TFALSE;
static LARGE_INTEGER s_iPerfFrequency;
static LONGLONG      s_iFrameTicks    = 0; // 0 = no limit
static LONGLONG      s_iNextFrameTime = 0;
static HANDLE        s_hWaitTimer     = NULL;

static void FrameLimiter_Initialise()
{
	s_bFrameLimiterInitialised = TTRUE;

	QueryPerformanceFrequency( &s_iPerfFrequency );

	const TCHAR* pchCommandLine = GetCommandLineA();
	TFLOAT       fLimit         = DEFAULT_FPS_LIMIT;

	if ( const TCHAR* pchArg = TStringManager::String8FindString( pchCommandLine, "-fpslimit " ) )
		fLimit = TFLOAT( atof( pchArg + TStringManager::String8Length( "-fpslimit " ) ) );

	if ( fLimit > 0.0f )
	{
		s_iFrameTicks = LONGLONG( TFLOAT( s_iPerfFrequency.QuadPart ) / fLimit );

		// High resolution waitable timer (Windows 10 1803+); a normal one otherwise (~1 ms with timeBeginPeriod)
		s_hWaitTimer = CreateWaitableTimerExW( NULL, NULL, 0x00000002 /* CREATE_WAITABLE_TIMER_HIGH_RESOLUTION */, TIMER_ALL_ACCESS );
		if ( !s_hWaitTimer )
		{
			timeBeginPeriod( 1 );
			s_hWaitTimer = CreateWaitableTimerExW( NULL, NULL, 0, TIMER_ALL_ACCESS );
		}
	}

	TINFO( "Frame rate limit: %.1f FPS%s\n", fLimit > 0.0f ? fLimit : 0.0f, fLimit > 0.0f ? "" : " (uncapped)" );
}

static void FrameLimiter_Wait()
{
	LARGE_INTEGER iNow;
	QueryPerformanceCounter( &iNow );

	if ( s_iFrameTicks == 0 ) return;

	if ( s_iNextFrameTime == 0 || iNow.QuadPart - s_iNextFrameTime > s_iFrameTicks )
	{
		// First frame or more than a frame late (loading, a hitch): don't try to catch up
		s_iNextFrameTime = iNow.QuadPart + s_iFrameTicks;
		return;
	}

	// Sleep until about 1 ms before the deadline, then spin
	const LONGLONG iSpinTicks = s_iPerfFrequency.QuadPart / 1000;
	LONGLONG       iRemaining = s_iNextFrameTime - iNow.QuadPart;

	if ( s_hWaitTimer && iRemaining > iSpinTicks )
	{
		// Relative due time in 100 ns units
		LARGE_INTEGER iDueTime;
		iDueTime.QuadPart = -( ( iRemaining - iSpinTicks ) * 10000000 / s_iPerfFrequency.QuadPart );

		if ( SetWaitableTimer( s_hWaitTimer, &iDueTime, 0, NULL, NULL, FALSE ) )
			WaitForSingleObject( s_hWaitTimer, INFINITE );
	}

	do
	{
		YieldProcessor();
		QueryPerformanceCounter( &iNow );
	} while ( iNow.QuadPart < s_iNextFrameTime );

	s_iNextFrameTime += s_iFrameTicks;
}

TBOOL AApplication::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !s_bFrameLimiterInitialised )
		FrameLimiter_Initialise();

	FrameLimiter_Wait();

	return TApplication::OnUpdate( a_fDeltaTime );
}

#endif // BARNYARD_COMMUNITY_PATCH
