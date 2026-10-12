#pragma once
#include "AFunctionTable.h"

#include <Toshi/TQList.h>
#include <Toshi/TClass.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

class AFSM;
class AFSMState;
class AFSMContext;
class AFSMInstruction;

//-----------------------------------------------------------------------------
// Types of the locals of the state that is being parsed.
// NOTE: the name is made up, AFSMParser keeps one at +0x4.
//-----------------------------------------------------------------------------
struct AFSMLocalTypes
{
	static constexpr TINT MAX_NUM_LOCALS = 8;

	// $Barnyard: FUNCTION 005fde70
	AFSMLocalTypes() { Reset(); }

	// $Barnyard: FUNCTION 005fddb0
	void Reset()
	{
		for ( TINT i = 0; i < MAX_NUM_LOCALS; i++ )
			apTypes[ i ] = PPropertyValue::TYPE_UNDEF;
	}

	// $Barnyard: FUNCTION 005fde00
	const Toshi::TClass* GetType( TINT a_iIndex ) const { return apTypes[ a_iIndex ]; }

	// Returns TFALSE if the local already had a type the new one isn't compatible with
	TBOOL SetType( TINT a_iIndex, const Toshi::TClass* a_pType );

	const Toshi::TClass* apTypes[ MAX_NUM_LOCALS ]; // 0x00
};

//-----------------------------------------------------------------------------
// List of instructions (a state, the body of an if-block or a transition).
// NOTE: the name is made up, retail uses Toshi::TQList<AFSMInstruction> directly.
//-----------------------------------------------------------------------------
class AFSMInstructionList
    : public Toshi::TQList<AFSMInstruction>
{
public:
	using Callback = void ( * )( AFSMContext* a_pContext, void* a_pUserData );

public:
	AFSMInstructionList() = default;

	// $Barnyard: FUNCTION 005fe2d0
	~AFSMInstructionList() { DeleteAll(); }

	// Executes the instructions until one of them makes a transition.
	// When the context says to stay in the state, the transitions are skipped.
	void Execute( AFSMContext* a_pContext );

	// Executes the instructions and calls a_fnCallback after each of them until one makes a transition
	void Execute( AFSMContext* a_pContext, Callback a_fnCallback, void* a_pUserData );

	// Returns TFALSE if any of the instructions is not valid
	TBOOL Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals );

	// Lets the instructions release their cached data
	void Reset();

	// Deletes all instructions
	void DeleteAll();
};

//-----------------------------------------------------------------------------
// Base of the compiled FSM script instructions (vtable 0x00735df8).
// Instructions that are values leave their result in the context.
//-----------------------------------------------------------------------------
class AFSMInstruction
    : public Toshi::TQList<AFSMInstruction>::TNode
{
public:
	AFSMInstruction() = default;

	// $Barnyard: FUNCTION 00600910
	// $Barnyard: FUNCTION 006008f0
	virtual ~AFSMInstruction() = default; // +0x00

	virtual void                 Execute( AFSMContext* a_pContext )         = 0; // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) = 0; // +0x08

	// RETURNS_FALSE
	virtual TBOOL IsConstant() { return TFALSE; } // +0x0C

	// RETURNS_FALSE
	virtual TBOOL IsTransition() { return TFALSE; } // +0x10

	// DOES_NOTHING
	virtual void GetConstantValue( PPropertyValue* a_pValue ) {} // +0x14

	// DOES_NOTHING
	virtual void SetConstantValue( const PPropertyValue& a_rcValue ) {} // +0x18

	// RETURNS_TRUE
	virtual TBOOL Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) { return TTRUE; } // +0x1C

	// DOES_NOTHING
	virtual void Reset() {} // +0x20
};

//-----------------------------------------------------------------------------
// "class::function( args )" or "target->class::function( args )" (vtable 0x00735e1c).
//-----------------------------------------------------------------------------
class AFSMInstructionAction
    : public AFSMInstruction
{
public:
	AFSMInstructionAction();
	~AFSMInstructionAction();

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C

	// Takes ownership of the instructions; the constant ones are stored as values and deleted
	TBOOL SetArguments( AFSMInstruction** a_ppArguments, TINT a_iNumArguments );

	void          SetTarget( AFSMInstruction* a_pTarget ) { m_pTarget = a_pTarget; }
	AFunctionRef& GetFunction() { return m_oFunction; }

private:
	AFSMInstruction*    m_pTarget;             // 0x0C, the object to call the function on, the unit if TNULL
	AFunctionRef        m_oFunction;           // 0x10
	PPropertyValueArray m_oArguments;          // 0x18
	AFSMInstruction**   m_ppArguments;         // 0x2C, instructions of the non-constant arguments
	TINT                m_iNumArgInstructions; // 0x30
};

//-----------------------------------------------------------------------------
// "local = value" or "global = value" (vtable 0x00735e40).
//-----------------------------------------------------------------------------
class AFSMInstructionAssignment
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005febd0
	AFSMInstructionAssignment()
	    : m_iLocalIndex( -1 )
	    , m_iGlobalIndex( -1 )
	    , m_pValue( TNULL )
	{}

	~AFSMInstructionAssignment();

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C

	void SetLocalIndex( TINT a_iIndex ) { m_iLocalIndex = a_iIndex; }
	void SetGlobalIndex( TINT a_iIndex ) { m_iGlobalIndex = a_iIndex; }
	void SetValue( AFSMInstruction* a_pValue ) { m_pValue = a_pValue; }

private:
	TINT             m_iLocalIndex;  // 0x0C
	TINT             m_iGlobalIndex; // 0x10
	AFSMInstruction* m_pValue;       // 0x14
};

//-----------------------------------------------------------------------------
// "left op right" or just "left" converted to a boolean (vtable 0x00735e7c).
//-----------------------------------------------------------------------------
class AFSMInstructionComparison
    : public AFSMInstruction
{
public:
	enum OPERATOR
	{
		OPERATOR_EQUAL,
		OPERATOR_NOT_EQUAL,
		OPERATOR_LESS_OR_EQUAL,
		OPERATOR_LESS,
		OPERATOR_GREATER_OR_EQUAL,
		OPERATOR_GREATER,
		OPERATOR_NONE,
		OPERATOR_NUMOF = OPERATOR_NONE
	};

public:
	// $Barnyard: FUNCTION 005fef80
	AFSMInstructionComparison()
	    : m_pLeft( TNULL )
	    , m_pRight( TNULL )
	    , m_eOperator( OPERATOR_NONE )
	{}

	~AFSMInstructionComparison();

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C

	void SetLeft( AFSMInstruction* a_pLeft ) { m_pLeft = a_pLeft; }
	void SetRight( AFSMInstruction* a_pRight ) { m_pRight = a_pRight; }
	void SetOperator( OPERATOR a_eOperator ) { m_eOperator = a_eOperator; }

private:
	AFSMInstruction* m_pLeft;     // 0x0C
	AFSMInstruction* m_pRight;    // 0x10
	OPERATOR         m_eOperator; // 0x14
};

//-----------------------------------------------------------------------------
// "setexception( index, condition )" (vtable 0x00735ea0).
// Machines called from the state are exited once the condition is met.
//-----------------------------------------------------------------------------
class AFSMInstructionSetException
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff170
	AFSMInstructionSetException( TINT a_iIndex, AFSMInstruction* a_pCondition )
	    : m_iIndex( a_iIndex )
	    , m_pCondition( a_pCondition )
	{}

	~AFSMInstructionSetException();

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C

private:
	TINT             m_iIndex;     // 0x0C
	AFSMInstruction* m_pCondition; // 0x10
};

//-----------------------------------------------------------------------------
// "checkexception( index )" (vtable 0x00735ec4).
//-----------------------------------------------------------------------------
class AFSMInstructionCheckException
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff130
	AFSMInstructionCheckException( TINT a_iIndex )
	    : m_iIndex( a_iIndex )
	{}

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08

private:
	TINT m_iIndex; // 0x0C
};

//-----------------------------------------------------------------------------
// "clearexception( index )" (vtable 0x00735ee8).
//-----------------------------------------------------------------------------
class AFSMInstructionClearException
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff150
	AFSMInstructionClearException( TINT a_iIndex )
	    : m_iIndex( a_iIndex )
	{}

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08

private:
	TINT m_iIndex; // 0x0C
};

//-----------------------------------------------------------------------------
// "stay", the transitions after it are skipped (vtable 0x00735f0c).
//-----------------------------------------------------------------------------
class AFSMInstructionStay
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff1d0
	AFSMInstructionStay() = default;

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08
};

//-----------------------------------------------------------------------------
// "if condition { instructions }" (vtable 0x00735f30).
//-----------------------------------------------------------------------------
class AFSMInstructionIfBlock
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff5c0
	AFSMInstructionIfBlock()
	    : m_pCondition( TNULL )
	{}

	~AFSMInstructionIfBlock();

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C

	void SetCondition( AFSMInstruction* a_pCondition ) { m_pCondition = a_pCondition; }
	void AddInstruction( AFSMInstruction* a_pInstruction );

protected:
	AFSMInstruction*    m_pCondition;     // 0x0C, always met if TNULL
	AFSMInstructionList m_llInstructions; // 0x10
};

//-----------------------------------------------------------------------------
// Where a transition goes.
// NOTE: the name is made up.
//-----------------------------------------------------------------------------
struct AFSMTransitionResult
{
	// $Barnyard: FUNCTION 005fdec0
	AFSMTransitionResult()
	    : pState( TNULL )
	    , pCallMachine( TNULL )
	    , bDirect( TFALSE )
	{}

	// $Barnyard: FUNCTION 005fd360
	~AFSMTransitionResult() = default;

	AFSMTransitionResult& operator=( const AFSMTransitionResult& a_rcOther );

	AFSMState*          pState;       // 0x00, state to go to (the one to return to if a machine is called)
	AFSM*               pCallMachine; // 0x04, machine to call
	TBOOL               bDirect;      // 0x08, the new state is updated in the same frame
	PPropertyValueArray oArguments;   // 0x0C, arguments of the called machine
};

//-----------------------------------------------------------------------------
// "transition condition { instructions [direct] goto state }" or
// "transition condition { instructions [direct] call [named] machine( args ) state }" (vtable 0x00735f54).
//-----------------------------------------------------------------------------
class AFSMInstructionTransition
    : public AFSMInstructionIfBlock
{
public:
	AFSMInstructionTransition();
	~AFSMInstructionTransition();

	virtual void  Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual TBOOL IsTransition() OVERRIDE { return TTRUE; }                     // +0x10, RETURNS_TRUE
	virtual TBOOL Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x1C
	virtual void  Reset() OVERRIDE;                                             // +0x20

	void SetMachineName( TINT a_iFSMFileIndex, AFSMInstruction* a_pMachineName );
	void AddArgument( AFSMInstruction* a_pArgument );

	void SetTargetState( AFSMState* a_pState ) { m_oResult.pState = a_pState; }
	void SetCallMachine( AFSM* a_pMachine ) { m_oResult.pCallMachine = a_pMachine; }
	void SetDirect( TBOOL a_bDirect ) { m_oResult.bDirect = a_bDirect; }

private:
	static void PushArgument( AFSMContext* a_pContext, void* a_pTransition );

private:
	AFSMInstruction*     m_pMachineName;  // 0x18, "call named" expression, the name of the machine to call
	TINT                 m_iFSMFileIndex; // 0x1C
	AFSMInstructionList  m_llArguments;   // 0x20
	TINT                 m_iNumArguments; // 0x28
	AFSMTransitionResult m_oResult;       // 0x2C
};

//-----------------------------------------------------------------------------
// "myunit" (vtable 0x007361a8).
//-----------------------------------------------------------------------------
class AFSMInstructionValueCurrentUnit
    : public AFSMInstruction
{
public:
	AFSMInstructionValueCurrentUnit() = default;

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08
};

//-----------------------------------------------------------------------------
// Literal value (vtable 0x00735f78).
//-----------------------------------------------------------------------------
class AFSMInstructionValueConstant
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff870
	AFSMInstructionValueConstant( const PPropertyValue& a_rcValue )
	    : m_oValue( a_rcValue )
	{}

	// $Barnyard: FUNCTION 005ff940
	// $Barnyard: FUNCTION 005ff960
	~AFSMInstructionValueConstant() = default;

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;                  // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE;          // +0x08
	virtual TBOOL                IsConstant() OVERRIDE { return TTRUE; }                       // +0x0C, RETURNS_TRUE
	virtual void                 GetConstantValue( PPropertyValue* a_pValue ) OVERRIDE;        // +0x14
	virtual void                 SetConstantValue( const PPropertyValue& a_rcValue ) OVERRIDE; // +0x18

private:
	PPropertyValue m_oValue; // 0x0C
};

//-----------------------------------------------------------------------------
// Local variable of the state (vtable 0x00735f9c).
//-----------------------------------------------------------------------------
class AFSMInstructionValueLocal
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff8f0
	AFSMInstructionValueLocal( TINT a_iIndex )
	    : m_iIndex( a_iIndex )
	{}

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08

private:
	TINT m_iIndex; // 0x0C
};

//-----------------------------------------------------------------------------
// Variable of the machine, either an argument or a static (vtable 0x00735fc0).
//-----------------------------------------------------------------------------
class AFSMInstructionValueGlobal
    : public AFSMInstruction
{
public:
	// $Barnyard: FUNCTION 005ff910
	AFSMInstructionValueGlobal( AFSM* a_pFSM, TINT a_iIndex )
	    : m_pFSM( a_pFSM )
	    , m_iIndex( a_iIndex )
	{}

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08

private:
	AFSM* m_pFSM;   // 0x0C
	TINT  m_iIndex; // 0x10
};

//-----------------------------------------------------------------------------
// "t_machine", time since the machine was started (vtable 0x00736050).
//-----------------------------------------------------------------------------
class AFSMInstructionValueMachineTime
    : public AFSMInstruction
{
public:
	AFSMInstructionValueMachineTime() = default;

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08
};

//-----------------------------------------------------------------------------
// "t_state", time since the state was entered (vtable 0x00736074).
//-----------------------------------------------------------------------------
class AFSMInstructionValueStateTime
    : public AFSMInstruction
{
public:
	AFSMInstructionValueStateTime() = default;

	virtual void                 Execute( AFSMContext* a_pContext ) OVERRIDE;         // +0x04
	virtual const Toshi::TClass* GetType( const AFSMLocalTypes* a_pLocals ) OVERRIDE; // +0x08
};
