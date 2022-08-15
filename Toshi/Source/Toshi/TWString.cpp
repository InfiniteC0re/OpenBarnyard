#include "pch.h"
#include "TWString.h"

int __thiscall Toshi::TWString::Length()
{
    return m_uilength & 0xFFFFFF;
}

Toshi::TWString::operator unsigned short* () const
{
    return *m_pBuffer;
}
