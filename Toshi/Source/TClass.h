#pragma once
#include "TObject.h"

namespace Toshi
{
	class TClass
	{
	public:
		char* m_pcClassName; //0
		void* m_pTObjectCTOR; //0x4
		void* m_pTObjectDTOR; //0x8
		void* m_pFunc; //0xC
		void* m_pFunc2; //0x10

		Toshi::TClass* m_parent; //0x14
		Toshi::TClass* m_attached; //0x18
		Toshi::TClass* m_tclass; //0x1C
		int m_version; //0x20
		void* m_unk3; // 0x24
		void* m_unk4; // 0x28
		bool m_bInitialised; // 0x2C

		__thiscall TClass(char const *, class Toshi::TClass*, void*, void *, void*, void*, unsigned int);

		char* __thiscall GetName();
		void __thiscall InitialsieStatic();
		void __thiscall DeinitialsieStatic();
		bool __thiscall AttachClassToParent();
		bool __thiscall IsAttached() const;

	};
}

