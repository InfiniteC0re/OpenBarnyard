#pragma once
#include <stdint.h>
#include "TClass.h"

namespace Toshi
{
	class TObject
	{
	public:
		virtual TClass* Class() = 0;
		virtual void Allocate() = 0;
		virtual ~TObject() = 0;

		// virtual void Delete(char freeMemory) = 0;

		//static class TClass m_sClass;
		//TBOOL m_bInitialised; // this + 0x2C

		/*virtual void __thiscall Delete();
		void __thiscall DestructTObject(uint32_t param_2);
		__thiscall ~TObject();*/
		//virtual TClass* GetClass();

	};
}

