#include "PrimeDebugPCH.h"
#include "PDebugSystem.h"
#include "Console/PConsole.h"
#include "Console/PKeyboard.h"

#ifdef PRIMEDEBUG
#  ifdef PRIMEDEBUG_DEV
#	include "Console/PBindings.h"
#	include "Draw/PDebugRenderer.h"
#	include "Draw/PDebugText.h"
#	include "Draw/PDebugWindow.h"
#	include "Draw/PDebugPrimitives.h"
#	include "Menu/PDebugMenu.h"
#	include "Profile/PProfileCounter.h"
#	include "Profile/PStats.h"
#  endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#  ifdef PRIMEDEBUG_DEV
static PCVar s_oStatsVar;
#  endif

void PDebugSystem::Startup( PDebugRenderer* a_pRenderer )
{
	PKeyboard::Reset();
	PConsole::Startup();

#  ifdef PRIMEDEBUG_DEV
	PDebugRenderer::SetRenderer( a_pRenderer );
	PBindings::Startup();
	PStats::Startup();
	PDebugText::Startup( MAX_TEXT_VERTICES );
	PDebugWindow::Startup();
	PDebugMenu::Startup();

	PDebugPrimitives::Settings oSettings;
	oSettings.iMaxLines     = 0x4000;
	oSettings.iMaxTriangles = 0x2000;
	PDebugPrimitives::Startup( &oSettings, 12, 8, 12, 12 );

	s_oStatsVar.CreateIntRange( "debug.stats", "Shows the frame and CPU times (averaged over two seconds)", PCVar::FLAGS_VALIDATERANGE | PCVar::FLAGS_BOOL, 0, TNULL, TNULL, 0, 1, 1 );
#  endif
}

void PDebugSystem::Shutdown()
{
#  ifdef PRIMEDEBUG_DEV
	s_oStatsVar.Remove();
	PDebugPrimitives::Shutdown();
	PDebugMenu::Shutdown();
	PDebugWindow::Shutdown();
	PDebugText::Shutdown();
	PStats::Shutdown();
	PBindings::Shutdown();
	PDebugRenderer::SetRenderer( TNULL );
#  endif

	PConsole::Shutdown();
}

#  ifdef PRIMEDEBUG_DEV

void PDebugSystem::Update( TFLOAT a_fDeltaTime )
{
	PBindings::Update();
	PDebugMenu::Update( a_fDeltaTime );
	PKeyboard::EndFrame();

	PProfileCounter::EndFrame();
	PProfileCounter::NextFrame();
}

void PDebugSystem::UpdateStats( TFLOAT a_fCpuTime, TFLOAT a_fFrameTime )
{
	PStats::Update( a_fCpuTime, a_fFrameTime );
}

void PDebugSystem::Render3D()
{
	PDebugPrimitives::Render();
}

void PDebugSystem::Render2D( const TVector4* a_pEye )
{
	PDebugPrimitives::Flush2D();
	PDebugWindow::DrawAll( a_pEye );
	PDebugMenu::Draw();

	if ( s_oStatsVar.GetType() != PCVar::TYPE_NONE && s_oStatsVar.GetBool() )
		PStats::Print();

	PDebugText::Flush();
}

#  endif // PRIMEDEBUG_DEV

#endif // PRIMEDEBUG
