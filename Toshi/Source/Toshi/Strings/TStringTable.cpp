#include "ToshiPCH.h"
#include "TStringTable.h"

namespace Toshi
{
	int TStringTable::GetStringID(const char* string)
	{
		{
			TASSERT(T2String8::IsLowerCase(string));

			int iVar2;
			short sVar3;
			short sVar4;
			int index;
			int uVar5;

			sVar3 = *(short*)&m_NumStrings;
			index = sVar3 >> 1;
			if (sVar3 >> 1 < 0x8000) {
				uVar5 = 0;
				do {
					sVar4 = (short)index;
					if (sVar3 <= sVar4) {
						return -1;
					}
					iVar2 = T2String8::Compare(string, m_Strings[sVar4], -1);
					if (iVar2 == 0) {
						return (int)sVar4;
					}
					if (-1 < iVar2) {
						uVar5 = index + 1 & 0xffff;
						sVar4 = sVar3;
					}
					sVar3 = sVar4;
					index = (((int)sVar3 - uVar5) >> 1) + uVar5 & 0xffff;
				} while ((short)uVar5 <= (short)index);
			}

			return -1;
		}
	}
}