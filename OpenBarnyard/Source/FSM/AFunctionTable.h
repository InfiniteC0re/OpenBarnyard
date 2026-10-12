#pragma once
#include <Toshi/TArray.h>
#include <Toshi/T2SortedVector.h>
#include <Toshi/TClass.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>
#include <Toshi/TString8.h>

class PPropertyValue;
class PPropertyValueArray;
class AUnit;

// Object reference handed to AFunctionTable fallbacks when a native is called from a script.
// NOTE: Layout from the fallbacks (AUnit 00412f40, ASteer 005705d0).
class AFunctionCaller
{
public:
	using KIND = TINT;
	enum KIND_ : KIND
	{
		KIND_OBJECT    = 0, // any object (AFunctionCaller( TObject* ))
		KIND_UNIT      = 1, // AFunctionCaller( AUnit* )
		KIND_COMPONENT = 2,
	};

	// NOTE: retail uses the folded constructor 005c8ef0 (see AFunctionRef)
	AFunctionCaller()
	    : iKind( KIND_OBJECT ), pObject( TNULL )
	{}

	// Any object
	// $Barnyard: FUNCTION 005c8f00
	AFunctionCaller( Toshi::TObject* a_pObject )
	    : iKind( KIND_OBJECT ), pObject( a_pObject )
	{}

	// Unit
	// $Barnyard: FUNCTION 005c8f20
	AFunctionCaller( AUnit* a_pUnit )
	    : iKind( KIND_UNIT ), pObject( TREINTERPRETCAST( Toshi::TObject*, a_pUnit ) )
	{}

	// $Barnyard: FUNCTION 005c8f40
	AFunctionCaller( const AFunctionCaller& a_rcOther )
	    : iKind( a_rcOther.iKind ), pObject( a_rcOther.pObject )
	{}

	AFunctionCaller& operator=( const AFunctionCaller& a_rcOther ) = default;

	// $Barnyard: FUNCTION 005c8f60
	Toshi::TObject* GetObject() const { return iKind == KIND_UNIT ? pObject : TNULL; }

	// $Barnyard: FUNCTION 005c8f70
	Toshi::TObject* GetComponent() const { return iKind == KIND_COMPONENT ? pObject : TNULL; }

	KIND            iKind;   // 0x00
	Toshi::TObject* pObject; // 0x04
};

class AFunction
{
public:
	AFunction( const TCHAR* a_szName );
	AFunction( const AFunction& a_rOther );

	virtual ~AFunction();                                                                                                                // +0x00
	virtual AFunction* Clone() const = 0;                                                                                                // +0x04
	virtual TBOOL      IsMethod() const { return TFALSE; }                                                                               // +0x08, RETURNS_FALSE (AMethodPointer: RETURNS_TRUE)
	virtual void       Call( Toshi::TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const       = 0; // +0x0C
	virtual void       GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const = 0; // +0x10

	const Toshi::TPString8* GetName() const { return m_pNameLower; }

protected:
	Toshi::TPString8* m_pNameLower; // 0x04
};

class AFunctionTable
    : public Toshi::TObject
    , public Toshi::TNodeList<AFunctionTable>::TNode
{
public:
	struct AFunctionSortResult
	{
		TINT operator()( AFunction* const& a_rLeft, AFunction* const& a_rRight ) const
		{
			const Toshi::TPooledString8* pLeft  = a_rLeft->GetName()->GetPooledString();
			const Toshi::TPooledString8* pRight = a_rRight->GetName()->GetPooledString();

			if ( pRight < pLeft )
				return 1;

			return -( pLeft < pRight );
		}
	};

	using FunctionContainer = Toshi::TArray<AFunction*>;
	using FunctionList      = Toshi::T2SortedVector<AFunction*, FunctionContainer, AFunctionSortResult>;

public:
	AFunctionTable( Toshi::TClass* a_pClass, AFunctionTable* a_pParentTable );
	~AFunctionTable() override;

	void Register( void* a_fnFallback );
	void Unregister();
	void AddFunction( AFunction* a_pFunction );

	Toshi::TClass*          GetFunctionClass() const { return m_pClass; }
	AFunctionTable*         GetParentTable() const { return m_pParentTable; }
	const Toshi::TPString8& GetClassNameLower() const { return m_strClassNameLower; }
	FunctionList&           GetFunctions() { return m_aFunctions; }
	const FunctionList&     GetFunctions() const { return m_aFunctions; }
	void*                   GetFallback() const { return m_fnFallback; }

	static Toshi::TNodeList<AFunctionTable>& GetRegisteredTables();

	// Returns the object the functions of this table are called on (through the fallback)
	Toshi::TObject* GetFallbackObject( const AFunctionCaller* a_pCaller ) const;

	// Returns the first function named a_rcName or TNULL
	AFunction* FindFunction( const Toshi::TPString8& a_rcName ) const;

	void        BuildSignatures() const;
	static void WalkRegisteredTables();

private:
	Toshi::TClass*   m_pClass;            // 0x10
	AFunctionTable*  m_pParentTable;      // 0x14
	Toshi::TPString8 m_strClassNameLower; // 0x18
	FunctionList     m_aFunctions;        // 0x1C
	void*            m_fnFallback;        // 0x2C
};

//-----------------------------------------------------------------------------
// A function of a registered AFunctionTable, looked up by the names.
// Used by AFSMInstructionAction (at +0x10) and AFunctionCallActivity.
//-----------------------------------------------------------------------------
struct AFunctionRef
{
	// NOTE: folded constructor that zeroes two pointers, also used for AFunctionCaller
	// $Barnyard: FUNCTION 005c8ef0
	AFunctionRef()
	    : pTable( TNULL )
	    , pFunction( TNULL )
	{}

	// Looks up the table and the function of it, TFALSE if either isn't found.
	TBOOL Find( const Toshi::TPString8& a_rcTable, const Toshi::TPString8& a_rcFunction );

	// Calls the function on the object a_rcCaller refers to (through the fallback of the table).
	// Returns TFALSE if it's a method and there's no object to call it on.
	TBOOL Call( const AFunctionCaller& a_rcCaller, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const;

	// Gets the return and argument types of the function, TFALSE if there's no function.
	TBOOL GetSignature( const Toshi::TClass** a_ppReturnType, Toshi::TArray<const Toshi::TClass*>& a_rArgTypes ) const;

	// "Table::Function", for debugging.
	Toshi::TString8 GetName() const;

	AFunctionTable* pTable;    // 0x00
	AFunction*      pFunction; // 0x04
};
