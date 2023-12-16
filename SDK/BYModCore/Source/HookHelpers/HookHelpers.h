#ifndef HOOK_HELPERS_H_INCLUDED
#define HOOK_HELPERS_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <detours.h>

bool UnprotectMemory(void* memory, std::size_t size, unsigned long& oldProtect);

template<typename T>
inline bool UnprotectMemory(T& val, unsigned long& oldProtect)
{
    return UnprotectMemory(&val, sizeof(T), oldProtect);
}

inline bool UnprotectMemory(void* memory, std::size_t size)
{
    unsigned long oldProtect;
    return UnprotectMemory(memory, size, oldProtect);
}

template<typename T>
inline bool UnprotectMemory(T& val)
{
    return UnprotectMemory(&val, sizeof(T));
}

bool WriteJump(void* jumpAddress, void* output);

/* Hooks */
#define CONST_MEMBER_HOOK(address, objName, hookName, returnType, ...)\
    struct hookName\
    {\
        using _func_ptr_type = returnType (__thiscall*)(const objName*, __VA_ARGS__);\
        static _func_ptr_type _orig_func_ptr;\
        static constexpr bool IS_MEMBER_HOOK = true;\
\
        struct _hook_obj : public objName\
        {\
            returnType CallOriginal(__VA_ARGS__) const;\
            returnType _hook_func(__VA_ARGS__) const;\
        };\
\
        static constexpr auto _hook_func_ptr =  &_hook_obj::_hook_func;\
    };\
\
    hookName::_func_ptr_type hookName::_orig_func_ptr =\
        reinterpret_cast<hookName::_func_ptr_type>(address);\
\
    __declspec(naked) returnType hookName::_hook_obj::CallOriginal(__VA_ARGS__) const\
    {\
        __asm jmp DWORD PTR [_orig_func_ptr]\
    }\
\
    returnType hookName::_hook_obj::_hook_func(__VA_ARGS__) const

#define MEMBER_HOOK(address, objName, hookName, returnType, ...)\
    struct hookName\
    {\
        using _func_ptr_type = returnType (__thiscall*)(objName*, __VA_ARGS__);\
        static _func_ptr_type _orig_func_ptr;\
        static constexpr bool IS_MEMBER_HOOK = true;\
\
        struct _hook_obj : public objName\
        {\
            returnType CallOriginal(__VA_ARGS__);\
            returnType _hook_func(__VA_ARGS__);\
        };\
\
        static constexpr auto _hook_func_ptr =  &_hook_obj::_hook_func;\
    };\
\
    hookName::_func_ptr_type hookName::_orig_func_ptr =\
        reinterpret_cast<hookName::_func_ptr_type>(address);\
\
    __declspec(naked) returnType hookName::_hook_obj::CallOriginal(__VA_ARGS__)\
    {\
        __asm jmp DWORD PTR [_orig_func_ptr]\
    }\
\
    returnType hookName::_hook_obj::_hook_func(__VA_ARGS__)

#define HOOK(address, hookName, returnType, ...)\
    struct hookName\
    {\
        using _func_ptr_type = returnType (__stdcall*)(__VA_ARGS__);\
        static _func_ptr_type _orig_func_ptr;\
        static constexpr bool IS_MEMBER_HOOK = false;\
\
        struct _hook_obj\
        {\
            static returnType __stdcall CallOriginal(__VA_ARGS__);\
            static returnType __stdcall _hook_func(__VA_ARGS__);\
        };\
\
        static constexpr auto _hook_func_ptr =  &_hook_obj::_hook_func;\
    };\
\
    hookName::_func_ptr_type hookName::_orig_func_ptr =\
        reinterpret_cast<hookName::_func_ptr_type>(address);\
\
    __declspec(naked) returnType __stdcall hookName::_hook_obj::CallOriginal(__VA_ARGS__)\
    {\
        __asm jmp DWORD PTR [_orig_func_ptr]\
    }\
\
    returnType hookName::_hook_obj::_hook_func(__VA_ARGS__)

#define CONST_DERIVED_MEMBER_HOOK(address, baseOffset, objName, hookName, returnType, ...)\
    struct hookName\
    {\
        using _func_ptr_type = returnType (__thiscall*)(const objName*, __VA_ARGS__);\
        static _func_ptr_type _orig_func_ptr;\
        static constexpr bool IS_MEMBER_HOOK = true;\
\
        struct _hook_obj : public objName\
        {\
            returnType CallOriginal(__VA_ARGS__) const;\
            returnType _hook_func(__VA_ARGS__) const;\
            returnType _inner_func(__VA_ARGS__) const;\
        };\
\
        static constexpr auto _hook_func_ptr =  &_hook_obj::_hook_func;\
    };\
\
    hookName::_func_ptr_type hookName::_orig_func_ptr =\
        reinterpret_cast<hookName::_func_ptr_type>(address);\
\
    __declspec(naked) returnType hookName::_hook_obj::CallOriginal(__VA_ARGS__) const\
    {\
        __asm add ecx, baseOffset\
        __asm jmp DWORD PTR [_orig_func_ptr]\
    }\
\
    __declspec(naked) returnType hookName::_hook_obj::_hook_func(__VA_ARGS__) const\
    {\
        __asm sub ecx, baseOffset\
        __asm jmp DWORD PTR [_inner_func]\
    }\
\
    returnType hookName::_hook_obj::_inner_func(__VA_ARGS__) const

#define DERIVED_MEMBER_HOOK(address, baseOffset, objName, hookName, returnType, ...)\
    struct hookName\
    {\
        using _func_ptr_type = returnType (__thiscall*)(objName*, __VA_ARGS__);\
        static _func_ptr_type _orig_func_ptr;\
        static constexpr bool IS_MEMBER_HOOK = true;\
\
        struct _hook_obj : public objName\
        {\
            returnType CallOriginal(__VA_ARGS__);\
            returnType _hook_func(__VA_ARGS__);\
            returnType _inner_func(__VA_ARGS__);\
        };\
\
        static constexpr auto _hook_func_ptr =  &_hook_obj::_hook_func;\
    };\
\
    hookName::_func_ptr_type hookName::_orig_func_ptr =\
        reinterpret_cast<hookName::_func_ptr_type>(address);\
\
    __declspec(naked) returnType hookName::_hook_obj::CallOriginal(__VA_ARGS__)\
    {\
        __asm add ecx, baseOffset\
        __asm jmp DWORD PTR [_orig_func_ptr]\
    }\
\
    __declspec(naked) returnType hookName::_hook_obj::_hook_func(__VA_ARGS__)\
    {\
        __asm sub ecx, baseOffset \
        __asm jmp DWORD PTR [_inner_func]\
    }\
\
    returnType hookName::_hook_obj::_inner_func(__VA_ARGS__)

void InstallHook(void*& origFunc, void* hookFunc);

template<typename ret_t, typename... args_t>
inline void InstallHook(ret_t (*&origFunc)(args_t...),
    ret_t (*hookFunc)(args_t...))
{
    InstallHook(reinterpret_cast<void*&>(origFunc),
        static_cast<void*>(hookFunc));
}

template<typename ret_t, typename obj_t, typename hook_obj_t, typename... args_t>
inline void InstallHook(ret_t(__thiscall*& origFunc)(const obj_t*, args_t...),
	ret_t(hook_obj_t::* hookFunc)(args_t...) const)
{
	InstallHook(reinterpret_cast<void*&>(origFunc),
		reinterpret_cast<void*&>(hookFunc));
}

template<typename ret_t, typename obj_t, typename hook_obj_t, typename... args_t>
inline void InstallHook(ret_t(__thiscall*& origFunc)(obj_t*, args_t...),
	ret_t(hook_obj_t::* hookFunc)(args_t...))
{
	InstallHook(reinterpret_cast<void*&>(origFunc),
		reinterpret_cast<void*&>(hookFunc));
}

template<typename ret_t, typename hook_obj_t, typename... args_t>
inline void InstallHook(ret_t(__stdcall*& origFunc)(args_t...),
	ret_t(__stdcall* hookFunc)(args_t...))
{
	InstallHook(reinterpret_cast<void*&>(origFunc),
		reinterpret_cast<void*&>(hookFunc));
}

template<typename hook_t>
inline void InstallHook()
{
    if constexpr (hook_t::IS_MEMBER_HOOK)
    {
		InstallHook(hook_t::_orig_func_ptr, hook_t::_hook_func_ptr);
    }
    else
    {
		InstallHook((void*&)hook_t::_orig_func_ptr, hook_t::_hook_func_ptr);
    }
}

// Virtual hook helpers.
inline bool UnprotectVTable(void** vtable, std::size_t virtualFuncCount,
    unsigned long& oldProtect)
{
    return UnprotectMemory(vtable, sizeof(void*) * virtualFuncCount, oldProtect);
}

inline bool UnprotectVTable(void** vtable, std::size_t virtualFuncCount)
{
    return UnprotectMemory(vtable, sizeof(void*) * virtualFuncCount);
}

void* InstallVTableHook(void** vtable,
    std::size_t virtualFuncIndex, void* hookFunc);

template<typename ret_t, typename hook_obj_t, typename... args_t>
inline void* InstallVTableHook(void** vtable, std::size_t virtualFuncIndex,
    ret_t (hook_obj_t::*hookFunc)(args_t...) const)
{
    return InstallVTableHook(vtable, virtualFuncIndex,
        reinterpret_cast<void*&>(hookFunc));
}

template<typename ret_t, typename hook_obj_t, typename... args_t>
inline void* InstallVTableHook(void** vtable, std::size_t virtualFuncIndex,
    ret_t (hook_obj_t::*hookFunc)(args_t...))
{
    return InstallVTableHook(vtable, virtualFuncIndex,
        reinterpret_cast<void*&>(hookFunc));
}

template<typename hook_t>
inline void* InstallVTableHook(void** vtable, std::size_t virtualFuncIndex)
{
    return InstallVTableHook(vtable, virtualFuncIndex, hook_t::_hook_func_ptr);
}

class vtable_ptr
{
    void** m_value;

public:
    inline bool Unprotect(std::size_t virtualFuncCount,
        unsigned long& oldProtect) const
    {
        return UnprotectMemory(m_value, sizeof(void*) * virtualFuncCount, oldProtect);
    }

    inline bool Unprotect(std::size_t virtualFuncCount) const
    {
        return UnprotectMemory(m_value, sizeof(void*) * virtualFuncCount);
    }

    inline void* InstallHook(std::size_t virtualFuncIndex, void* hookFunc) const
    {
        return InstallVTableHook(m_value, virtualFuncIndex, hookFunc);
    }

    template<typename ret_t, typename hook_obj_t, typename... args_t>
    inline void* InstallHook(std::size_t virtualFuncIndex,
        ret_t (hook_obj_t::* hookFunc)(args_t...) const) const
    {
        return InstallVTableHook(m_value, virtualFuncIndex, hookFunc);
    }

    template<typename ret_t, typename hook_obj_t, typename... args_t>
    inline void* InstallHook(std::size_t virtualFuncIndex,
        ret_t (hook_obj_t::* hookFunc)(args_t...)) const
    {
        return InstallVTableHook(m_value, virtualFuncIndex, hookFunc);
    }

    template<typename hook_t>
    inline void* InstallHook(std::size_t virtualFuncIndex) const
    {
        return InstallVTableHook(m_value, virtualFuncIndex, hook_t::_hook_func_ptr);
    }

    inline vtable_ptr() = default;

    inline vtable_ptr(std::nullptr_t) noexcept :
        m_value(nullptr) {}

    inline vtable_ptr(std::uintptr_t address) noexcept :
        m_value(reinterpret_cast<void**>(address)) {}

    inline vtable_ptr(void** ptr) noexcept :
        m_value(ptr) {}
};
#endif

