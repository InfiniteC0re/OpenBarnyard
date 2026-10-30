#include "pch.h"
#include "APrimeDebugFSM.h"

#ifdef PRIMEDEBUG_DEV
#  include <Render/TCameraObject.h>
#  include <Console/PConsole.h>
#  include <Console/PCFunc.h>
#  include <Console/PCVar.h>
#  include <Menu/PDebugMenu.h>

#  include "FSM/AFunctionTable.h"
#  include "FSM/AMethodPointer.h"
#  include "FSM/AMiscFunctions.h"
#  include "Debug/ADebugMenuState.h"
#  include "GameInterface/AGameStateController.h"
#  include "Helpers/ABikeManager.h"
#  include "Helpers/AGateManager.h"
#  include "Helpers/ARegrowthManager.h"
#  include "Helpers/AReplayManager.h"
#  include "Helpers/ATreasureManager.h"
#  include "Player/APlayerManager.h"
#  include "Sound/ASoundManager.h"
#  include "Units/AUnit.h"
#  include "Units/AUnitComponent.h"
#  include "Units/AUnitManager.h"
#  include "Units/AUnitNameManager.h"
#  include "Units/AUnitType.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#  include "UnitMovement/Steering/ASteer.h"

#  include <stdio.h>
#  include <stdlib.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TINT MAX_PENDING = 64;
static constexpr TINT MAX_ARGS    = 16;

// The longest a command line call waits for the world (seconds)
static constexpr TFLOAT MAX_WAIT_TIME = 600.0f;

enum CALLRESULT
{
	CALLRESULT_OK,
	CALLRESULT_ERROR,
	CALLRESULT_NOTREADY, // the object of the native (or the world) isn't there yet
};

struct PendingCall
{
	TCHAR  szLine[ PConsole::MAX_LINE ]; // the arguments of fsm.call, or a console line
	TBOOL  bConsoleLine;                 // run with PConsole::Execute
	TBOOL  bWaitForWorld;                // from the command line / the dev config: waits for the world
	TFLOAT fWaited;
};

static PendingCall s_aPending[ MAX_PENDING ];
static TINT        s_iNumPending;
static TBOOL       s_bUpdated;    // TFALSE while the command line runs (before the first frame)
static TFLOAT      s_fPlayerTime; // how long the player unit has been there
static TFLOAT      s_fDelay = 3.0f;
static TBOOL       s_bChartShown;

static PCFunc s_oCallFunc;
static PCFunc s_oListFunc;
static PCVar  s_oDelayVar;

//-----------------------------------------------------------------------------
// Helpers
//-----------------------------------------------------------------------------

static AUnit* GetPlayer()
{
	if ( !APlayerManager::IsSingletonCreated() )
		return TNULL;

	return APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
}

static TBOOL GetUnitPosition( AUnit* a_pUnit, TVector4& a_rPosition )
{
	if ( !a_pUnit || !a_pUnit->GetSteer() || !a_pUnit->GetSteer()->GetPosition() )
		return TFALSE;

	a_rPosition = *a_pUnit->GetSteer()->GetPosition();
	return TTRUE;
}

static TBOOL IsSimOnStack()
{
	return AGameStateController::IsSingletonCreated() && AGameStateController::GetSingleton()->GetNumStates() > 0 &&
	       AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ASimState" ) ) != TNULL;
}

// Fallbacks using GetSingletonSafe assert without the singleton
static TBOOL CanCallFallback( const AFunctionTable* a_pTable )
{
	// Other fallbacks look for game states
	if ( !AGameStateController::IsSingletonCreated() || AGameStateController::GetSingleton()->GetNumStates() == 0 )
		return TFALSE;

	const TCHAR* szName = a_pTable->GetClassNameLower().GetString();

	if ( !szName ) return TFALSE;
	if ( !strcmp( szName, "amiscfunctions" ) ) return AMiscFunctions::IsSingletonCreated();
	if ( !strcmp( szName, "agatemanager" ) ) return AGateManager::IsSingletonCreated();
	if ( !strcmp( szName, "aregrowthmanager" ) ) return ARegrowthManager::IsSingletonCreated();
	if ( !strcmp( szName, "areplaymanager" ) ) return AReplayManager::IsSingletonCreated();
	if ( !strcmp( szName, "atreasuremanager" ) ) return ATreasureManager::IsSingletonCreated();
	if ( !strcmp( szName, "aplayermanager" ) ) return APlayerManager::IsSingletonCreated();
	if ( !strcmp( szName, "asoundmanager" ) ) return ASoundManager::IsSingletonCreated();

	return TTRUE;
}

// The object the natives of a_pTable are called on for a_rcCaller, TNULL if none or it isn't of the class of the table
static TObject* GetTableObject( const AFunctionTable* a_pTable, const AFunctionCaller& a_rcCaller )
{
	if ( !CanCallFallback( a_pTable ) )
		return TNULL;

	TObject* pObject = a_pTable->GetFallbackObject( &a_rcCaller );

	if ( pObject && !pObject->GetClass()->IsA( a_pTable->GetFunctionClass() ) )
		return TNULL;

	return pObject;
}

static const TCHAR* GetTypeName( const TClass* a_pType )
{
	if ( a_pType == PPropertyValue::TYPE_UNDEF ) return "any";
	if ( a_pType == PPropertyValue::TYPE_INT ) return "int";
	if ( a_pType == PPropertyValue::TYPE_UINT32 ) return "uint";
	if ( a_pType == PPropertyValue::TYPE_FLOAT ) return "float";
	if ( a_pType == PPropertyValue::TYPE_BOOL ) return "bool";
	if ( a_pType == PPropertyValue::TYPE_TPSTRING8 ) return "string";

	return a_pType->GetName();
}

static void FormatSignature( const AFunctionTable* a_pTable, AFunction* a_pFunction, TCHAR* a_szBuffer, TINT a_iSize )
{
	const TClass*         pReturnType = TNULL;
	TArray<const TClass*> vecArgTypes( 10, 0 );
	a_pFunction->GetSignature( &pReturnType, vecArgTypes );

	TINT iLength = _snprintf( a_szBuffer, a_iSize, "%s::%s(", a_pTable->GetFunctionClass()->GetName(), a_pFunction->GetName()->GetString() );

	for ( TINT i = 0; i < vecArgTypes.Size() && iLength < a_iSize; i++ )
		iLength += _snprintf( a_szBuffer + iLength, a_iSize - iLength, "%s%s", i == 0 ? " " : ", ", GetTypeName( vecArgTypes[ i ] ) );

	if ( iLength < a_iSize )
		_snprintf( a_szBuffer + iLength, a_iSize - iLength, "%s) -> %s%s", vecArgTypes.Size() ? " " : "", pReturnType ? GetTypeName( pReturnType ) : "void", a_pFunction->IsMethod() ? "" : " (static)" );

	a_szBuffer[ a_iSize - 1 ] = '\0';
}

static AFunctionTable* FindTable( const TCHAR* a_szClass )
{
	TNodeList<AFunctionTable>& rTables = AFunctionTable::GetRegisteredTables();

	for ( auto it = rTables.Begin(); it != rTables.End(); it++ )
	{
		if ( it->GetClassNameLower().GetString() && !_stricmp( it->GetClassNameLower().GetString(), a_szClass ) )
			return it;
	}

	return TNULL;
}

static AFunction* FindFunction( const AFunctionTable* a_pTable, const TCHAR* a_szFunction )
{
	const AFunctionTable::FunctionList& rFunctions = a_pTable->GetFunctions();

	for ( TINT i = 0; i < rFunctions.Size(); i++ )
	{
		if ( !_stricmp( rFunctions[ i ]->GetName()->GetString(), a_szFunction ) )
			return rFunctions[ i ];
	}

	return TNULL;
}

// Does a_pUnit fit: of a_pClass (if given) and with an object for a_pTable (if given)
static TBOOL IsUnitSuitable( AUnit* a_pUnit, const TClass* a_pClass, const AFunctionTable* a_pTable )
{
	if ( a_pUnit->IsDying() )
		return TFALSE;

	if ( a_pClass && !a_pUnit->GetClass()->IsA( a_pClass ) )
		return TFALSE;

	if ( a_pTable && !GetTableObject( a_pTable, AFunctionCaller( a_pUnit ) ) )
		return TFALSE;

	return TTRUE;
}

// The suitable unit closest to the player (not the player), of the type a_szType if given
static AUnit* FindNearestUnit( const TCHAR* a_szType, const TClass* a_pClass, const AFunctionTable* a_pTable )
{
	AUnit*   pPlayer = GetPlayer();
	TVector4 vecPlayerPos;

	if ( !AUnitManager::IsSingletonCreated() || !GetUnitPosition( pPlayer, vecPlayerPos ) )
		return TNULL;

	AUnit* pNearest   = TNULL;
	TFLOAT fNearestSq = TMath::MAXFLOAT;

	TNodeList<AUnit>& rUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = rUnits.Begin(); it != rUnits.End(); it++ )
	{
		AUnit* pUnit = it;

		if ( pUnit == pPlayer )
			continue;

		if ( a_szType && ( !pUnit->GetType() || _stricmp( pUnit->GetType()->GetNameString(), a_szType ) ) )
			continue;

		TVector4 vecPos;
		if ( !GetUnitPosition( pUnit, vecPos ) || !IsUnitSuitable( pUnit, a_pClass, a_pTable ) )
			continue;

		TFLOAT fDistanceSq = TVector4::DistanceSq( vecPos, vecPlayerPos );

		if ( fDistanceSq < fNearestSq )
		{
			fNearestSq = fDistanceSq;
			pNearest   = pUnit;
		}
	}

	return pNearest;
}

// A unit by text: player, nearest, nearest:<type>, a script name or a unit type name
static AUnit* ResolveUnit( const TCHAR* a_szName, const TClass* a_pClass, const AFunctionTable* a_pTable )
{
	if ( !_stricmp( a_szName, "player" ) )
	{
		AUnit* pPlayer = GetPlayer();
		return ( pPlayer && IsUnitSuitable( pPlayer, a_pClass, a_pTable ) ) ? pPlayer : TNULL;
	}

	if ( !_stricmp( a_szName, "nearest" ) )
		return FindNearestUnit( TNULL, a_pClass, a_pTable );

	if ( !_strnicmp( a_szName, "nearest:", 8 ) )
		return FindNearestUnit( a_szName + 8, a_pClass, a_pTable );

	if ( AUnitNameManager::IsSingletonCreated() )
	{
		AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( TPString8( a_szName ) );

		if ( !pUnit )
			pUnit = AUnitNameManager::GetSingleton()->RetrieveByType( TPString8( a_szName ) );

		if ( pUnit && IsUnitSuitable( pUnit, a_pClass, a_pTable ) )
			return pUnit;
	}

	// Any unit of that type (the nearest)
	return FindNearestUnit( a_szName, a_pClass, a_pTable );
}

static TBOOL ParseBool( const TCHAR* a_szText, TBOOL& a_rbValue )
{
	if ( !_stricmp( a_szText, "1" ) || !_stricmp( a_szText, "true" ) || !_stricmp( a_szText, "on" ) || !_stricmp( a_szText, "yes" ) )
	{
		a_rbValue = TTRUE;
		return TTRUE;
	}

	if ( !_stricmp( a_szText, "0" ) || !_stricmp( a_szText, "false" ) || !_stricmp( a_szText, "off" ) || !_stricmp( a_szText, "no" ) )
	{
		a_rbValue = TFALSE;
		return TTRUE;
	}

	return TFALSE;
}

static TBOOL ParseInt( const TCHAR* a_szText, TINT& a_riValue )
{
	TCHAR* pEnd;
	long   lValue = strtol( a_szText, &pEnd, 0 );

	if ( pEnd == a_szText || *pEnd != '\0' )
		return TFALSE;

	a_riValue = TINT( lValue );
	return TTRUE;
}

static TBOOL ParseFloat( const TCHAR* a_szText, TFLOAT& a_rfValue )
{
	TCHAR* pEnd;
	double dValue = strtod( a_szText, &pEnd );

	if ( pEnd == a_szText || *pEnd != '\0' )
		return TFALSE;

	a_rfValue = TFLOAT( dValue );
	return TTRUE;
}

static void PrintResult( const TCHAR* a_szName, const PPropertyValue& a_rcResult )
{
	const TClass* pType = a_rcResult.GetType();

	if ( pType == PPropertyValue::TYPE_UNDEF )
		TINFO( "[fsm] %s done\n", a_szName );
	else if ( pType == PPropertyValue::TYPE_INT )
		TINFO( "[fsm] %s = %d\n", a_szName, a_rcResult.GetInteger() );
	else if ( pType == PPropertyValue::TYPE_UINT32 )
		TINFO( "[fsm] %s = %u\n", a_szName, a_rcResult.GetUINT32() );
	else if ( pType == PPropertyValue::TYPE_FLOAT )
		TINFO( "[fsm] %s = %f\n", a_szName, a_rcResult.GetFloat() );
	else if ( pType == PPropertyValue::TYPE_BOOL )
		TINFO( "[fsm] %s = %s\n", a_szName, a_rcResult.GetBoolean() ? "true" : "false" );
	else if ( pType == PPropertyValue::TYPE_TPSTRING8 )
		TINFO( "[fsm] %s = \"%s\"\n", a_szName, a_rcResult.GetTPString8().GetString() ? a_rcResult.GetTPString8().GetString() : "" );
	else
	{
		TObject* pObject = a_rcResult.GetTObject();

		if ( pObject && pObject->GetClass()->IsA( &TGetClass( AUnit ) ) )
		{
			AUnit* pUnit = TSTATICCAST( AUnit, pObject );
			TINFO( "[fsm] %s = %s %p (unit %d, type %s)\n", a_szName, pObject->GetClass()->GetName(), pObject, pUnit->GetId(), pUnit->GetType() ? pUnit->GetType()->GetNameString() : "?" );
		}
		else
		{
			TINFO( "[fsm] %s = %s %p\n", a_szName, pObject ? pObject->GetClass()->GetName() : pType->GetName(), pObject );
		}
	}
}

//-----------------------------------------------------------------------------
// The call
//-----------------------------------------------------------------------------

// a_ppArgv: [@unit] <Class> <Function> [args...]. a_bWaiting: missing objects are CALLRESULT_NOTREADY without an error
static CALLRESULT Call( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bWaiting )
{
	TINT         iArg     = 0;
	const TCHAR* szTarget = TNULL;

	if ( iArg < a_iArgc && a_ppArgv[ iArg ][ 0 ] == '@' )
		szTarget = a_ppArgv[ iArg++ ] + 1;

	if ( a_iArgc - iArg < 2 )
	{
		TWARN( "[fsm] usage: fsm.call [@unit] <Class> <Function> [args...]\n" );
		return CALLRESULT_ERROR;
	}

	const TCHAR* szClass    = a_ppArgv[ iArg++ ];
	const TCHAR* szFunction = a_ppArgv[ iArg++ ];

	// The tables are registered by ARootTask
	if ( AFunctionTable::GetRegisteredTables().IsEmpty() || !AGameStateController::IsSingletonCreated() )
	{
		if ( !a_bWaiting ) TWARN( "[fsm] the FSM natives aren't registered yet\n" );
		return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
	}

	AFunctionTable* pTable = FindTable( szClass );

	if ( !pTable )
	{
		TWARN( "[fsm] no FSM class '%s' (fsm.list lists them)\n", szClass );
		return CALLRESULT_ERROR;
	}

	AFunction* pFunction = FindFunction( pTable, szFunction );

	if ( !pFunction )
	{
		TWARN( "[fsm] %s has no native '%s' (fsm.list %s)\n", pTable->GetFunctionClass()->GetName(), szFunction, pTable->GetFunctionClass()->GetName() );
		return CALLRESULT_ERROR;
	}

	TCHAR szSignature[ 256 ];
	FormatSignature( pTable, pFunction, szSignature, sizeof( szSignature ) );

	const TClass*         pReturnType = TNULL;
	TArray<const TClass*> vecArgTypes( 10, 0 );
	pFunction->GetSignature( &pReturnType, vecArgTypes );

	if ( a_iArgc - iArg != vecArgTypes.Size() )
	{
		TWARN( "[fsm] %s takes %d argument(s), %d given\n", szSignature, vecArgTypes.Size(), a_iArgc - iArg );
		return CALLRESULT_ERROR;
	}

	// The object: the manager / state of the table, else a unit (the player by default)
	AFunctionCaller oCaller;
	TObject*        pObject = TNULL;

	if ( szTarget )
	{
		AUnit* pUnit = ResolveUnit( szTarget, TNULL, pFunction->IsMethod() ? pTable : TNULL );

		if ( !pUnit )
		{
			if ( !a_bWaiting ) TWARN( "[fsm] no unit '%s' with a %s\n", szTarget, pTable->GetFunctionClass()->GetName() );
			return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
		}

		oCaller = AFunctionCaller( pUnit );
		pObject = GetTableObject( pTable, oCaller );
	}
	else
	{
		pObject = GetTableObject( pTable, oCaller );

		if ( !pObject && pFunction->IsMethod() )
		{
			if ( AUnit* pPlayer = GetPlayer() )
			{
				oCaller = AFunctionCaller( pPlayer );
				pObject = GetTableObject( pTable, oCaller );
			}
		}
	}

	if ( pFunction->IsMethod() && !pObject )
	{
		if ( !a_bWaiting ) TWARN( "[fsm] no %s object to call %s on here\n", pTable->GetFunctionClass()->GetName(), szFunction );
		return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
	}

	// Natives that crash out of their context
	const TCHAR* szTableName    = pTable->GetClassNameLower().GetString();
	const TCHAR* szFunctionName = pFunction->GetName()->GetString();

	if ( !strcmp( szTableName, "atreasuremanager" ) && ( !strcmp( szFunctionName, "addgophertraps" ) || !strcmp( szFunctionName, "removegophertraps" ) ) )
	{
		if ( !AInteriorNetworkManager::IsSingletonCreated() || !AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8D( "traps" ) ) )
		{
			if ( !a_bWaiting ) TWARN( "[fsm] %s needs the 'traps' network of the main world\n", szSignature );
			return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
		}
	}
	else if ( !strcmp( szTableName, "amiscfunctions" ) && ( !strcmp( szFunctionName, "showchampionchart" ) || !strcmp( szFunctionName, "hidechampionchart" ) ) )
	{
		// Hide restores the HUD saved by Show (without it the HUD goes)
		TBOOL bShow = !strcmp( szFunctionName, "showchampionchart" );

		if ( bShow == s_bChartShown )
		{
			TWARN( "[fsm] the champion chart is %s\n", bShow ? "already shown" : "not shown by fsm.call" );
			return CALLRESULT_ERROR;
		}

		s_bChartShown = bShow;
	}
	else if ( !strcmp( szTableName, "abikemanager" ) && !strcmp( szFunctionName, "startupplayerbike" ) )
	{
		// It needs the bikes of the sim world
		if ( !IsSimOnStack() || !GetPlayer() )
		{
			if ( !a_bWaiting ) TWARN( "[fsm] %s needs the sim\n", szSignature );
			return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
		}
	}

	// The arguments
	PPropertyValueArray oArgs;

	for ( TINT i = 0; i < vecArgTypes.Size(); i++ )
	{
		const TClass* pType  = vecArgTypes[ i ];
		const TCHAR*  szText = a_ppArgv[ iArg + i ];
		TBOOL         bValid = TTRUE;

		if ( pType == PPropertyValue::TYPE_INT )
		{
			TINT iValue = 0;
			bValid      = ParseInt( szText, iValue );
			oArgs.Push( PPropertyValue( iValue ) );
		}
		else if ( pType == PPropertyValue::TYPE_UINT32 )
		{
			TINT iValue = 0;
			bValid      = ParseInt( szText, iValue );
			oArgs.Push( PPropertyValue( TUINT32( iValue ) ) );
		}
		else if ( pType == PPropertyValue::TYPE_FLOAT )
		{
			TFLOAT fValue = 0.0f;
			bValid        = ParseFloat( szText, fValue );
			oArgs.Push( PPropertyValue( fValue ) );
		}
		else if ( pType == PPropertyValue::TYPE_BOOL )
		{
			TBOOL bValue = TFALSE;
			bValid       = ParseBool( szText, bValue );
			oArgs.Push( PPropertyValue( bValue ) );
		}
		else if ( pType == PPropertyValue::TYPE_TPSTRING8 )
		{
			oArgs.Push( PPropertyValue( TPString8( szText ) ) );
		}
		else if ( pType == PPropertyValue::TYPE_UNDEF )
		{
			TINT   iValue;
			TFLOAT fValue;

			if ( ParseInt( szText, iValue ) )
				oArgs.Push( PPropertyValue( iValue ) );
			else if ( ParseFloat( szText, fValue ) )
				oArgs.Push( PPropertyValue( fValue ) );
			else
				oArgs.Push( PPropertyValue( TPString8( szText ) ) );
		}
		else if ( pType->IsA( &TGetClass( AUnit ) ) )
		{
			AUnit* pUnit = TNULL;

			if ( _stricmp( szText, "null" ) )
			{
				pUnit = ResolveUnit( szText, pType, TNULL );

				if ( !pUnit )
				{
					if ( !a_bWaiting ) TWARN( "[fsm] argument %d of %s: no %s '%s'\n", i + 1, szSignature, pType->GetName(), szText );
					return a_bWaiting ? CALLRESULT_NOTREADY : CALLRESULT_ERROR;
				}
			}

			PPropertyValue oValue;
			oValue.SetTObject( pType, pUnit );
			oArgs.Push( oValue );
		}
		else
		{
			TWARN( "[fsm] argument %d of %s: %s arguments can't be typed in\n", i + 1, szSignature, pType->GetName() );
			return CALLRESULT_ERROR;
		}

		if ( !bValid )
		{
			TWARN( "[fsm] argument %d of %s: '%s' isn't a %s\n", i + 1, szSignature, szText, GetTypeName( pType ) );
			return CALLRESULT_ERROR;
		}
	}

	// Called like AFSMInstructionAction::Execute does
	AFunctionRef oFunction;
	oFunction.pTable    = pTable;
	oFunction.pFunction = pFunction;

	PPropertyValue oResult;

	if ( !oFunction.Call( oCaller, &oResult, &oArgs ) )
	{
		TWARN( "[fsm] %s couldn't be called\n", szSignature );
		return CALLRESULT_ERROR;
	}

	TCHAR szName[ 128 ];
	_snprintf( szName, sizeof( szName ), "%s::%s%s%s", pTable->GetFunctionClass()->GetName(), szFunctionName, szTarget ? " @" : "", szTarget ? szTarget : "" );
	szName[ sizeof( szName ) - 1 ] = '\0';

	PrintResult( szName, oResult );
	return CALLRESULT_OK;
}

static TBOOL Queue( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bWaitForWorld, TBOOL a_bConsoleLine = TFALSE )
{
	if ( s_iNumPending >= MAX_PENDING )
	{
		TWARN( "[fsm] too many queued calls\n" );
		return TFALSE;
	}

	PendingCall& rCall = s_aPending[ s_iNumPending ];
	rCall.szLine[ 0 ]  = '\0';
	rCall.bConsoleLine  = a_bConsoleLine;
	rCall.bWaitForWorld = a_bWaitForWorld;
	rCall.fWaited       = 0.0f;

	// The arguments are quoted again
	TINT iLength = 0;

	for ( TINT i = 0; i < a_iArgc; i++ )
	{
		TINT iArgLength = TINT( strlen( a_ppArgv[ i ] ) );

		if ( iLength + iArgLength + 4 >= PConsole::MAX_LINE )
		{
			TWARN( "[fsm] the call is too long\n" );
			return TFALSE;
		}

		// The name of a console command stays unquoted
		iLength += sprintf( rCall.szLine + iLength, ( a_bConsoleLine && i == 0 ) ? "%s%s" : "%s\"%s\"", i ? " " : "", a_ppArgv[ i ] );
	}

	s_iNumPending++;
	return TTRUE;
}

static TBOOL CallCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	TINT    iArgc  = a_iArgc - 1;
	TCHAR** ppArgv = a_ppArgv + 1;

	if ( iArgc < 2 )
	{
		TWARN( "[fsm] usage: fsm.call [@unit] <Class> <Function> [args...]\n" );
		return TFALSE;
	}

	// The command line and the dev config run before the world: wait for it
	if ( !s_bUpdated )
	{
		if ( !Queue( iArgc, ppArgv, TTRUE ) )
			return TFALSE;

		TINFO( "[fsm] fsm.call %s waits for the world\n", s_aPending[ s_iNumPending - 1 ].szLine );
		return TTRUE;
	}

	// The game is paused under the debug menu: called once it has closed
	if ( ADebugMenuState::GetOpenMenu() )
	{
		if ( !Queue( iArgc, ppArgv, TFALSE ) )
			return TFALSE;

		if ( !ADebugMenuState::RequestAction( ADebugMenuState::ACTION_CLOSE, 0 ) )
			PDebugMenu::SetVisible( TFALSE );

		return TTRUE;
	}

	return Call( iArgc, ppArgv, TFALSE ) == CALLRESULT_OK;
}

static TBOOL ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	const TCHAR* szPrefix = a_iArgc > 1 ? a_ppArgv[ 1 ] : "";
	TSIZE        uiLength = strlen( szPrefix );
	TINT         iCount   = 0;

	TNodeList<AFunctionTable>& rTables = AFunctionTable::GetRegisteredTables();

	for ( auto it = rTables.Begin(); it != rTables.End(); it++ )
	{
		AFunctionTable* pTable      = it;
		const TCHAR*    szClassName = pTable->GetFunctionClass()->GetName();
		TBOOL           bClassMatch = !_stricmp( szClassName, szPrefix );

		const AFunctionTable::FunctionList& rFunctions = pTable->GetFunctions();

		for ( TINT i = 0; i < rFunctions.Size(); i++ )
		{
			AFunction* pFunction = rFunctions[ i ];

			// The functions of the parent tables are listed with them (unless the class is asked for)
			if ( !bClassMatch && pTable->GetParentTable() && pTable->GetParentTable()->FindFunction( *pFunction->GetName() ) )
				continue;

			TCHAR szSignature[ 256 ];
			FormatSignature( pTable, pFunction, szSignature, sizeof( szSignature ) );

			if ( !bClassMatch && _strnicmp( szSignature, szPrefix, uiLength ) )
				continue;

			TINFO( "[fsm] %s\n", szSignature );
			iCount++;
		}
	}

	TINFO( "[fsm] %d native(s)\n", iCount );
	return TTRUE;
}

//-----------------------------------------------------------------------------
// The menu
//-----------------------------------------------------------------------------

struct MenuEntry
{
	const TCHAR* szMenu;
	const TCHAR* szName;
	const TCHAR* szCommand;
};

static const MenuEntry s_aMenuEntries[] = {
	// World
	{ "World", "Open closest gate", "fsm.call AGateManager OpenClosestGate" },
	{ "World", "Add gopher traps", "fsm.call ATreasureManager AddGopherTraps" },
	{ "World", "Remove gopher traps", "fsm.call ATreasureManager RemoveGopherTraps" },
	{ "World", "Milkweed for this area", "fsm.call ALadderBeansGame EnableMilkWeedForCurrentArea" },

	{ "World/Open gates", "Barn", "fsm.call AGateManager SetGate barn 1" },
	{ "World/Open gates", "South path", "fsm.call AGateManager SetGate southpath 1" },
	{ "World/Open gates", "Woods", "fsm.call AGateManager SetGate woods 1" },
	{ "World/Open gates", "Golf", "fsm.call AGateManager SetGate golf 1" },
	{ "World/Open gates", "Honey", "fsm.call AGateManager SetGate honey 1" },
	{ "World/Open gates", "Junkyard", "fsm.call AGateManager SetGate junkyard 1" },
	{ "World/Open gates", "Old mill", "fsm.call AGateManager SetGate oldmill 1" },
	{ "World/Open gates", "Gulch", "fsm.call AGateManager SetGate gulch 1" },
	{ "World/Open gates", "Night barn", "fsm.call AGateManager SetGate nightbarn 1" },
	{ "World/Open gates", "Tomato fence", "fsm.call AGateManager SetGate tomato 1" },
	{ "World/Close gates", "Barn", "fsm.call AGateManager SetGate barn 0" },
	{ "World/Close gates", "South path", "fsm.call AGateManager SetGate southpath 0" },
	{ "World/Close gates", "Woods", "fsm.call AGateManager SetGate woods 0" },
	{ "World/Close gates", "Golf", "fsm.call AGateManager SetGate golf 0" },
	{ "World/Close gates", "Honey", "fsm.call AGateManager SetGate honey 0" },
	{ "World/Close gates", "Junkyard", "fsm.call AGateManager SetGate junkyard 0" },
	{ "World/Close gates", "Old mill", "fsm.call AGateManager SetGate oldmill 0" },
	{ "World/Close gates", "Gulch", "fsm.call AGateManager SetGate gulch 0" },
	{ "World/Close gates", "Night barn", "fsm.call AGateManager SetGate nightbarn 0" },
	{ "World/Close gates", "Tomato fence", "fsm.call AGateManager SetGate tomato 0" },

	// Player
	{ "Player", "Get on the closest bike", "fsm.call ABikeManager StartupPlayerBike" },
	{ "Player", "Can get on bikes", "fsm.call AMiscFunctions PlayerCanGetOnBike 1" },
	{ "Player", "Squirt on", "fsm.call AMiscFunctions EnablePlayerSquirt 1" },
	{ "Player", "Squirt off", "fsm.call AMiscFunctions EnablePlayerSquirt 0" },
	{ "Player", "Super squirt particles on", "fsm.call AMiscFunctions EnableSuperSquirtParticles 1" },
	{ "Player", "Super squirt particles off", "fsm.call AMiscFunctions EnableSuperSquirtParticles 0" },
	{ "Player", "Hide", "fsm.call @player AAnimatedUnit SetVisible 0" },
	{ "Player", "Show", "fsm.call @player AAnimatedUnit SetVisible 1" },

	// Nearest NPC
	{ "Nearest NPC", "Come to me", "fsm.call @nearest AInteriorSteer Command_SeekPlayer" },
	{ "Nearest NPC", "Stand still", "fsm.call @nearest ASteer Command_StandStill" },
	{ "Nearest NPC", "Cancel command", "fsm.call @nearest ASteer Command_CancelCurrentCommand" },
	{ "Nearest NPC", "Face the camera", "fsm.call @nearest ASteer FaceCamera" },
	{ "Nearest NPC", "Random facing", "fsm.call @nearest ASteer SetRandomFacing" },
	{ "Nearest NPC", "Run", "fsm.call @nearest ACharacterUnit SetDesireRun" },
	{ "Nearest NPC", "Walk", "fsm.call @nearest ACharacterUnit SetDesireWalk" },
	{ "Nearest NPC", "Hide", "fsm.call @nearest AAnimatedUnit SetVisible 0" },
	{ "Nearest NPC", "Show", "fsm.call @nearest AAnimatedUnit SetVisible 1" },
	{ "Nearest NPC", "Look around", "fsm.call @nearest AUnitFacialComponent SetLookRandom 1" },
	{ "Nearest NPC", "Look at the player", "fsm.call @nearest AUnitFacialComponent SetLookTargetByName player" },
	{ "Nearest NPC", "Stop looking", "fsm.call @nearest AUnitFacialComponent StopLook" },
	{ "Nearest NPC", "Blink off", "fsm.call @nearest AUnitFacialComponent SetBlink 0" },
	{ "Nearest NPC", "Blink on", "fsm.call @nearest AUnitFacialComponent SetBlink 1" },
	{ "Nearest NPC", "Happiness (log)", "fsm.call @nearest AAnimalAttributes GetHappiness" },
	{ "Nearest NPC", "Attributes debug toggle", "fsm.call @nearest AAnimalAttributes DebugToggle" },
	{ "Nearest NPC", "Activity location (log)", "fsm.call @nearest ANPCBehaviourComponent GetActivityLocationName" },

	// Barn (ABarnExploreMinigameState)
	{ "Barn", "Drop the ladder", "fsm.call ABarnExploreMinigameState DropLadder" },
	{ "Barn", "Music on", "fsm.call ABarnExploreMinigameState ToggleMusic 1" },
	{ "Barn", "Music off", "fsm.call ABarnExploreMinigameState ToggleMusic 0" },
	{ "Barn", "Next dance goal", "fsm.call ABarnExploreMinigameState IncDanceGoal" },

	// UI
	{ "UI", "Show champion chart", "fsm.call AMiscFunctions ShowChampionChart" },
	{ "UI", "Hide champion chart", "fsm.call AMiscFunctions HideChampionChart" },

	// Debug
	{ "Debug", "Quest tier (log)", "fsm.call AMiscFunctions GetCurAbsQuestTier" },
	{ "Debug", "Is VO playing (log)", "fsm.call AMiscFunctions IsVOPlaying" },
	{ "Debug", "List all natives (log)", "fsm.list" },
};

static void AddMenu()
{
	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "FSM" );

	TCHAR szPath[ 128 ];

	for ( TINT i = 0; i < TARRAYSIZE( s_aMenuEntries ); i++ )
	{
		const MenuEntry& rcEntry = s_aMenuEntries[ i ];

		// The submenus, one level after the other
		TCHAR szMenu[ 64 ];
		strncpy( szMenu, rcEntry.szMenu, sizeof( szMenu ) - 1 );
		szMenu[ sizeof( szMenu ) - 1 ] = '\0';

		strcpy( szPath, "./FSM" );

		for ( TCHAR* pLevel = strtok( szMenu, "/" ); pLevel; pLevel = strtok( TNULL, "/" ) )
		{
			PDebugMenu::AddSubmenu( szPath, pLevel );
			strcat( szPath, "/" );
			strcat( szPath, pLevel );
		}

		PDebugMenu::AddCommandLine( szPath, rcEntry.szName, rcEntry.szCommand );
	}
}

//-----------------------------------------------------------------------------
// APrimeDebugFSM
//-----------------------------------------------------------------------------

void APrimeDebugFSM::Register()
{
	s_oDelayVar.CreateFloatRefRange( "fsm.delay", "Seconds the fsm.call lines of the command line / the dev config wait after the player is in the world", PCVar::FLAGS_VALIDATERANGE, &s_fDelay, TNULL, TNULL, 0.0f, 600.0f, 1.0f );

	s_oCallFunc.Create( "fsm.call", "Calls an FSM script native (an AFunctionTable function, see fsm.list) like the scripts do. @unit picks the object of unit/component/steer natives (player by default): player, nearest, nearest:<type>, a script name or a unit type, unit arguments take the same. Bools: 1/0/true/false/on/off. Calls from the debug menu run once it has closed, calls of the command line once the player has been in the world for fsm.delay seconds.", "fsm.call [@unit] <Class> <Function> [args...]", CallCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	s_oListFunc.Create( "fsm.list", "Lists the FSM script natives with their argument and return types. A prefix (Class or Class::Function) limits the list, a class name lists its inherited natives too.", "fsm.list [prefix]", ListCommand, PCFunc::ANY_NUM_ARGS, TNULL );

	AddMenu();
}

void APrimeDebugFSM::Unregister()
{
	s_oListFunc.Remove();
	s_oCallFunc.Remove();
	s_oDelayVar.Remove();
	s_iNumPending = 0;
}

TBOOL APrimeDebugFSM::IsRunningCommandLine()
{
	return !s_bUpdated;
}

TBOOL APrimeDebugFSM::QueueCommand( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bWaitForWorld )
{
	if ( !Queue( a_iArgc, a_ppArgv, a_bWaitForWorld, TTRUE ) )
		return TFALSE;

	if ( a_bWaitForWorld )
	{
		TINFO( "[primedebug] %s waits for the world\n", s_aPending[ s_iNumPending - 1 ].szLine );
	}
	else if ( !ADebugMenuState::RequestAction( ADebugMenuState::ACTION_CLOSE, 0 ) )
	{
		PDebugMenu::SetVisible( TFALSE );
	}

	return TTRUE;
}

TBOOL APrimeDebugFSM::HasPendingWorldCalls()
{
	for ( TINT i = 0; i < s_iNumPending; i++ )
	{
		if ( s_aPending[ i ].bWaitForWorld )
			return TTRUE;
	}

	return TFALSE;
}

void APrimeDebugFSM::Update( TFLOAT a_fDeltaTime )
{
	s_bUpdated = TTRUE;

	if ( GetPlayer() )
		s_fPlayerTime += a_fDeltaTime;
	else
		s_fPlayerTime = 0.0f;

	// The game is paused under the menu
	if ( s_iNumPending == 0 || ADebugMenuState::GetOpenMenu() )
		return;

	for ( TINT i = 0; i < s_iNumPending; )
	{
		PendingCall& rCall = s_aPending[ i ];
		CALLRESULT   eResult;

		if ( rCall.bWaitForWorld && s_fPlayerTime < s_fDelay )
		{
			eResult = CALLRESULT_NOTREADY;
		}
		else if ( rCall.bConsoleLine )
		{
			// Out of the queue first: the command may queue again
			TCHAR szLine[ PConsole::MAX_LINE ];
			strcpy( szLine, rCall.szLine );

			for ( TINT j = i + 1; j < s_iNumPending; j++ )
				s_aPending[ j - 1 ] = s_aPending[ j ];

			s_iNumPending--;
			TINFO( "[primedebug] running %s\n", szLine );

			if ( !PConsole::Execute( szLine ) )
				TWARN( "[primedebug] '%s' failed\n", szLine );

			continue;
		}
		else
		{
			TCHAR  szLine[ PConsole::MAX_LINE ];
			TCHAR* apArgs[ MAX_ARGS ];
			TINT   iNumArgs = 0;
			strcpy( szLine, rCall.szLine );

			if ( PConsole::ParseArguments( szLine, apArgs, MAX_ARGS, &iNumArgs ) )
				eResult = Call( iNumArgs, apArgs, rCall.bWaitForWorld );
			else
				eResult = CALLRESULT_ERROR;
		}

		rCall.fWaited += a_fDeltaTime;

		if ( eResult == CALLRESULT_NOTREADY && rCall.fWaited < MAX_WAIT_TIME )
		{
			i++;
			continue;
		}

		if ( eResult == CALLRESULT_NOTREADY )
			TWARN( rCall.bConsoleLine ? "[primedebug] gave up waiting for: %s\n" : "[fsm] gave up waiting for: fsm.call %s\n", rCall.szLine );

		// Done: removed from the queue (the order is kept)
		for ( TINT j = i + 1; j < s_iNumPending; j++ )
			s_aPending[ j - 1 ] = s_aPending[ j ];

		s_iNumPending--;
	}
}

#endif // PRIMEDEBUG_DEV
