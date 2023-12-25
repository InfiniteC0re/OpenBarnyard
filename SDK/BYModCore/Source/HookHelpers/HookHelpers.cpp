#include "pch.h"
#include "HookHelpers.h"

#include <detours.h>

bool UnprotectMemory(void* memory, std::size_t size, unsigned long& oldProtect)
{
    return (VirtualProtect(memory, size, PAGE_EXECUTE_READWRITE, &oldProtect) != 0);
}

bool WriteJump(void* jumpAddress, void* output)
{
#if defined(_X86_)
    // Unprotect memory.
    if (!UnprotectMemory(output, 5))
    {
        return false;
    }
    
    // Write x86 jmp instruction.
    auto ouputBytes = static_cast<std::uint8_t*>(output);
    *ouputBytes = 0xE9;
    *reinterpret_cast<std::uint32_t*>(ouputBytes + 1) = (
        reinterpret_cast<std::uintptr_t>(jumpAddress) -
        (reinterpret_cast<std::uintptr_t>(output) + 5));

    return true;
#else
// TODO: Support other architectures.
#error "Unsupported architecture"
#endif
}

void InstallHook(void*& origFunc, void* hookFunc)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&origFunc, hookFunc);
    DetourTransactionCommit();
}

void* InstallVTableHook(void** vtable, std::size_t virtualFuncIndex, void* hookFunc)
{
    // Get original function pointer.
    void* origFunc = vtable[virtualFuncIndex];

    // Set new function pointer and return original function pointer.
    vtable[virtualFuncIndex] = hookFunc;
    return origFunc;
}

