#include "pch.h"
#include "AFSMInstance.h"
#include "AFSMContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// T2ObjectPool<AFSMInstance::AFSMStackNode, 6, 60, 4> methods (vtable 0x00735dd0):
// $Barnyard: FUNCTION 005fd230
// $Barnyard: FUNCTION 005fd240
// $Barnyard: FUNCTION 005fd260
// $Barnyard: FUNCTION 005fd270
// $Barnyard: FUNCTION 005fd290

// $Barnyard: FUNCTION 005fd2b0
AFSMInstance::AFSMStackNode::AFSMStackNode()
    : pMachine( TNULL )
    , pState( TNULL )
    , fMachineTime( 0.0f )
    , fStateTime( 0.0f )
{
	for ( TINT i = 0; i < MAX_NUM_EXCEPTIONS; i++ )
	{
		aExceptions[ i ].pCondition = TNULL;
		aExceptions[ i ].bRaised    = TFALSE;
	}

	pVariables = new PPropertyValue[ AFSM::MAX_NUM_VARIABLES ];
}

// $Barnyard: FUNCTION 005fd200
AFSMInstance::AFSMStackNode::~AFSMStackNode()
{
	if ( pVariables )
		delete[] pVariables;
}

// $Barnyard: FUNCTION 005fd520
AFSMInstance::AFSMInstance()
    : m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005fd950
AFSMInstance::~AFSMInstance()
{
	m_uiFlags = FLAGS_NONE;
	ClearStack();
}

// $Barnyard: FUNCTION 005fd490
AFSMInstance::AFSMStackNode* AFSMInstance::CreateStackNode()
{
	return m_oStackPool.NewObject();
}

// $Barnyard: FUNCTION 005fd700
AFSMInstance::AFSMStackNode* AFSMInstance::GetCallerStackNode()
{
	if ( GetStackDepth() <= 1 )
		return TNULL;

	return m_llStack.GetPrev( m_llStack.Tail() ).Get();
}

// $Barnyard: FUNCTION 005fd770
void AFSMInstance::ClearStack()
{
	while ( !m_llStack.IsEmpty() )
	{
		AFSMStackNode* pNode = m_llStack.Head();
		pNode->Remove();
		m_oStackPool.DeleteObject( pNode );
	}
}

// $Barnyard: FUNCTION 005fd830
void AFSMInstance::Push( AFSM* a_pMachine, const PPropertyValueArray* a_pArgs )
{
	AFSMState*     pStartState = a_pMachine->GetStartState();
	AFSMStackNode* pNode       = CreateStackNode();

	m_llStack.InsertTail( pNode );
	pNode->pMachine = a_pMachine;
	pNode->pState   = pStartState;

	// The arguments are the first variables of the machine
	if ( a_pArgs )
	{
		for ( TINT i = 0; i < a_pArgs->GetSize(); i++ )
			pNode->pVariables[ i ] = *a_pArgs->GetValue( i );
	}

	TUtil::MemSet( pNode->aExceptions, 0, sizeof( pNode->aExceptions ) );
	pNode->fMachineTime = 0.0f;
	pNode->fStateTime   = 0.0f;
}

// $Barnyard: FUNCTION 005fd900
void AFSMInstance::Pop()
{
	m_uiFlags &= ~FLAGS_EXITING;
	m_uiFlags &= ~FLAGS_FINISHED;

	AFSMStackNode* pNode = m_llStack.Tail();
	pNode->Remove();
	m_oStackPool.DeleteObject( pNode );

	// Keep stopping the caller
	if ( m_uiFlags & FLAGS_STOP_REQUESTED )
		Exit();
}

// $Barnyard: FUNCTION 005fda40
void AFSMInstance::Start( AFSM* a_pMachine, const PPropertyValueArray* a_pArgs )
{
	m_uiFlags = FLAGS_NONE;
	ClearStack();

	if ( a_pMachine )
		Push( a_pMachine, a_pArgs );
}

// $Barnyard: FUNCTION 005fd5f0
TBOOL AFSMInstance::Exit()
{
	if ( ( m_uiFlags & ( FLAGS_EXITING | FLAGS_NOT_INTERRUPTABLE ) ) == 0 )
	{
		m_uiFlags |= FLAGS_EXITING;

		if ( !m_llStack.IsEmpty() )
		{
			AFSM* pMachine = m_llStack.Tail()->pMachine;

			if ( pMachine )
			{
				AFSMState* pExitStartState = pMachine->GetExitStartState();

				if ( pExitStartState )
				{
					m_llStack.Tail()->pState     = pExitStartState;
					m_llStack.Tail()->fStateTime = 0.0f;

					// The exit is over already if it's just one state without instructions
					if ( pMachine->GetExitEndState() == pExitStartState && pExitStartState->GetInstructions().IsEmpty() )
						m_uiFlags |= FLAGS_FINISHED;
				}
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005fd7b0
TINT AFSMInstance::RequestStop()
{
	if ( m_uiFlags & FLAGS_STOP_REQUESTED )
		return 2;

	m_uiFlags |= FLAGS_STOP_REQUESTED;
	return Exit();
}

// $Barnyard: FUNCTION 005fd660
TBOOL AFSMInstance::IsMachineInterruptable()
{
	AFSM*      pMachine = GetCurrentMachine();
	AFSMState* pState   = GetCurrentState();

	return !pMachine || !pState || !pMachine->GetExitEndState() || ( m_uiFlags & FLAGS_FINISHED );
}

// $Barnyard: FUNCTION 005fd6b0
TBOOL AFSMInstance::IsMachineFinished()
{
	AFSM*      pMachine = GetCurrentMachine();
	AFSMState* pState   = GetCurrentState();

	if ( !pMachine || !pState )
		return TTRUE;

	// Machines without exit states are finished as soon as they're stopped
	if ( pMachine->GetExitEndState() )
		return ( m_uiFlags & FLAGS_FINISHED ) != 0;

	return ( m_uiFlags & FLAGS_EXITING ) != 0;
}

// $Barnyard: FUNCTION 005fd7d0
TBOOL AFSMInstance::CanChange()
{
	return IsMachineInterruptable() && !( m_uiFlags & FLAGS_NOT_INTERRUPTABLE ) && GetStackDepth() <= 1;
}

// $Barnyard: FUNCTION 005fd800
TBOOL AFSMInstance::IsFinished()
{
	return IsMachineFinished() && GetStackDepth() <= 1;
}

// $Barnyard: FUNCTION 005fd5c0
void AFSMInstance::SetInterruptable( TBOOL a_bInterruptable )
{
	if ( a_bInterruptable )
		m_uiFlags &= ~FLAGS_NOT_INTERRUPTABLE;
	else
		m_uiFlags |= FLAGS_NOT_INTERRUPTABLE;
}

// $Barnyard: FUNCTION 005fd370
AFSM* AFSMInstance::GetCurrentMachine()
{
	return m_llStack.IsEmpty() ? TNULL : m_llStack.Tail()->pMachine;
}

// $Barnyard: FUNCTION 005fd390
AFSMState* AFSMInstance::GetCurrentState()
{
	return m_llStack.IsEmpty() ? TNULL : m_llStack.Tail()->pState;
}

// $Barnyard: FUNCTION 005fd3b0
TFLOAT AFSMInstance::GetMachineTime()
{
	return m_llStack.IsEmpty() ? 0.0f : m_llStack.Tail()->fMachineTime;
}

// $Barnyard: FUNCTION 005fd3e0
TFLOAT AFSMInstance::GetStateTime()
{
	return m_llStack.IsEmpty() ? 0.0f : m_llStack.Tail()->fStateTime;
}

// $Barnyard: FUNCTION 005fd740
PPropertyValue* AFSMInstance::GetVariable( TINT a_iIndex )
{
	// The conditions of the exceptions belong to the caller
	if ( m_uiFlags & FLAGS_CHECKING_EXCEPTIONS )
		return &GetCallerStackNode()->pVariables[ a_iIndex ];

	return &m_llStack.Tail()->pVariables[ a_iIndex ];
}

// $Barnyard: FUNCTION 005fd410
void AFSMInstance::SetVariable( TINT a_iIndex, const PPropertyValue& a_rcValue )
{
	m_llStack.Tail()->pVariables[ a_iIndex ] = a_rcValue;
}

// $Barnyard: FUNCTION 005fd430
void AFSMInstance::SetException( TINT a_iIndex, AFSMInstruction* a_pCondition )
{
	Exception& rException = m_llStack.Tail()->aExceptions[ a_iIndex ];
	rException.pCondition = a_pCondition;
	rException.bRaised    = TFALSE;
}

// $Barnyard: FUNCTION 005fd450
TBOOL AFSMInstance::IsExceptionRaised( TINT a_iIndex )
{
	return m_llStack.Tail()->aExceptions[ a_iIndex ].bRaised;
}

// $Barnyard: FUNCTION 005fd470
void AFSMInstance::ClearException( TINT a_iIndex )
{
	Exception& rException = m_llStack.Tail()->aExceptions[ a_iIndex ];
	rException.pCondition = TNULL;
	rException.bRaised    = TFALSE;
}

// $Barnyard: FUNCTION 005fd9a0
AFSMInstance::RESULT AFSMInstance::ProcessInstructions( AFSMInstructionList* a_pInstructions, AFSMContext* a_pContext, TBOOL a_bIgnoreTransition )
{
	a_pInstructions->Execute( a_pContext );

	AFSMState* pNewState    = a_pContext->GetTransition().pState;
	AFSM*      pCallMachine = a_pContext->GetTransition().pCallMachine;

	// Return to the caller once the called machine is over
	if ( GetStackDepth() > 1 && IsMachineFinished() )
	{
		Pop();
		return RESULT_RETURNED;
	}

	if ( !pNewState )
		return RESULT_NO_TRANSITION;

	if ( a_bIgnoreTransition )
	{
		a_pContext->ClearTransition();
		return RESULT_NO_TRANSITION;
	}

	m_llStack.Tail()->pState     = pNewState;
	m_llStack.Tail()->fStateTime = 0.0f;

	// The machine continues in the new state once the called machine is over
	if ( pCallMachine )
		Push( pCallMachine, &a_pContext->GetTransition().oArguments );

	return RESULT_TRANSITION;
}

// $Barnyard: FUNCTION 005fda80
void AFSMInstance::Update( const AFunctionCaller& a_rcCaller, TFLOAT a_fDeltaTime )
{
	if ( m_llStack.IsEmpty() || !m_llStack.Tail()->pMachine || !GetCurrentState() )
		return;

	TBOOL bDirect;

	do
	{
		AFSM*      pMachine     = GetCurrentMachine();
		AFSMState* pState       = GetCurrentState();
		AFSMState* pCommonState = pMachine->GetCommonState();

		if ( pMachine->GetExitStartState() == pState )
			m_uiFlags |= FLAGS_EXITING;

		if ( pMachine->GetExitEndState() == pState && pState->GetInstructions().IsEmpty() )
			m_uiFlags |= FLAGS_FINISHED;

		AFSMContext oContext( this, a_rcCaller );

		// The caller can make this machine exit with its exceptions
		Exception* pExceptions = TNULL;

		if ( GetStackDepth() > 1 )
		{
			AFSMStackNode* pCaller = GetCallerStackNode();

			if ( pCaller )
				pExceptions = pCaller->aExceptions;
		}

		bDirect                = TFALSE;
		TBOOL bExceptionRaised = TFALSE;

		if ( pExceptions && !( m_uiFlags & FLAGS_EXITING ) )
		{
			m_uiFlags |= FLAGS_CHECKING_EXCEPTIONS;

			TBOOL bRaised = TFALSE;

			for ( TINT i = 0; i < MAX_NUM_EXCEPTIONS; i++ )
			{
				if ( pExceptions[ i ].pCondition )
				{
					pExceptions[ i ].pCondition->Execute( &oContext );
					pExceptions[ i ].bRaised = oContext.GetResult().GetBoolean();
				}
				else
				{
					pExceptions[ i ].bRaised = TFALSE;
				}

				bRaised |= pExceptions[ i ].bRaised;
			}

			m_uiFlags &= ~FLAGS_CHECKING_EXCEPTIONS;

			if ( bRaised )
			{
				Exit();
				bExceptionRaised = TTRUE;
			}
		}

		if ( !bExceptionRaised )
		{
			// The transitions of the common state are checked first, except while exiting
			RESULT iResult;

			if ( !pCommonState || pMachine->GetExitStartState() == pState || pMachine->GetExitEndState() == pState ||
			     ( iResult = ProcessInstructions( &pCommonState->GetInstructions(), &oContext, TFALSE ) ) == RESULT_NO_TRANSITION )
			{
				iResult = ProcessInstructions( &pState->GetInstructions(), &oContext, TFALSE );
			}

			if ( pMachine->GetExitEndState() == pState && ( iResult == RESULT_NO_TRANSITION || iResult == RESULT_TRANSITION ) )
				m_uiFlags |= FLAGS_FINISHED;

			if ( iResult == RESULT_TRANSITION )
				bDirect = oContext.GetTransition().bDirect;
		}
	} while ( bDirect );

	AFSMStackNode* pNode = m_llStack.Tail();
	pNode->fMachineTime += a_fDeltaTime;
	pNode->fStateTime += a_fDeltaTime;
}
