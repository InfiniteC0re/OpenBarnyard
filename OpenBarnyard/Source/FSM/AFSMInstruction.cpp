#include "pch.h"
#include "AFSMInstruction.h"
#include "AFSMContext.h"
#include "AFSMInstance.h"
#include "AFSMManager.h"
#include "AFSM.h"
#include "Units/AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail TClass::IsA (006bbd60) tolerates a TNULL class, ours is a member function
static TBOOL IsTypeA( const TClass* a_pType, const TClass* a_pBase )
{
	return a_pType != TNULL && a_pType->IsA( a_pBase );
}

//-----------------------------------------------------------------------------
// AFSMLocalTypes
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005fde10
TBOOL AFSMLocalTypes::SetType( TINT a_iIndex, const TClass* a_pType )
{
	const TClass* pOldType = apTypes[ a_iIndex ];

	if ( pOldType == PPropertyValue::TYPE_UNDEF )
	{
		apTypes[ a_iIndex ] = a_pType;
		return TTRUE;
	}

	if ( a_pType == pOldType || IsTypeA( a_pType, pOldType ) )
	{
		apTypes[ a_iIndex ] = a_pType;
		return TTRUE;
	}

	apTypes[ a_iIndex ] = a_pType;
	return TFALSE;
}

//-----------------------------------------------------------------------------
// AFSMInstructionList
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005fdf00
void AFSMInstructionList::Execute( AFSMContext* a_pContext )
{
	for ( AFSMInstruction* pInstruction = Begin(); pInstruction != End(); pInstruction = pInstruction->Next() )
	{
		if ( !a_pContext->IsStaying() || !pInstruction->IsTransition() )
		{
			pInstruction->Execute( a_pContext );

			if ( a_pContext->GetTransition().pState )
				return;
		}
	}
}

// $Barnyard: FUNCTION 005fdf70
void AFSMInstructionList::Execute( AFSMContext* a_pContext, Callback a_fnCallback, void* a_pUserData )
{
	for ( AFSMInstruction* pInstruction = Begin(); pInstruction != End(); pInstruction = pInstruction->Next() )
	{
		pInstruction->Execute( a_pContext );
		a_fnCallback( a_pContext, a_pUserData );

		if ( a_pContext->GetTransition().pState )
			return;
	}
}

// $Barnyard: FUNCTION 005fdfe0
TBOOL AFSMInstructionList::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	TBOOL bValid = TTRUE;

	for ( AFSMInstruction* pInstruction = Begin(); pInstruction != End(); pInstruction = pInstruction->Next() )
		bValid &= pInstruction->Validate( a_pFSM, a_pLocals );

	return bValid;
}

// $Barnyard: FUNCTION 005fe040
void AFSMInstructionList::Reset()
{
	for ( AFSMInstruction* pInstruction = Begin(); pInstruction != End(); pInstruction = pInstruction->Next() )
		pInstruction->Reset();
}

// $Barnyard: FUNCTION 005fe090
void AFSMInstructionList::DeleteAll()
{
	while ( !IsEmpty() )
	{
		AFSMInstruction* pInstruction = Begin();
		pInstruction->Remove();
		delete pInstruction;
	}
}

//-----------------------------------------------------------------------------
// AFSMInstructionAction
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005fe6c0
AFSMInstructionAction::AFSMInstructionAction()
    : m_pTarget( TNULL )
    , m_ppArguments( TNULL )
    , m_iNumArgInstructions( 0 )
{
}

// $Barnyard: FUNCTION 005fe800
// $Barnyard: FUNCTION 005fe730
AFSMInstructionAction::~AFSMInstructionAction()
{
	if ( m_ppArguments )
	{
		TINT iNumArguments = m_iNumArgInstructions;

		for ( TINT i = 0; i < iNumArguments; i++ )
		{
			if ( m_ppArguments[ i ] )
				delete m_ppArguments[ i ];

			m_ppArguments[ i ] = TNULL;
		}

		TFree( m_ppArguments );
		m_ppArguments = TNULL;
	}

	if ( m_pTarget )
		delete m_pTarget;
}

// $Barnyard: FUNCTION 005fe340
void AFSMInstructionAction::Execute( AFSMContext* a_pContext )
{
	// Evaluate the arguments that aren't constant
	if ( m_iNumArgInstructions != 0 )
	{
		TINT iNumArguments = m_oArguments.GetSize();

		for ( TINT i = 0; i < iNumArguments; i++ )
		{
			AFSMInstruction* pArgument = m_ppArguments[ i ];

			if ( pArgument )
			{
				pArgument->Execute( a_pContext );
				*m_oArguments.GetValue( i ) = a_pContext->GetResult();
			}
		}
	}

	PPropertyValue  oResult;
	AFunctionCaller oCaller;

	if ( !m_pTarget )
	{
		oCaller = a_pContext->GetCaller();
	}
	else
	{
		m_pTarget->Execute( a_pContext );

		if ( IsTypeA( a_pContext->GetResult().GetType(), &TGetClass( AUnit ) ) )
		{
			oCaller = AFunctionCaller( TSTATICCAST( AUnit, a_pContext->GetResult().GetTObject() ) );
		}
		else if ( IsTypeA( a_pContext->GetResult().GetType(), &TGetClass( TObject ) ) )
		{
			oCaller = AFunctionCaller( a_pContext->GetResult().GetTObject() );
		}
	}

	m_oFunction.Call( oCaller, &oResult, &m_oArguments );
	a_pContext->SetResult( oResult );
}

// $Barnyard: FUNCTION 005fe470
const TClass* AFSMInstructionAction::GetType( const AFSMLocalTypes* a_pLocals )
{
	const TClass*         pReturnType;
	TArray<const TClass*> vecArgTypes( 10, 0 );

	if ( !m_oFunction.GetSignature( &pReturnType, vecArgTypes ) )
		pReturnType = PPropertyValue::TYPE_UNDEF;

	return pReturnType;
}

// $Barnyard: FUNCTION 005fe4f0
TBOOL AFSMInstructionAction::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	TBOOL bValid = TTRUE;

	if ( m_pTarget )
	{
		if ( !m_pTarget->Validate( a_pFSM, a_pLocals ) )
			return TFALSE;

		// Functions can only be called on objects
		bValid = IsTypeA( m_pTarget->GetType( a_pLocals ), &TGetClass( TObject ) );
	}

	for ( TINT i = 0; i < m_iNumArgInstructions; i++ )
	{
		AFSMInstruction* pArgument = m_ppArguments[ i ];

		if ( pArgument && !pArgument->Validate( a_pFSM, a_pLocals ) )
		{
			bValid = TFALSE;
			break;
		}
	}

	TINT                  iNumArguments = m_oArguments.GetSize();
	const TClass*         pReturnType;
	TArray<const TClass*> vecArgTypes( 10, 0 );

	if ( m_oFunction.GetSignature( &pReturnType, vecArgTypes ) )
	{
		if ( vecArgTypes.Size() == iNumArguments )
		{
			for ( TINT i = 0; i < iNumArguments; i++ )
			{
				const TClass* pType;

				if ( m_iNumArgInstructions != 0 && m_ppArguments[ i ] )
					pType = m_ppArguments[ i ]->GetType( a_pLocals );
				else
					pType = m_oArguments.GetValue( i )->GetType();

				const TClass* pExpectedType = vecArgTypes[ i ];

				// NOTE: retail formats warnings into strings that are never printed here
				if ( pType != pExpectedType && pType != PPropertyValue::TYPE_UNDEF && pExpectedType != PPropertyValue::TYPE_UNDEF &&
				     ( pExpectedType != PPropertyValue::TYPE_FLOAT || pType != PPropertyValue::TYPE_INT ) && !IsTypeA( pType, pExpectedType ) )
				{
					bValid = TFALSE;
				}
			}
		}
		else
		{
			bValid = TFALSE;
		}
	}

	return bValid;
}

// $Barnyard: FUNCTION 005fe820
TBOOL AFSMInstructionAction::SetArguments( AFSMInstruction** a_ppArguments, TINT a_iNumArguments )
{
	// Only keep the instructions if some of the arguments aren't constant
	for ( TINT i = 0; i < a_iNumArguments; i++ )
	{
		if ( !a_ppArguments[ i ]->IsConstant() )
		{
			m_ppArguments         = TSTATICCAST( AFSMInstruction*, TMalloc( sizeof( AFSMInstruction* ) * a_iNumArguments ) );
			m_iNumArgInstructions = a_iNumArguments;

			for ( TINT k = 0; k < a_iNumArguments; k++ )
				m_ppArguments[ k ] = TNULL;

			break;
		}
	}

	m_oArguments.SetSize( a_iNumArguments );

	for ( TINT i = 0; i < a_iNumArguments; i++ )
	{
		AFSMInstruction* pArgument = a_ppArguments[ i ];

		if ( pArgument->IsConstant() )
		{
			pArgument->GetConstantValue( m_oArguments.GetValue( i ) );

			if ( m_iNumArgInstructions != 0 )
				m_ppArguments[ i ] = TNULL;

			delete a_ppArguments[ i ];
		}
		else
		{
			m_ppArguments[ i ] = pArgument;
		}

		a_ppArguments[ i ] = TNULL;
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// AFSMInstructionAssignment
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005fec00
// $Barnyard: FUNCTION 005feb60
AFSMInstructionAssignment::~AFSMInstructionAssignment()
{
	if ( m_pValue )
		delete m_pValue;
}

// $Barnyard: FUNCTION 005fe900
void AFSMInstructionAssignment::Execute( AFSMContext* a_pContext )
{
	m_pValue->Execute( a_pContext );

	if ( m_iLocalIndex != -1 )
	{
		a_pContext->SetLocal( m_iLocalIndex, a_pContext->GetResult() );
	}
	else if ( m_iGlobalIndex != -1 )
	{
		a_pContext->GetInstance()->SetVariable( m_iGlobalIndex, a_pContext->GetResult() );
	}

	a_pContext->ClearResult();
}

// $Barnyard: FUNCTION 005fe8f0
const TClass* AFSMInstructionAssignment::GetType( const AFSMLocalTypes* a_pLocals )
{
	return PPropertyValue::TYPE_UNDEF;
}

// $Barnyard: FUNCTION 005fe960
TBOOL AFSMInstructionAssignment::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	if ( !m_pValue || !m_pValue->Validate( a_pFSM, a_pLocals ) )
		return TFALSE;

	const TClass* pType = m_pValue->GetType( a_pLocals );

	if ( pType == PPropertyValue::TYPE_UNDEF )
		return TFALSE;

	if ( m_iLocalIndex != -1 )
	{
		if ( m_pValue->IsConstant() )
		{
			// null can be assigned to any object
			PPropertyValue oValue;
			m_pValue->GetConstantValue( &oValue );

			if ( pType == &TGetClass( TObject ) && oValue.GetTObject() == TNULL && IsTypeA( a_pLocals->GetType( m_iLocalIndex ), &TGetClass( TObject ) ) )
				return TTRUE;
		}

		return a_pLocals->SetType( m_iLocalIndex, pType );
	}

	if ( m_iGlobalIndex != -1 )
	{
		if ( m_pValue->IsConstant() )
		{
			// null can be assigned to any object
			PPropertyValue oValue;
			m_pValue->GetConstantValue( &oValue );

			if ( pType == &TGetClass( TObject ) && oValue.GetTObject() == TNULL && IsTypeA( a_pFSM->GetVariableType( m_iGlobalIndex ), &TGetClass( TObject ) ) )
				return TTRUE;
		}

		return a_pFSM->SetVariableType( m_iGlobalIndex, pType );
	}

	return TFALSE;
}

//-----------------------------------------------------------------------------
// AFSMInstructionComparison
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005fefa0
// $Barnyard: FUNCTION 005fef00
AFSMInstructionComparison::~AFSMInstructionComparison()
{
	if ( m_pLeft )
		delete m_pLeft;

	if ( m_pRight )
		delete m_pRight;
}

// $Barnyard: FUNCTION 005fec20
void AFSMInstructionComparison::Execute( AFSMContext* a_pContext )
{
	m_pLeft->Execute( a_pContext );
	PPropertyValue oLeft( a_pContext->GetResult() );

	TBOOL bResult;

	if ( m_eOperator == OPERATOR_NONE )
	{
		bResult = oLeft.GetBoolean();
	}
	else
	{
		m_pRight->Execute( a_pContext );
		PPropertyValue oRight( a_pContext->GetResult() );

		switch ( m_eOperator )
		{
			case OPERATOR_EQUAL: bResult = oLeft == oRight; break;
			case OPERATOR_NOT_EQUAL: bResult = oLeft != oRight; break;
			case OPERATOR_LESS_OR_EQUAL: bResult = oLeft <= oRight; break;
			case OPERATOR_LESS: bResult = oLeft < oRight; break;
			case OPERATOR_GREATER_OR_EQUAL: bResult = oLeft >= oRight; break;
			case OPERATOR_GREATER: bResult = oLeft > oRight; break;
			default: bResult = TFALSE; break;
		}
	}

	a_pContext->SetResult( PPropertyValue( bResult ) );
}

// $Barnyard: FUNCTION 005fed00
const TClass* AFSMInstructionComparison::GetType( const AFSMLocalTypes* a_pLocals )
{
	return PPropertyValue::TYPE_BOOL;
}

// $Barnyard: FUNCTION 005fed10
TBOOL AFSMInstructionComparison::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	if ( m_eOperator == OPERATOR_NONE )
	{
		TBOOL bValid = m_pLeft->Validate( a_pFSM, a_pLocals );

		// Only values that can be converted to a boolean
		const TClass* pType = m_pLeft->GetType( a_pLocals );

		if ( pType == PPropertyValue::TYPE_BOOL || pType == PPropertyValue::TYPE_INT || pType == PPropertyValue::TYPE_UINT32 )
			return bValid;

		return TFALSE;
	}

	TBOOL bValid = m_pLeft->Validate( a_pFSM, a_pLocals );
	bValid &= m_pRight->Validate( a_pFSM, a_pLocals );

	const TClass* pLeftType  = m_pLeft->GetType( a_pLocals );
	const TClass* pRightType = m_pRight->GetType( a_pLocals );

	if ( pLeftType == pRightType )
		return bValid;

	// Comparing an object with null makes null an object of the same type
	TBOOL bFixed = TFALSE;

	if ( m_pRight->IsConstant() )
	{
		PPropertyValue oValue;
		m_pRight->GetConstantValue( &oValue );

		if ( oValue.GetInteger() == 0 && IsTypeA( pLeftType, &TGetClass( TObject ) ) )
		{
			oValue.SetTObject( pLeftType, TNULL );
			m_pRight->SetConstantValue( oValue );
			bFixed = TTRUE;
		}
	}
	else
	{
		if ( !m_pLeft->IsConstant() )
			return TFALSE;

		PPropertyValue oValue;
		m_pLeft->GetConstantValue( &oValue );

		if ( oValue.GetInteger() == 0 && IsTypeA( pRightType, &TGetClass( TObject ) ) )
		{
			oValue.SetTObject( pRightType, TNULL );
			m_pLeft->SetConstantValue( oValue );
			bFixed = TTRUE;
		}
	}

	if ( !bFixed )
		return TFALSE;

	return bValid;
}

//-----------------------------------------------------------------------------
// AFSMInstructionSetException
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff1a0
// $Barnyard: FUNCTION 005ff0c0
AFSMInstructionSetException::~AFSMInstructionSetException()
{
	if ( m_pCondition )
		delete m_pCondition;
}

// $Barnyard: FUNCTION 005fefc0
void AFSMInstructionSetException::Execute( AFSMContext* a_pContext )
{
	a_pContext->GetInstance()->SetException( m_iIndex, m_pCondition );
}

const TClass* AFSMInstructionSetException::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005fe8f0
	return PPropertyValue::TYPE_UNDEF;
}

// $Barnyard: FUNCTION 005fefe0
TBOOL AFSMInstructionSetException::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	if ( TUINT( m_iIndex ) >= AFSMInstance::MAX_NUM_EXCEPTIONS || !m_pCondition )
		return TFALSE;

	if ( m_pCondition->GetType( a_pLocals ) != PPropertyValue::TYPE_BOOL )
		return TFALSE;

	return m_pCondition->Validate( a_pFSM, a_pLocals );
}

//-----------------------------------------------------------------------------
// AFSMInstructionCheckException
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff030
void AFSMInstructionCheckException::Execute( AFSMContext* a_pContext )
{
	a_pContext->SetResult( PPropertyValue( a_pContext->GetInstance()->IsExceptionRaised( m_iIndex ) ) );
}

const TClass* AFSMInstructionCheckException::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005fed00
	return PPropertyValue::TYPE_BOOL;
}

//-----------------------------------------------------------------------------
// AFSMInstructionClearException
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff0a0
void AFSMInstructionClearException::Execute( AFSMContext* a_pContext )
{
	a_pContext->GetInstance()->ClearException( m_iIndex );
}

const TClass* AFSMInstructionClearException::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005fe8f0
	return PPropertyValue::TYPE_UNDEF;
}

//-----------------------------------------------------------------------------
// AFSMInstructionStay
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff1c0
void AFSMInstructionStay::Execute( AFSMContext* a_pContext )
{
	a_pContext->SetStaying();
}

const TClass* AFSMInstructionStay::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005fe8f0
	return PPropertyValue::TYPE_UNDEF;
}

//-----------------------------------------------------------------------------
// AFSMInstructionIfBlock
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff4a0
// $Barnyard: FUNCTION 005ff300
AFSMInstructionIfBlock::~AFSMInstructionIfBlock()
{
	if ( m_pCondition )
		delete m_pCondition;
}

// $Barnyard: FUNCTION 005ff1f0
void AFSMInstructionIfBlock::Execute( AFSMContext* a_pContext )
{
	if ( !m_pCondition )
	{
		m_llInstructions.Execute( a_pContext );
	}
	else
	{
		m_pCondition->Execute( a_pContext );

		if ( a_pContext->GetResult().GetBoolean() )
			m_llInstructions.Execute( a_pContext );
	}

	a_pContext->ClearResult();
}

const TClass* AFSMInstructionIfBlock::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005fe8f0
	return PPropertyValue::TYPE_UNDEF;
}

// $Barnyard: FUNCTION 005ff230
TBOOL AFSMInstructionIfBlock::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	if ( !m_pCondition )
		return m_llInstructions.Validate( a_pFSM, a_pLocals );

	if ( !m_pCondition->Validate( a_pFSM, a_pLocals ) )
		return TFALSE;

	return m_llInstructions.Validate( a_pFSM, a_pLocals );
}

// $Barnyard: FUNCTION 005ff380
void AFSMInstructionIfBlock::AddInstruction( AFSMInstruction* a_pInstruction )
{
	m_llInstructions.Push( a_pInstruction );
}

//-----------------------------------------------------------------------------
// AFSMInstructionTransition
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff5e0
AFSMInstructionTransition::AFSMInstructionTransition()
    : m_pMachineName( TNULL )
    , m_iFSMFileIndex( 1 )
    , m_iNumArguments( 0 )
{
}

// $Barnyard: FUNCTION 005ff4c0
// $Barnyard: FUNCTION 005ff3b0
AFSMInstructionTransition::~AFSMInstructionTransition()
{
	if ( m_pMachineName )
		delete m_pMachineName;
}

// $Barnyard: FUNCTION 005ff4e0
void AFSMInstructionTransition::PushArgument( AFSMContext* a_pContext, void* a_pTransition )
{
	TSTATICCAST( AFSMInstructionTransition, a_pTransition )->m_oResult.oArguments.Push( a_pContext->GetResult() );
}

// $Barnyard: FUNCTION 005ff500
void AFSMInstructionTransition::Execute( AFSMContext* a_pContext )
{
	TBOOL bConditionMet = TTRUE;

	if ( m_pCondition )
	{
		m_pCondition->Execute( a_pContext );
		bConditionMet = a_pContext->GetResult().GetBoolean();
	}

	if ( bConditionMet )
	{
		m_llInstructions.Execute( a_pContext );

		// The instructions might have made a transition already
		if ( !a_pContext->GetTransition().pState )
		{
			if ( m_pMachineName )
			{
				m_pMachineName->Execute( a_pContext );
				m_oResult.pCallMachine = AFSMManager::GetSingleton()->FindStateMachine( m_iFSMFileIndex, a_pContext->GetResult().GetTPString8() );
			}

			// Evaluate the arguments of the called machine
			m_oResult.oArguments.SetSize( m_iNumArguments );
			m_oResult.oArguments.Clear();
			m_llArguments.Execute( a_pContext, PushArgument, this );

			a_pContext->SetTransition( m_oResult );
		}
	}

	a_pContext->ClearResult();
}

// $Barnyard: FUNCTION 005ff290
TBOOL AFSMInstructionTransition::Validate( AFSM* a_pFSM, AFSMLocalTypes* a_pLocals )
{
	if ( m_pCondition && !m_pCondition->Validate( a_pFSM, a_pLocals ) )
		return TFALSE;

	if ( !m_llInstructions.Validate( a_pFSM, a_pLocals ) )
		return TFALSE;

	if ( !m_pMachineName )
		return TTRUE;

	if ( !m_pMachineName->Validate( a_pFSM, a_pLocals ) )
		return TFALSE;

	return m_pMachineName->GetType( a_pLocals ) == PPropertyValue::TYPE_TPSTRING8;
}

// $Barnyard: FUNCTION 005ff460
void AFSMInstructionTransition::Reset()
{
	m_oResult.oArguments.Resize( 0 );
}

// $Barnyard: FUNCTION 005ff270
void AFSMInstructionTransition::SetMachineName( TINT a_iFSMFileIndex, AFSMInstruction* a_pMachineName )
{
	m_pMachineName  = a_pMachineName;
	m_iFSMFileIndex = a_iFSMFileIndex;
}

// $Barnyard: FUNCTION 005ff430
void AFSMInstructionTransition::AddArgument( AFSMInstruction* a_pArgument )
{
	m_llArguments.Push( a_pArgument );
	m_iNumArguments++;
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueCurrentUnit
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff670
void AFSMInstructionValueCurrentUnit::Execute( AFSMContext* a_pContext )
{
	TObject* pObject = a_pContext->GetCaller().GetObject();

	PPropertyValue oValue;
	oValue.SetTObject( pObject );
	a_pContext->SetResult( oValue );
}

// $Barnyard: FUNCTION 005ff6f0
const TClass* AFSMInstructionValueCurrentUnit::GetType( const AFSMLocalTypes* a_pLocals )
{
	return &TGetClass( AUnit );
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueConstant
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff650
void AFSMInstructionValueConstant::Execute( AFSMContext* a_pContext )
{
	a_pContext->SetResult( m_oValue );
}

// $Barnyard: FUNCTION 005ff660
const TClass* AFSMInstructionValueConstant::GetType( const AFSMLocalTypes* a_pLocals )
{
	return m_oValue.GetType();
}

// $Barnyard: FUNCTION 005ff8d0
void AFSMInstructionValueConstant::GetConstantValue( PPropertyValue* a_pValue )
{
	*a_pValue = m_oValue;
}

// $Barnyard: FUNCTION 005ff8e0
void AFSMInstructionValueConstant::SetConstantValue( const PPropertyValue& a_rcValue )
{
	m_oValue = a_rcValue;
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueLocal
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff700
void AFSMInstructionValueLocal::Execute( AFSMContext* a_pContext )
{
	a_pContext->SetResult( *a_pContext->GetLocal( m_iIndex ) );
}

// $Barnyard: FUNCTION 005ff720
const TClass* AFSMInstructionValueLocal::GetType( const AFSMLocalTypes* a_pLocals )
{
	return a_pLocals->GetType( m_iIndex );
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueGlobal
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff730
void AFSMInstructionValueGlobal::Execute( AFSMContext* a_pContext )
{
	a_pContext->SetResult( *a_pContext->GetInstance()->GetVariable( m_iIndex ) );
}

// $Barnyard: FUNCTION 005ff750
const TClass* AFSMInstructionValueGlobal::GetType( const AFSMLocalTypes* a_pLocals )
{
	return m_pFSM->GetVariableType( m_iIndex );
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueMachineTime
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff760
void AFSMInstructionValueMachineTime::Execute( AFSMContext* a_pContext )
{
	PPropertyValue oValue;
	oValue.SetFloat( a_pContext->GetInstance()->GetMachineTime() );
	a_pContext->SetResult( oValue );
}

// $Barnyard: FUNCTION 005ff860
const TClass* AFSMInstructionValueMachineTime::GetType( const AFSMLocalTypes* a_pLocals )
{
	return PPropertyValue::TYPE_FLOAT;
}

//-----------------------------------------------------------------------------
// AFSMInstructionValueStateTime
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 005ff7e0
void AFSMInstructionValueStateTime::Execute( AFSMContext* a_pContext )
{
	PPropertyValue oValue;
	oValue.SetFloat( a_pContext->GetInstance()->GetStateTime() );
	a_pContext->SetResult( oValue );
}

const TClass* AFSMInstructionValueStateTime::GetType( const AFSMLocalTypes* a_pLocals )
{
	// 005ff860
	return PPropertyValue::TYPE_FLOAT;
}
