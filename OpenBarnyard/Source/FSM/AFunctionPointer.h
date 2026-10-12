#pragma once
#include "AMethodPointer.h"

//-----------------------------------------------------------------------------
// Script-callable free (static) functions, one class template per arity (and
// per return type for the RT variants). They mirror AMethodPointer.h.
// The destructor is the shared (identical code folding) one at 00499c90 and
// IsMethod is RETURNS_FALSE. Create, Clone and Call exist once per instantiation,
// GetSignature is mostly folded.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Binds a static function with one argument and no result to a script function name.
//-----------------------------------------------------------------------------
template <class A1>
class AFunctionPointer1Args
    : public AFunction
{
public:
	using Function = void ( * )( A1 );

public:
	// TFLOAT: ACoinPickupMicroGame "SetTimeLimit"
	// TINT:   ACoinPickupMicroGame "SetSplineID", "SetNumberOfCoins" and AShakeTreeMicroGame "PushState"
	// TBOOL:  ATreasueHuntMicroGame "PushState"
	// $Barnyard: FUNCTION 005361a0
	// $Barnyard: FUNCTION 0052cc90
	// $Barnyard: FUNCTION 0052e3c0
	static AFunctionPointer1Args* Create( const TCHAR* a_szName, Function a_fnFunction )
	{
		return new AFunctionPointer1Args( a_szName, a_fnFunction );
	}

	// Shared (identical code folding) deleting destructor of every AFunctionPointer class
	// $Barnyard: FUNCTION 00499c90
	~AFunctionPointer1Args() override = default;

	// $Barnyard: FUNCTION 00536220
	// $Barnyard: FUNCTION 0052cd10
	// $Barnyard: FUNCTION 0052e440
	AFunction* Clone() const override
	{
		return new AFunctionPointer1Args( *this );
	}

	// $Barnyard: FUNCTION 005362a0
	// $Barnyard: FUNCTION 0052cd90
	// $Barnyard: FUNCTION 0052e4c0
	void Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override
	{
		m_fnFunction( AFunctionArg<A1>::Get( a_pArgs->GetValue( 0 ) ) );
	}

	// TINT: folded 005d8a30
	// $Barnyard: FUNCTION 005371a0
	// $Barnyard: FUNCTION 00593d00
	void GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override
	{
		*a_ppReturnType = AFunctionArg<void>::GetType();
		a_rArgTypes.Push( AFunctionArg<A1>::GetType() );
	}

private:
	AFunctionPointer1Args( const TCHAR* a_szName, Function a_fnFunction )
	    : AFunction( a_szName )
	    , m_fnFunction( a_fnFunction )
	{}

	AFunctionPointer1Args( const AFunctionPointer1Args& a_rOther )
	    : AFunction( a_rOther )
	    , m_fnFunction( a_rOther.m_fnFunction )
	{}

private:
	Function m_fnFunction; // 0x08
};

//-----------------------------------------------------------------------------
// Binds a static function without arguments to a script function name.
//-----------------------------------------------------------------------------
template <class TRet>
class AFunctionPointer0ArgsRT
    : public AFunction
{
public:
	using Function = TRet ( * )();

public:
	// TBOOL: AGopherShopInterface "RemoveAllBoughtItems"
	// AInteractionEventManager*: AInteractionEventManager "GetInteractionEventManagerNonInline" (vtable 0072c838)
	// $Barnyard: FUNCTION 00499b90
	// $Barnyard: FUNCTION 005c5ce0
	static AFunctionPointer0ArgsRT* Create( const TCHAR* a_szName, Function a_fnFunction )
	{
		return new AFunctionPointer0ArgsRT( a_szName, a_fnFunction );
	}

	// $Barnyard: FUNCTION 00499c10
	// $Barnyard: FUNCTION 005c5d60
	AFunction* Clone() const override
	{
		return new AFunctionPointer0ArgsRT( *this );
	}

	// $Barnyard: FUNCTION 00499ad0
	// $Barnyard: FUNCTION 005c5bf0
	void Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override
	{
		AFunctionArg<TRet>::Set( a_pResult, m_fnFunction() );
	}

	// TBOOL: the shared 004130d0
	// $Barnyard: FUNCTION 005c5de0
	void GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override
	{
		*a_ppReturnType = AFunctionArg<TRet>::GetType();
	}

private:
	AFunctionPointer0ArgsRT( const TCHAR* a_szName, Function a_fnFunction )
	    : AFunction( a_szName )
	    , m_fnFunction( a_fnFunction )
	{}

	AFunctionPointer0ArgsRT( const AFunctionPointer0ArgsRT& a_rOther )
	    : AFunction( a_rOther )
	    , m_fnFunction( a_rOther.m_fnFunction )
	{}

private:
	Function m_fnFunction; // 0x08
};

//-----------------------------------------------------------------------------
// Binds a static function with one argument and a result to a script function name.
//-----------------------------------------------------------------------------
template <class TRet, class A1>
class AFunctionPointer1ArgsRT
    : public AFunction
{
public:
	using Function = TRet ( * )( A1 );

public:
	// TINT(TINT): AGopherShopInterface "AGopherShopInterface::GetNumBought"
	// $Barnyard: FUNCTION 00499cb0
	static AFunctionPointer1ArgsRT* Create( const TCHAR* a_szName, Function a_fnFunction )
	{
		return new AFunctionPointer1ArgsRT( a_szName, a_fnFunction );
	}

	// $Barnyard: FUNCTION 00499d30
	AFunction* Clone() const override
	{
		return new AFunctionPointer1ArgsRT( *this );
	}

	// $Barnyard: FUNCTION 00499db0
	void Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override
	{
		A1 arg1 = AFunctionArg<A1>::Get( a_pArgs->GetValue( 0 ) );
		AFunctionArg<TRet>::Set( a_pResult, m_fnFunction( arg1 ) );
	}

	// $Barnyard: FUNCTION 0049a010
	void GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override
	{
		*a_ppReturnType = AFunctionArg<TRet>::GetType();
		a_rArgTypes.Push( AFunctionArg<A1>::GetType() );
	}

private:
	AFunctionPointer1ArgsRT( const TCHAR* a_szName, Function a_fnFunction )
	    : AFunction( a_szName )
	    , m_fnFunction( a_fnFunction )
	{}

	AFunctionPointer1ArgsRT( const AFunctionPointer1ArgsRT& a_rOther )
	    : AFunction( a_rOther )
	    , m_fnFunction( a_rOther.m_fnFunction )
	{}

private:
	Function m_fnFunction; // 0x08
};

//-----------------------------------------------------------------------------
// Binds a static function with two arguments and a result to a script function name.
//-----------------------------------------------------------------------------
template <class TRet, class A1, class A2>
class AFunctionPointer2ArgsRT
    : public AFunction
{
public:
	using Function = TRet ( * )( A1, A2 );

public:
	// TBOOL(TINT, TBOOL): AInteractionManager "ForceRequirementSatisfied" (vtable 00729e90)
	// $Barnyard: FUNCTION 005a2a60
	static AFunctionPointer2ArgsRT* Create( const TCHAR* a_szName, Function a_fnFunction )
	{
		return new AFunctionPointer2ArgsRT( a_szName, a_fnFunction );
	}

	// $Barnyard: FUNCTION 005a2c90
	AFunction* Clone() const override
	{
		return new AFunctionPointer2ArgsRT( *this );
	}

	// $Barnyard: FUNCTION 005a2d10
	void Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const override
	{
		A1 arg1 = AFunctionArg<A1>::Get( a_pArgs->GetValue( 0 ) );
		A2 arg2 = AFunctionArg<A2>::Get( a_pArgs->GetValue( 1 ) );

		AFunctionArg<TRet>::Set( a_pResult, m_fnFunction( arg1, arg2 ) );
	}

	// $Barnyard: FUNCTION 005a4420
	void GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const override
	{
		*a_ppReturnType = AFunctionArg<TRet>::GetType();
		a_rArgTypes.Push( AFunctionArg<A1>::GetType() );
		a_rArgTypes.Push( AFunctionArg<A2>::GetType() );
	}

private:
	AFunctionPointer2ArgsRT( const TCHAR* a_szName, Function a_fnFunction )
	    : AFunction( a_szName )
	    , m_fnFunction( a_fnFunction )
	{}

	AFunctionPointer2ArgsRT( const AFunctionPointer2ArgsRT& a_rOther )
	    : AFunction( a_rOther )
	    , m_fnFunction( a_rOther.m_fnFunction )
	{}

private:
	Function m_fnFunction; // 0x08
};
