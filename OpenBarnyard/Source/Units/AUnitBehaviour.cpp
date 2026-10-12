#include "pch.h"
#include "AUnitBehaviour.h"
#include "AUnitBehaviourData.h"
#include "AAnimalAttributes.h"
#include "ADriveComponent.h"
#include "ADriveSystem.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "FSM/AFSMManager.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00592e00, CreateObject 00593d40, CreateObjectInPlace 00593da0
TDEFINE_CLASS_INIT( AUnitBehaviour, 0, 3 );

TPSTRING8_DECLARE( command );
TPSTRING8_DECLARE( schedule );

AFunctionTable AUnitBehaviour::ms_oFunctionTable( &TGetClass( AUnitBehaviour ), &AUnitComponent::ms_oFunctionTable );

// $Barnyard: FUNCTION 005933e0
void AUnitBehaviour::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitBehaviour, TBOOL>::Create( "SetCurrentFSMInterruptable", &AUnitBehaviour::SetCurrentFSMInterruptable ) );
}

// $Barnyard: FUNCTION 00592e10
void AUnitBehaviour::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

//-----------------------------------------------------------------------------
// AUnitBehaviour::FSMRequest
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00593410
AUnitBehaviour::FSMRequest::~FSMRequest()
{
}

// $Barnyard: FUNCTION 00593510
TBOOL AUnitBehaviour::FSMRequest::Set( const TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs )
{
	strFSM = a_rcFSM;

	if ( a_pArgs )
	{
		// NOTE: retail grows the array if needed and copies the values over the old ones, the
		// count isn't accessible here so the array is rebuilt with the same values instead
		TINT iNumArgs = a_pArgs->GetSize();

		oArgs.~PPropertyValueArray();
		new ( &oArgs ) PPropertyValueArray( iNumArgs );

		for ( TINT i = 0; i < iNumArgs; i++ )
			oArgs.Push( *a_pArgs->GetValue( i ) );

		pArgs = &oArgs;
		return TTRUE;
	}

	pArgs = TNULL;
	return TTRUE;
}

// $Barnyard: FUNCTION 00592f10
void AUnitBehaviour::FSMRequest::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( strFSM );

	TINT iNumArgs = pArgs ? pArgs->GetSize() : 0;
	a_pBuffer->WriteInt( iNumArgs );

	for ( TINT i = 0; i < iNumArgs; i++ )
	{
		a_pBuffer->WritePropertyValue( *oArgs.GetValue( i ) );
	}
}

// $Barnyard: FUNCTION 005936f0
void AUnitBehaviour::FSMRequest::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadString( strFSM );

	TINT iNumArgs = 0;
	a_pBuffer->ReadInt( iNumArgs );

	if ( iNumArgs )
	{
		pArgs = &oArgs;

		// NOTE: see Set
		oArgs.~PPropertyValueArray();
		new ( &oArgs ) PPropertyValueArray( iNumArgs );

		for ( TINT i = 0; i < iNumArgs; i++ )
		{
			PPropertyValue oValue;

			a_pBuffer->ReadPropertyValue( oValue );

			oArgs.Push( oValue );
		}
	}
	else
	{
		pArgs = TNULL;
	}
}

//-----------------------------------------------------------------------------
// AUnitBehaviour
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00593870
AUnitBehaviour::AUnitBehaviour()
    : m_pData( TNULL )
    , m_pCurrentDrive( TNULL )
    , m_pPrevDrive( TNULL )
    , m_fDriveTimer( 2.0f )
    , m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 00593470
// $Barnyard: FUNCTION 005939e0
AUnitBehaviour::~AUnitBehaviour()
{
}

// $Barnyard: FUNCTION 00593500
AFunctionTable* AUnitBehaviour::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* AUnitBehaviour::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00593250
void AUnitBehaviour::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, GetOwner()->FindComponent( &TGetClass( AAnimalAttributes ) ) );

	TFLOAT fCommand  = 0.0f;
	TFLOAT fSchedule = 0.0f;

	if ( pAttributes )
	{
		fCommand  = pAttributes->GetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND );
		fSchedule = pAttributes->GetAttribute( AAnimalAttributes::ATTRIBUTE_SCHEDULE );
	}

	// NOTE: retail writes the floats with WriteInt, the bytes are the same
	a_pBuffer->WriteFloat( fCommand );
	a_pBuffer->WriteFloat( fSchedule );

	m_oCommand.Save( a_pBuffer );
	m_oSchedule.Save( a_pBuffer );
}

// $Barnyard: FUNCTION 00593f90
void AUnitBehaviour::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	TFLOAT fCommand;
	TFLOAT fSchedule;
	a_pBuffer->ReadFloat( fCommand );
	a_pBuffer->ReadFloat( fSchedule );

	m_oCommand.Load( a_pBuffer );
	m_oSchedule.Load( a_pBuffer );

	// Restart the command, or the schedule if there was no command
	if ( fCommand != 1.0f || m_oCommand.strFSM.IsEmpty() || !SetCommand( m_oCommand.strFSM, m_oCommand.pArgs, TFALSE ) )
	{
		if ( fSchedule == 1.0f && !m_oSchedule.strFSM.IsEmpty() )
			SetSchedule( m_oSchedule.strFSM, m_oSchedule.pArgs, TFALSE );
	}
}

// $Barnyard: FUNCTION 00593a00
void AUnitBehaviour::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( GetOwner()->HasInteraction() || ( m_uiFlags & FLAGS_BEHAVIOUR_STOPPED ) )
		return;

	// NOTE: retail keeps a reference to the name of the unit's type here that is never used

	if ( m_uiFlags & FLAGS_SUGGESTION_NEW )
	{
		AAnimalAttributes::ATTRIBUTE eSuggestion = AAnimalAttributes::GetAttributeFromName( TPS8D( "suggestion" ).GetString() );
		AAnimalAttributes*           pAttributes = TSTATICCAST( AAnimalAttributes, GetOwner()->FindComponentExact( &TGetClass( AAnimalAttributes ) ) );

		if ( pAttributes )
			pAttributes->SetAttribute( eSuggestion, 1.0f );
	}

	ADriveComponent* pDriveComponent = TSTATICCAST( ADriveComponent, GetOwner()->FindComponent( &TGetClass( ADriveComponent ) ) );
	ADrive*          pDrive          = pDriveComponent ? pDriveComponent->GetCurrentDrive() : TNULL;

	m_fDriveTimer -= a_fDeltaTime;

	if ( m_fDriveTimer <= 0.0f || IsFSMFinished() )
	{
		m_fDriveTimer = 5.0f;

		if ( pDrive && ( IsFSMFinished() || m_pCurrentDrive != pDrive || ( m_uiFlags & FLAGS_SUGGESTION_NEW ) ) )
			SetDrive( pDrive );
	}

	// The natives are called on the unit
	AFunctionCaller oCaller( GetOwner() );

	m_oFSMInstance.Update( oCaller, a_fDeltaTime );
}

// $Barnyard: FUNCTION 00592e20
TBOOL AUnitBehaviour::IsFSMFinished()
{
	return m_oFSMInstance.IsFinished();
}

// $Barnyard: FUNCTION 00592e50
void AUnitBehaviour::SetCurrentFSMInterruptable( TBOOL a_bInterruptable )
{
	if ( m_oFSMInstance.GetCurrentMachine() )
		m_oFSMInstance.SetInterruptable( a_bInterruptable );
}

// $Barnyard: FUNCTION 00592f60
void AUnitBehaviour::ClearCommand()
{
	AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, GetOwner()->FindComponent( &TGetClass( AAnimalAttributes ) ) );

	if ( pAttributes )
	{
		pAttributes->SetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND, 0.0f );
		m_fDriveTimer = 0.0f;
	}
}

// $Barnyard: FUNCTION 005935b0
TBOOL AUnitBehaviour::SetDrive( ADrive* a_pDrive )
{
	if ( !m_oFSMInstance.CanChange() )
	{
		// Ask the current FSM to exit, the drive is checked again later
		m_oFSMInstance.RequestStop();
		return TFALSE;
	}

	TBOOL                      bStart = TFALSE;
	AFSM*                      pFSM   = TNULL;
	const PPropertyValueArray* pArgs  = TNULL;

	if ( a_pDrive->GetName() == TPS8D( "suggestion" ) )
	{
		if ( m_uiFlags & FLAGS_SUGGESTION_NEW )
		{
			bStart = TTRUE;
			pFSM   = FindStateMachine( m_oSuggestion.strFSM );
			pArgs  = m_oSuggestion.pArgs;
		}
	}
	else if ( a_pDrive->GetName() == TPS8( command ) )
	{
		if ( m_uiFlags & FLAGS_COMMAND_PENDING )
		{
			bStart = TTRUE;
			pFSM   = FindStateMachine( m_oCommand.strFSM );
			pArgs  = m_oCommand.pArgs;
		}
	}
	else if ( a_pDrive->GetName() == TPS8( schedule ) )
	{
		if ( m_pCurrentDrive != a_pDrive && !m_oSchedule.strFSM.IsEmpty() )
		{
			bStart = TTRUE;
			pFSM   = FindStateMachine( m_oSchedule.strFSM );
			pArgs  = m_oSchedule.pArgs;
		}
	}
	else
	{
		// Any other drive runs the FSM it's named after
		bStart = TTRUE;
		pFSM   = FindStateMachine( a_pDrive->GetName() );
	}

	if ( bStart )
	{
		// NOTE: not in retail, shows which FSMs the units start
		if ( m_pCurrentDrive != a_pDrive )
			TINFO( "AUnitBehaviour: %s takes drive %s (FSM found: %s)\n", GetOwner()->GetType()->GetName().GetString(), a_pDrive->GetName().GetString(), pFSM ? "yes" : "no" );

		m_oFSMInstance.Start( pFSM, pArgs );
	}

	m_pPrevDrive    = m_pCurrentDrive;
	m_pCurrentDrive = a_pDrive;
	m_uiFlags &= ~( FLAGS_SUGGESTION_NEW | FLAGS_COMMAND_PENDING );

	return TTRUE;
}

// $Barnyard: FUNCTION 00593bc0
TBOOL AUnitBehaviour::SetSuggestion( const TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bUnused )
{
	ADriveComponent* pDriveComponent = TSTATICCAST( ADriveComponent, GetOwner()->FindComponent( &TGetClass( ADriveComponent ) ) );
	ADrive*          pDrive          = pDriveComponent ? pDriveComponent->GetCurrentDrive() : TNULL;

#ifdef BARNYARD_COMMUNITY_PATCH
	// Units without drives can't take suggestions, retail reads the name of a null drive
	if ( !pDrive )
		return TFALSE;
#endif

	if ( pDrive->GetName() != TPS8D( "suggestion" ) )
		return TFALSE;

	if ( m_oSuggestion.strFSM != a_rcFSM )
		m_uiFlags |= FLAGS_SUGGESTION_NEW;

	m_oSuggestion.Set( a_rcFSM, a_pArgs );

	return TTRUE;
}

// $Barnyard: FUNCTION 00593c60
TBOOL AUnitBehaviour::StartRequest( FSMRequest* a_pRequest, const TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, const TPString8& a_rcDrive, TBOOL a_bSetRequest )
{
	if ( !m_oFSMInstance.CanChange() )
	{
		// Ask the current FSM to exit, the drive is checked again later
		m_oFSMInstance.RequestStop();
		return TFALSE;
	}

	if ( a_bSetRequest )
		a_pRequest->Set( a_rcFSM, a_pArgs );

	AFSM* pFSM = FindStateMachine( a_pRequest->strFSM );

	// NOTE: not in retail, shows which FSMs the units start
	TINFO( "AUnitBehaviour: %s starts %s FSM %s (found: %s)\n", GetOwner()->GetType()->GetName().GetString(), a_rcDrive.GetString(), a_pRequest->strFSM.GetString(), pFSM ? "yes" : "no" );

	m_oFSMInstance.Start( pFSM, a_pRequest->pArgs );

	m_pPrevDrive = m_pCurrentDrive;

	ADriveComponent* pDriveComponent = TSTATICCAST( ADriveComponent, GetOwner()->FindComponentExact( &TGetClass( ADriveComponent ) ) );

	if ( pDriveComponent )
		m_pCurrentDrive = pDriveComponent->GetDriveSystem()->FindDrive( a_rcDrive );
	else
		m_pCurrentDrive = TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 00593e90
TBOOL AUnitBehaviour::SetCommand( const TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bSetRequest )
{
	AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, GetOwner()->FindComponent( &TGetClass( AAnimalAttributes ) ) );

	if ( pAttributes )
		pAttributes->SetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND, 1.0f );

	TBOOL bStarted = StartRequest( &m_oCommand, a_rcFSM, a_pArgs, TPS8( command ), a_bSetRequest );

	if ( !bStarted )
	{
		// Start it once the current FSM is over
		if ( a_bSetRequest )
			m_oCommand.Set( a_rcFSM, a_pArgs );

		m_uiFlags = ( m_uiFlags & ~( FLAGS_SUGGESTION_NEW | FLAGS_COMMAND_PENDING ) ) | FLAGS_COMMAND_PENDING;
	}

	return bStarted;
}

// $Barnyard: FUNCTION 00593f20
TBOOL AUnitBehaviour::SetSchedule( const TPString8& a_rcFSM, const PPropertyValueArray* a_pArgs, TBOOL a_bSetRequest )
{
	AAnimalAttributes* pAttributes = TSTATICCAST( AAnimalAttributes, GetOwner()->FindComponent( &TGetClass( AAnimalAttributes ) ) );

	if ( pAttributes )
	{
		// Commands have priority
		if ( pAttributes->GetAttribute( AAnimalAttributes::ATTRIBUTE_COMMAND ) == 1.0f )
			return TFALSE;

		pAttributes->SetAttribute( AAnimalAttributes::ATTRIBUTE_SCHEDULE, 1.0f );
	}

	return StartRequest( &m_oSchedule, a_rcFSM, a_pArgs, TPS8( schedule ), a_bSetRequest );
}

// $Barnyard: FUNCTION 00592ef0
AFSM* AUnitBehaviour::FindStateMachine( const TPString8& a_rcName )
{
	return AFSMManager::GetSingleton()->FindStateMachine( 0, a_rcName );
}
