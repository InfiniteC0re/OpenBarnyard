#include "ToshiPCH.h"
#include "TDebugConsole.h"
#include "Platform/Windows/TDebugConsoleWindows.h"

namespace Toshi
{
    STL::Scope<TDebugConsole> TDebugConsoleFactory::Create()
    {
#ifdef TOSHI_PLATFORM_WINDOWS
        return STL::CreateScope<TDebugConsoleWindows, TDebugConsole>();
#else
#error "TDebugConsole: Unsupported Platform"
#endif
    }
}
