#include "pch.h"
#include "TCString.h"

void __thiscall Toshi::TCString::Copy(Toshi::TWString * param_1, int a_iLength)
{
    int iVar1 = param_1->Length();
    if ((iVar1 < a_iLength) || (a_iLength == -1))
    {
        a_iLength = param_1->Length();
    }
    AllocBuffer(a_iLength, true);
    TSystem::StringUnicodeToChar(*m_pBuffer, param_1->operator unsigned short* (), a_iLength);
    m_pBuffer[a_iLength] = 0;
}

void __thiscall Toshi::TCString::Copy(const TCString* param_1, int param_2)
{
    //TODO
}

void __thiscall Toshi::TCString::Copy(char const* param_1, int a_iLength)
{
    int size;

    if (param_1 != *m_pBuffer)
    {
        if (param_1 == TNULL)
        {
            size = 0;
        }
        else
        {
            size = Toshi::TSystem::StringLength(param_1);
        }
        if ((size < a_iLength) || (a_iLength == -1))
        {
            a_iLength = size;
        }
        AllocBuffer(a_iLength, true);
        TSystem::MemCopy(m_pBuffer, param_1, a_iLength);
        m_pBuffer[a_iLength] = 0;
    }
}

bool __thiscall Toshi::TCString::IsIndexValid(int param_1) const
{
    if ((param_1 <= length & 0xffffff) && (-1 < param_1))
    {
        return true;
    }
    return false;
}

int __thiscall Toshi::TCString::Find(char param_1, int param_2) const
{
    if ((length & 0xffffff < param_2) || (param_2 < 0))
    {
        return -1;
    }
    //if (DAT_00990290 == 0)
    bool isValid = IsIndexValid(0);
    if (!isValid)
    {
        TASSERT(IsIndexValid(0));
        return -1;
    }
    char* foundAt = strchr(m_pBuffer[param_2], param_1);
    if (foundAt == TNULL)
    {
        return -1;
    }
    isValid = IsIndexValid(0);
    if (!isValid)
    {
        TASSERT(IsIndexValid(0));
        return -1;
    }
    return foundAt - *m_pBuffer;
}

bool __thiscall Toshi::TCString::AllocBuffer(int a_iLength, bool freeMemory)
{
    int a_iX;
    bool rVal;

    TASSERT(a_iLength >= 0);

    a_iX = length & 0xFFFFFF;
    TASSERT(a_iX <= TINT32_MAX);

    if (a_iLength == a_iX)
    {
        rVal = false;
    }
    else
    {
        if (a_iLength == 0)
        {
            if (freeMemory)
            {
                tfree(m_pBuffer);
            }
            m_pBuffer = nullptr;
            rVal = true;
            //this+7 = 0
        }
        else
        {
            int iVar2 = (a_iX - a_iLength); // + this[7]
            if ((iVar2 < 0) || (0xFF < iVar2))
            {
                if ((a_iX != 0) && freeMemory)
                {
                    tfree(m_pBuffer);
                }
                void * mem = tmalloc(a_iLength + 1);
                *m_pBuffer = (char*)mem;
                //this + 7 = 0
                TASSERT(m_pBuffer != TNULL);
            }
            else
            {
                //this + 7 = (char) uVar1;
                rVal = false;
            }
        }
        length = length & 0xFF000000 | a_iLength & 0xFFFFFF;
    }
    if (freeMemory)
    {
        m_pBuffer = nullptr;
    }
    return rVal;
}

int Toshi::TCString::Length() const
{
    return length & 0xFFFFFF;
}

const char* Toshi::TCString::GetString(int param_1) const
{
    int valid = IsIndexValid(param_1);
    if (valid)
    {
        return ((*m_pBuffer) + param_1);
    }
    return TNULL;
}

__thiscall Toshi::TCString::TCString()
{
    m_pBuffer = nullptr;
    length = length & 0xFF000000;
    // this+7 = 0
    /*  if (param_1 == 0) {
    if ((_DAT_009b1704 == 0) && (sm_pDefaultAllocatorCB == (code *)0x0)) {
      puVar1 = (undefined4 *)
               Assert("TNULL!=sm_pDefaultAllocatorCB","..\\..\\Source\\Toshi\\TString8.cpp",100,
                      "TASSERT");
      return puVar1;
    }
    param_1 = (*sm_pDefaultAllocatorCB)();
  }
  *(int *)((int)this + 8) = param_1;
  uVar2 = (*(int *)((int)this + 4) << 8) >> 8;
  if ((_DAT_009b1700 == 0) && (0x7fffffff < uVar2)) {
    puVar1 = (undefined4 *)
             Assert("a_iX <= TINT32_MAX",
                    "E:\\src\\Development\\ToshiBranch\\Source\\Math/TMathInline.h",0x67,"TASSERT");
    return puVar1;
  }
  if (uVar2 != 0) {
    (**(code**)(**(int**)((int)this + 8) + 8))(*this);
    *(undefined*)((int)this + 7) = 0;
    *(uint*)((int)this + 4) = *(uint*)((int)this + 4) & 0xff000000;
    *(undefined1**)this = &m_aNull;
}
**this = 0;
return (undefined4*)this;
    */
}

Toshi::TCString::TCString(int param_1)
{
    //this->m_pBuffer = &m_aNull;
    length = length & 0xff000000;
    //this + 7 = 0
    //TODO
}
