#pragma once
#include <stdint.h>
#include "TClass.h"
#include "Typedefs.h"
namespace Toshi
{
	class TObject
	{
	public:
		static class Toshi::TClass m_sClass;
		TBOOL m_bInitialised; // this + 0x2C

		virtual void __thiscall Delete();
		void __thiscall DestructTObject(uint32_t param_2);
		__thiscall ~TObject();
		virtual class Toshi::TClass* __thiscall GetClass();

	};
}

