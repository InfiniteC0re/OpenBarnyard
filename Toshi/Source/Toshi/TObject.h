#pragma once
#include "TClass.h"

#include <type_traits>

//-----------------------------------------------------------------------------
// Disabled memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOff.h>

#define TClassObjectName   ms_oClass
#define TGetClass( CLASS ) CLASS::TClassObjectName

//-----------------------------------------------------------------------------
// Declares default methods used to register class in the dynamic class system.
// Note: Doesn't declare ms_oClass object (Use TDECLARE_CLASS to auto-declare).
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS )                       \
public:                                                                       \
	using ThisClass                             = THIS_CLASS;                 \
	using BaseClass                             = PARENT_CLASS;               \
	static constexpr Toshi::TClass* PARENTCLASS = &TGetClass( PARENT_CLASS ); \
                                                                              \
	virtual Toshi::TClass* GetClass() override;                               \
                                                                              \
	static Toshi::TObject* CreateTObject();                                   \
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );              \
	static void            InitialiseClass();                                 \
	static void            DeinitialiseClass();                               \
                                                                              \
	static TFORCEINLINE Toshi::TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

//-----------------------------------------------------------------------------
// Declares default methods to register derived class and ms_oClass object.
// Note: Use one of the TDEFINE_CLASS macros in a cpp file to register class.
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS( THIS_CLASS, PARENT_CLASS )   \
	TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS ); \
	static Toshi::TClass TClassObjectName;

//-----------------------------------------------------------------------------
// Declares default methods to register class and ms_oClass object.
// Note: Use one of the TDEFINE_CLASS macros in a cpp file to register class.
//-----------------------------------------------------------------------------
#define TDECLARE_CLASS_COMPILETIME( THIS_CLASS, PARENT_CLASS ) \
	TDECLARE_CLASS_BODY( THIS_CLASS, PARENT_CLASS );           \
	static constinit Toshi::TClass TClassObjectName;

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE( CLASS )                                                                                    \
	TSTATICASSERT( CLASS::IsTObject );                                                                                 \
	TSTATICASSERT( std::is_base_of<CLASS::BaseClass, CLASS>::value && !std::is_same<CLASS::BaseClass, CLASS>::value ); \
	Toshi::TClass*  CLASS::GetClass() { return CLASS::GetClassStatic(); }                                              \
	Toshi::TObject* CLASS::CreateTObject() { return new CLASS(); }                                                     \
	Toshi::TObject* CLASS::CreateTObjectInPlace( void* a_pPtr ) { return new ( a_pPtr ) CLASS(); }

//-----------------------------------------------------------------------------
// Defines core methods of TObject and prohibits creating object at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE_NORUNTIME( CLASS )                                                                          \
	TSTATICASSERT( CLASS::IsTObject );                                                                                 \
	TSTATICASSERT( std::is_base_of<CLASS::BaseClass, CLASS>::value && !std::is_same<CLASS::BaseClass, CLASS>::value ); \
	Toshi::TClass*  CLASS::GetClass() { return CLASS::GetClassStatic(); }                                              \
	Toshi::TObject* CLASS::CreateTObject()                                                                             \
	{                                                                                                                  \
		TASSERT( TFALSE, "This class cannot be created at runtime!" );                                                 \
		return TNULL;                                                                                                  \
	}                                                                                                                  \
	Toshi::TObject* CLASS::CreateTObjectInPlace( void* a_pPtr )                                                        \
	{                                                                                                                  \
		TASSERT( TFALSE, "This class cannot be created at runtime!" );                                                 \
		return TNULL;                                                                                                  \
	}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Note: Doesn't have type checks.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_CORE_COMPILETIME( CLASS )                           \
	TSTATICASSERT( CLASS::IsTObject );                                    \
	Toshi::TClass*  CLASS::GetClass() { return CLASS::GetClassStatic(); } \
	Toshi::TObject* CLASS::CreateTObject() { return new CLASS(); }        \
	Toshi::TObject* CLASS::CreateTObjectInPlace( void* a_pPtr ) { return new ( a_pPtr ) CLASS(); }

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL( CLASS )  \
	TDEFINE_CLASS_CORE( CLASS )      \
	void CLASS::InitialiseClass() {} \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and prohibits creating object at runtime.
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL_NORUNTIME( CLASS ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )     \
	void CLASS::InitialiseClass() {}          \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines core methods of TObject and allows creating object at runtime.
// Also, adds empty InitialiseClass and DeinitialiseClass methods.
// Note: Doesn't have type checks.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_FULL_COMPILETIME( CLASS ) \
	TDEFINE_CLASS_CORE_COMPILETIME( CLASS )     \
	void CLASS::InitialiseClass() {}            \
	void CLASS::DeinitialiseClass() {}

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: Can be created at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS( ... ) CALL_OVERLOAD( TDEFINE_CLASS, __VA_ARGS__ )
#define TDEFINE_CLASS1( CLASS ) \
	TDEFINE_CLASS_FULL( CLASS ) \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL( CLASS )                       \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), alignof( CLASS ) );

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: Cannot be created at runtime!
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_NORUNTIME1( CLASS ) \
	TDEFINE_CLASS_FULL_NORUNTIME( CLASS ) \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_NORUNTIME3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL_NORUNTIME( CLASS )                       \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_NORUNTIME( ... ) CALL_OVERLOAD( TDEFINE_CLASS_NORUNTIME, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines core TObject methods and the ms_oClass object.
// Provide InitialiseClass and DeinitialiseClass methods manually.
// Note: Can be created at runtime.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_INIT1( CLASS ) \
	TDEFINE_CLASS_CORE( CLASS )      \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_INIT3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_CORE( CLASS )                            \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_INIT( ... ) CALL_OVERLOAD( TDEFINE_CLASS_INIT, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines core TObject methods and the ms_oClass object.
// Provide InitialiseClass and DeinitialiseClass methods manually.
// Note: Cannot be created at runtime!
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_INIT_NORUNTIME1( CLASS ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )      \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, 0, 1, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_INIT_NORUNTIME3( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_CORE_NORUNTIME( CLASS )                            \
	Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::PARENTCLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), alignof( CLASS ) );
#define TDEFINE_CLASS_INIT_NORUNTIME( ... ) CALL_OVERLOAD( TDEFINE_CLASS_INIT_NORUNTIME, __VA_ARGS__ )

//-----------------------------------------------------------------------------
// Defines all TObject methods and the ms_oClass object.
// Note: ms_oClass object is created at compile time.
//-----------------------------------------------------------------------------
#define TDEFINE_CLASS_COMPILETIME( CLASS, VER_MAJOR, VER_MINOR ) \
	TDEFINE_CLASS_FULL_COMPILETIME( CLASS )                      \
	constinit Toshi::TClass CLASS::TClassObjectName = Toshi::TClass( #CLASS, CLASS::CreateTObject, CLASS::CreateTObjectInPlace, CLASS::InitialiseClass, CLASS::DeinitialiseClass, VER_MAJOR, VER_MINOR, sizeof( CLASS ), alignof( CLASS ) );

TOSHI_NAMESPACE_START

class TObject
{
public:
	enum
	{
		IsTObject = TTRUE
	};
	static constexpr Toshi::TClass* PARENTCLASS = TNULL;

public:
	virtual Toshi::TClass* GetClass();
	virtual void           Delete();
	virtual ~TObject();

public:
	TBOOL IsExactly( TClass* a_pClass ) { return GetClass() == a_pClass; }
	TBOOL IsA( TClass* a_pClass ) { return GetClass()->IsA( a_pClass ); }

public:
	static Toshi::TObject* CreateTObject();
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );
	static void            InitialiseClass();
	static void            DeinitialiseClass();

	static TFORCEINLINE TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

public:
	static constinit Toshi::TClass TClassObjectName;
};

TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Safely casts TObject to specified TClass
//-----------------------------------------------------------------------------
template <class T>
TFORCEINLINE T* TDynamicCast( Toshi::TObject* a_pObject )
{
	TSTATICASSERT( T::IsTObject );

	if ( a_pObject )
	{
		if ( a_pObject->IsA( &TGetClass( T ) ) )
		{
			return TSTATICCAST( T, a_pObject );
		}
	}

	return TNULL;
}

#define TDYNAMICCAST( T, OBJECT ) ( TDynamicCast<T>( OBJECT ) )
