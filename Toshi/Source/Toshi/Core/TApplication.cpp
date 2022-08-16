#include "pch.h"
#include "TApplication.h"

bool __thiscall Toshi::TApplication::OnCreate()
{
    // *(uint *)(param_1 + 0x10) = *(uint *)(param_1 + 0x10) | 1;
    unk |= 1;
    return true;
}

bool Toshi::TApplication::OnDestroy()
{
    return true;
}

bool Toshi::TApplication::IsCreated()
{
    return unk1 & 1 != 0;
}

HWND Toshi::TApplication::UnkFun()
{
    TCString str = TCString((char *)someKindOfKey);
    DWORD time = timeGetTime();
    char buf[_MAX_ULTOSTR_BASE16_COUNT]{};
    _ultoa(time, buf, 0x10);

    /*  Stuff de blob does Reminder: puVar5 = buf
    * Start of code
    pcVar3 = DAT_009823b0;
  do {
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
    pcVar3[(int)&uStack229 - (int)DAT_009823b0] = cVar1;
  } while (cVar1 != '\0');
  puVar5 = local_1c;
  do {
    cVar1 = *(char *)puVar5;
    puVar5 = (undefined4 *)((int)puVar5 + 1);
  } while (cVar1 != '\0');
  uVar6 = (int)puVar5 - (int)local_1c;
  puVar5 = &uStack229;
  do {
    pcVar3 = (char *)((int)puVar5 + 1);
    puVar5 = (undefined4 *)((int)puVar5 + 1);
  } while (*pcVar3 != '\0');
  puVar7 = local_1c;
  for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *puVar5 = *puVar7;
    puVar7 = puVar7 + 1;
    puVar5 = puVar5 + 1;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined *)puVar5 = *(undefined *)puVar7;
    puVar7 = (undefined4 *)((int)puVar7 + 1);
    puVar5 = (undefined4 *)((int)puVar5 + 1);
  }
  
  */

    str += buf;
    char lpConsoleTitle[0x200];
    DWORD unk3 = GetConsoleTitleA(lpConsoleTitle, 0x200);
    if (unk3)
    {
        //delete str;
        return TNULL;
    }
    SetConsoleTitleA(str);
    FindWindowA(TNULL, str);
    SetConsoleTitleA(lpConsoleTitle);
    // delete str
}
