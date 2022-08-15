#pragma once
#include "TObject.h"

namespace Toshi
{
	class TClass
	{
		typedef TObject* (*TObjectCTOR)();
		typedef TObject* (*TObjectCTOR2)(void *);
		typedef void (*Func)();
	public:
		char* m_pcClassName; //0
		TObjectCTOR m_pTObjectCTOR; //0x4
		TObjectCTOR2 m_pTObjectCTOR2; //0x8
		Func m_pFunc; //0xC
		Func m_pFunc2; //0x10

		TClass* m_parent; //0x14
		TClass* m_attached; //0x18
		TClass* m_tclass; //0x1C
		int m_version; //0x20
		void* m_unk3; // 0x24
		void* m_unk4; // 0x28
		bool m_bInitialised; // 0x2C

		__thiscall TClass(char const *, class TClass*, TObjectCTOR, TObjectCTOR2, Func, Func, unsigned int);

		char* __thiscall GetName();
		void __thiscall InitialiseStatic();
		void __thiscall DeinitialiseStatic();
		bool __thiscall AttachClassToParent();
		bool __thiscall IsAttached() const;

	};
}

