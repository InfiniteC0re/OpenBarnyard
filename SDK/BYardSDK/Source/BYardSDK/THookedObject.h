#pragma once

class THookedObject
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
	virtual ~THookedObject();

public:
	TBOOL IsExactly( Toshi::TClass* a_pClass ) { return GetClass() == a_pClass; }
	TBOOL IsA( Toshi::TClass* a_pClass ) { return GetClass()->IsA( a_pClass ); }

public:
	static Toshi::TObject* CreateTObject();
	static Toshi::TObject* CreateTObjectInPlace( void* a_pPtr );

	static TFORCEINLINE Toshi::TClass* GetClassStatic() { return std::addressof( TClassObjectName ); }

public:
	inline static Toshi::TClass* ms_pClass        = TREINTERPRETCAST( Toshi::TClass*, 0x0077cf30 );
	inline static Toshi::TClass& TClassObjectName = *ms_pClass;
};
