#pragma once
#include "TClass.h"

#include "Toshi/Core/Core.h"
#include "Toshi/Core/TMemory.h"
#include "Toshi/Utils/TLog.h"

#define TOSHI_CLASS_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return &s_Class; } \
	static Toshi::TObject* CreateTObject() { return new CLASSNAME; } \
	static Toshi::TObject* CreateTObjectInPlace(void* block) { return new (block) CLASSNAME(); } \
	static Toshi::TClass s_Class;

#define TOSHI_CLASS_NO_CREATE_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return &s_Class; } \
	static Toshi::TObject* CreateTObject() { return nullptr; } \
	static Toshi::TObject* CreateTObjectInPlace(void* block) { return nullptr; } \
	static Toshi::TClass s_Class;

#define TOSHI_CLASS_STATIC_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return s_Class; } \
	static Toshi::TObject* CreateTObject() { return new CLASSNAME; } \
	static Toshi::TObject* CreateTObjectInPlace(void* block) { return new (block) CLASSNAME(); } \
	static constinit Toshi::TClassProps s_Class;

#define TOSHI_CLASS_DERIVED_INITIALIZE(CLASSNAME, PARENT, VER) \
Toshi::TClass CLASSNAME::s_Class(#CLASSNAME, &PARENT::s_Class, VER, sizeof(CLASSNAME), CLASSNAME::CreateTObject, CLASSNAME::CreateTObjectInPlace, 0, 0);

#define TOSHI_CLASS_INITIALIZE(CLASSNAME, VER) \
Toshi::TClass CLASSNAME::s_Class(#CLASSNAME, TNULL, VER, sizeof(CLASSNAME), CLASSNAME::CreateTObject, CLASSNAME::CreateTObjectInPlace, 0, 0);

namespace Toshi
{
	class TObjectInterface
	{
	public:
		virtual TClass* GetClass() = 0;
		virtual void Delete();
		virtual ~TObjectInterface();

		// Operators
		static inline void* operator new(size_t size) { return tmalloc(size); }
		static inline void* operator new(size_t size, void* at) { return at; }

		static inline void operator delete(void* block) { tfree(block); }
		static inline void operator delete(void* block, void* at) { tfree(block); }
	};

	class TObject : public TObjectInterface
	{
		TOSHI_CLASS_STATIC_DEFINE(TObject)

	public:
		inline bool IsExactly(TClass* toCompare) { return GetClass() == toCompare; }
	};
}

