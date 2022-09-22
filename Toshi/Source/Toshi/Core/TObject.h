#pragma once
#include "TClass.h"

#include "Toshi/Core/Core.h"
#include "Toshi/Core/TMemory.h"
#include "Toshi/Utils/TLog.h"

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

	template<class T, class Parent, STL::StringLiteral Name, uint32_t Version, bool Instantiable>
	class TGenericClassDerived : public Parent
	{
	public:
		virtual TClass* GetClass()
		{
			return &s_Class;
		}

		static TObject* CreateTObject()
		{
			if constexpr (Instantiable) { return Toshi::tnew<T>(); }
			else { return nullptr; }
		}

		static TObject* CreateTObjectInPlace(void* ptr)
		{
			if constexpr (Instantiable) { return new (ptr) T(); }
			else { return nullptr; }
		}

	public:
		static TClass s_Class;
	};

	template <class T, class Parent, STL::StringLiteral Name, uint32_t Version, bool Instantiable>
	TClass TGenericClassDerived<T, Parent, Name, Version, Instantiable>::s_Class = TClass(Name.value, &Parent::s_Class, Version, sizeof(T), T::CreateTObject, T::CreateTObjectInPlace, 0, 0);
}

