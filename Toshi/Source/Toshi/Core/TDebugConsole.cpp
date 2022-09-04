#include "ToshiPCH.h"
#include "TDebugConsole.h"
#include "Platform/TDebugConsoleWindows.h"

namespace Toshi
{
    Scope<TDebugConsole> TDebugConsoleFactory::Create()
    {
#ifdef TOSHI_PLATFORM_WINDOWS
        return CreateScope<TDebugConsoleWindows>();
#else
#error "TDebugConsole: Unsupported Platform"
#endif
    }
}
