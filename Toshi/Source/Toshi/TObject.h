#pragma once
#include <stdint.h>
#include "TMemory.h"
#include "TClass.h"
#include "Typedefs.h"

#define TOSHI_CLASS_REGISTER(CLASSNAME) \
public: \
	virtual Toshi::TClass* Class() { return &s_Class; } \
	static void* CreateObject() { return new CLASSNAME; } \
	static void* CreateObjectAtPlace() { TASSERT(false); return nullptr; } \
private: \
	static Toshi::TClass s_Class;

#define TOSHI_CLASS_INITIALIZE(CLASSNAME, PARENT, VERSION_MAJOR, VERSION_MINOR) \
Toshi::TClass CLASSNAME::s_Class(#CLASSNAME, PARENT, CLASSNAME::CreateObject, CLASSNAME::CreateObjectAtPlace, 0, VERSION_MAJOR, VERSION_MINOR, sizeof(CLASSNAME));

namespace Toshi
{
	class TObject
	{
	public:
		virtual TClass* Class() = 0;
		virtual void Destroy();
		virtual ~TObject();

		// Operators
		static void* operator new(size_t size)
		{
			return tmalloc(size);
		}

		static void operator delete(void* block)
		{
			tfree(block);
		}
	};
}

