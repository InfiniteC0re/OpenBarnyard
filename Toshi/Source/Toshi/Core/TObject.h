#pragma once
#include "TClass.h"

#include "Toshi/Core/Core.h"
#include "Toshi/Core/TMemory.h"
#include "Toshi/Utils/TLog.h"

#define TOSHI_CLASS_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return &s_Class; } \
	static Toshi::TObject* CreateTObject() { return Toshi::tnew<CLASSNAME>(); } \
	static Toshi::TObject* CreateTObjectInPlace(void* ptr) { return new (ptr) CLASSNAME(); } \
	static Toshi::TClass s_Class;

#define TOSHI_CLASS_NO_CREATE_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return &s_Class; } \
	static Toshi::TObject* CreateTObject() { return nullptr; } \
	static Toshi::TObject* CreateTObjectInPlace(void* ptr) { return nullptr; } \
	static Toshi::TClass s_Class;

#define TOSHI_CLASS_STATIC_DEFINE(CLASSNAME) \
public: \
	virtual Toshi::TClass* GetClass() { return s_Class; } \
	inline static Toshi::TObject* CreateTObject() { return Toshi::tnew<CLASSNAME>(); } \
	inline static Toshi::TObject* CreateTObjectInPlace(void* ptr) { return new (ptr) CLASSNAME(); } \
	static constinit Toshi::TClass s_Class;

#define TOSHI_CLASS_DERIVED_INITIALIZE(CLASSNAME, PARENT, VER) \
Toshi::TClass CLASSNAME::s_Class(#CLASSNAME, &PARENT::s_Class, VER, sizeof(CLASSNAME), CLASSNAME::CreateTObject, CLASSNAME::CreateTObjectInPlace, 0, 0);

#define TOSHI_CLASS_INITIALIZE(CLASSNAME, VER) \
Toshi::TClass CLASSNAME::s_Class(#CLASSNAME, TNULL, VER, sizeof(CLASSNAME), CLASSNAME::CreateTObject, CLASSNAME::CreateTObjectInPlace, 0, 0);

namespace Toshi
{
	class TObject
	{
	public:
		virtual TClass* GetClass() { return &s_Class; }
		virtual void Delete() { tdelete(this); };
		virtual ~TObject() = default;

		inline static TObject* CreateTObject() { return nullptr; } 
		inline static TObject* CreateTObjectInPlace(void* ptr) { return nullptr; }

		static inline void* operator new(size_t size) = delete;
		static inline void* operator new(size_t size, void* at) { return at; }

		static inline void operator delete(void* block) { tfree(block); }
		static inline void operator delete(void* block, void* at) { tfree(block); }
	
		inline bool IsExactly(TClass* toCompare) { return GetClass() == toCompare; }
	
	public:
		static constinit TClass s_Class;
	};
}

