#include "ToshiPCH.h"
#include "TDebugConsole.h"
#include "Platform/TDebugConsoleWindows.h"

namespace Toshi
{
    STL::Scope<TDebugConsole> TDebugConsoleFactory::Create()
    {
#ifdef TOSHI_PLATFORM_WINDOWS
        return STL::CreateScope<TDebugConsoleWindows>();
#else
#error "TDebugConsole: Unsupported Platform"
#endif
    }
}
