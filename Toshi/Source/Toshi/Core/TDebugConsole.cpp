#include "ToshiPCH.h"
#include "TDebugConsole.h"
#include "Platform/Windows/TDebugConsole_Win.h"

namespace Toshi
{
    STL::Scope<TDebugConsole> TDebugConsoleFactory::Create()
    {
#ifdef TOSHI_PLATFORM_WINDOWS
        return STL::CreateScope<TDebugConsole_Win, TDebugConsole>();
#else
#error "TDebugConsole: Unsupported Platform"
#endif
    }
}
