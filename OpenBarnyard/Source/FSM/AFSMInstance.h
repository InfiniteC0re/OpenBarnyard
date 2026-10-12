#pragma once
#include "AFSM.h"
#include "AFunctionTable.h"

#include <Toshi/TDList.h>
#include <Toshi/T2ObjectPool.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

class AFSMContext;

//-----------------------------------------------------------------------------
// Runs an AFSM for an object (AUnitBehaviour has one at +0x20).
// Machines that are called from a transition are pushed on a stack and run
// until they reach their exit end state, then the caller continues in the state
// the transition named.
//-----------------------------------------------------------------------------
class AFSMInstance
{
public:
	static constexpr TINT MAX_NUM_EXCEPTIONS = 4;
	static constexpr TINT MAX_STACK_DEPTH    = 6;

	struct Exception
	{
		AFSMInstruction* pCondition; // 0x00
		TBOOL            bRaised;    // 0x04
	};

	// Machine on the stack
	struct AFSMStackNode
	    : public Toshi::TDList<AFSMStackNode>::TNode
	{
		AFSMStackNode();
		~AFSMStackNode();

		AFSM*           pMachine;                          // 0x08
		AFSMState*      pState;                            // 0x0C
		PPropertyValue* pVariables;                        // 0x10, AFSM::MAX_NUM_VARIABLES values
		Exception       aExceptions[ MAX_NUM_EXCEPTIONS ]; // 0x14
		TFLOAT          fMachineTime;                      // 0x34
		TFLOAT          fStateTime;                        // 0x38
	};

	using StackPool = Toshi::T2ObjectPool<AFSMStackNode, MAX_STACK_DEPTH, 60, 4>;

	// Result of ProcessInstructions
	using RESULT = TINT;
	enum RESULT_ : RESULT
	{
		RESULT_RETURNED      = 0, // the called machine is over, back in the caller
		RESULT_TRANSITION    = 1, // went to a new state (and maybe called a machine)
		RESULT_NO_TRANSITION = 2, // stays in the state
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                = 0,
		FLAGS_EXITING             = BITFLAG( 0 ), // the machine went to its exit start state
		FLAGS_STOP_REQUESTED      = BITFLAG( 1 ),
		FLAGS_FINISHED            = BITFLAG( 2 ), // the machine reached its exit end state
		FLAGS_CHECKING_EXCEPTIONS = BITFLAG( 3 ), // the variables are the ones of the caller
		FLAGS_NOT_INTERRUPTABLE   = BITFLAG( 4 ),
	};

public:
	AFSMInstance();
	~AFSMInstance();

	// Stops the current machines and starts a_pMachine (if not TNULL) with the arguments
	void Start( AFSM* a_pMachine, const PPropertyValueArray* a_pArgs );

	// Executes the current state of the machine, a_rcCaller is the object the natives are called on
	void Update( const AFunctionCaller& a_rcCaller, TFLOAT a_fDeltaTime );

	// Makes the machine go to its exit start state. Returns 2 if it's already stopping.
	TINT RequestStop();

	// Returns TTRUE if a new machine can be started without waiting for the current one
	TBOOL CanChange();

	// Returns TTRUE if the machine has finished
	TBOOL IsFinished();

	void SetInterruptable( TBOOL a_bInterruptable );

	AFSM*      GetCurrentMachine();
	AFSMState* GetCurrentState();
	TFLOAT     GetMachineTime();
	TFLOAT     GetStateTime();

	PPropertyValue* GetVariable( TINT a_iIndex );
	void            SetVariable( TINT a_iIndex, const PPropertyValue& a_rcValue );

	void  SetException( TINT a_iIndex, AFSMInstruction* a_pCondition );
	TBOOL IsExceptionRaised( TINT a_iIndex );
	void  ClearException( TINT a_iIndex );

private:
	AFSMStackNode* CreateStackNode();
	AFSMStackNode* GetCallerStackNode();
	TINT           GetStackDepth() { return m_llStack.Count(); }

	void Push( AFSM* a_pMachine, const PPropertyValueArray* a_pArgs );
	void Pop();
	void ClearStack();

	// Makes the machine go to its exit start state
	TBOOL Exit();

	// Returns TTRUE if the current machine is finished
	TBOOL IsMachineFinished();

	// Returns TTRUE if the current machine can be interrupted
	TBOOL IsMachineInterruptable();

	// Executes the instructions and makes the transition. Returns 0 if a called machine
	// has finished, 1 if a transition was made and 2 if there was none.
	RESULT ProcessInstructions( AFSMInstructionList* a_pInstructions, AFSMContext* a_pContext, TBOOL a_bIgnoreTransition );

private:
	StackPool                    m_oStackPool; // 0x000
	Toshi::TDList<AFSMStackNode> m_llStack;    // 0x170
	FLAGS                        m_uiFlags;    // 0x178
};

TSTATICASSERT( sizeof( AFSMInstance::AFSMStackNode ) == 60 );
TSTATICASSERT( sizeof( AFSMInstance ) == 0x17C );
