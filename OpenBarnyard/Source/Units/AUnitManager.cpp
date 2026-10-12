#include "pch.h"
#include "AUnitManager.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AUnitFactory.h"
#include "AUnitHash.h"
#include "AUIDMap.h"
#include "AUnitPlayer.h"
#include "ACharacterUnit.h"
#include "AVisibleUnit.h"
#include "AUnitActor.h"
#include "ASSeqActorData.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "AUnitRef.h"
#include "AUnitTransform.h"
#include "AUnitNameManager.h"
#include "AUnitComponentManager.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "Tasks/ARootTask.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040f410, CreateObject 00410e60, CreateObjectInPlace 00410ec0
TDEFINE_CLASS_INIT( AUnitManager, 4, 0 );

// All five live in BSS, so they start at zero and are filled in elsewhere.
TFLOAT AUnitManager::ms_fGlobalUpdateInterval   = 0.0f;
TFLOAT AUnitManager::ms_fThinkIntervalNonPlayer = 0.0f;
TFLOAT AUnitManager::ms_fThinkIntervalPlayer    = 0.0f;
TFLOAT AUnitManager::ms_fAIIntervalNonPlayer    = 0.0f;
TFLOAT AUnitManager::ms_fAIIntervalPlayer       = 0.0f;

AFunctionTable AUnitManager::ms_oFunctionTable( &TGetClass( AUnitManager ), TNULL );

// $Barnyard: FUNCTION 0049d270
TNodeList<AUnit>::Iterator AUnitManager::BeginUnits()
{
	return m_oUnits.Begin();
}

// $Barnyard: FUNCTION 0049cd80
TNodeList<AUnit>::Iterator AUnitManager::EndUnits()
{
	return m_oUnits.End();
}

// FSM fallback: AUnitManager natives always run on the singleton.
// $Barnyard: FUNCTION 0040f650
static TObject* __stdcall AUnitManagerFunctionFallback( void* a_pUnk1, void* a_pUnk2 )
{
	return AUnitManager::GetSingleton();
}

// $Barnyard: FUNCTION 00411290
void AUnitManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AUnitManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitManager, AUnit*>::Create( "DestroyUnit", &AUnitManager::DestroyUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitManager, AUnit*>::Create( "DestroyUnitNextFrame", &AUnitManager::DestroyUnitNextFrame ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<AUnit*, AUnitManager, const TPString8&>::Create( "FindUnitByTypeName", &AUnitManager::FindUnitByTypeName ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AUnitManager, AUnit*, TBOOL>::Create( "SetUnitActive", &AUnitManager::SetUnitActive ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AUnitManager, const TPString8&>::Create( "DestroyUnitsOfType", &AUnitManager::DestroyUnitsOfType ) );
}

// $Barnyard: FUNCTION 0040f420
void AUnitManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00410c10
AUnitManager::AUnitManager()
    : m_iUnk20( 0 )
    , m_pUnitFactory( TNULL )
    , m_pUnitHash( TNULL )
    , m_pComponentManagers( TNULL )
    , m_pUIDMap( TNULL )
    , m_iNextUnitId( 1 )
    , m_bUpdateAI( TTRUE )
    , m_bUpdateUnits( TTRUE )
    , m_bUpdateThink( TTRUE )
    , m_bUnk1ff( TFALSE )
    , m_bProcessPending( TFALSE )
    , m_fThinkAccumNonPlayer( 0.01f )
    , m_fAIAccumNonPlayer( 0.01f )
    , m_fGlobalUpdateAccum( 0.01f )
    , m_fThinkAccumPlayer( 0.01f )
    , m_fAIAccumPlayer( 0.01f )
    , m_iNumUnitTypes( 0 )
    , m_pUnitTypeRecords( TNULL )
    , m_iMemBlockStackIndex( 0 )
    , m_oUnitCreatedEmitter( this )
    , m_oUnitsRemovedEmitter( this )
{
}

// $Barnyard: FUNCTION 00411950
// $Barnyard: FUNCTION 004117d0
AUnitManager::~AUnitManager()
{
	RemoveAllUnits();
	UnloadUnitTypes();

	if ( m_pComponentManagers )
	{
		delete m_pComponentManagers;
		m_pComponentManagers = TNULL;
	}

	if ( m_pUnitFactory )
	{
		delete m_pUnitFactory;
		m_pUnitFactory = TNULL;
	}

	if ( m_pUnitHash )
	{
		delete m_pUnitHash;
		m_pUnitHash = TNULL;
	}

	if ( m_pUIDMap )
	{
		delete m_pUIDMap;
		m_pUIDMap = TNULL;
	}
}

// $Barnyard: FUNCTION 00410d60
AFunctionTable* AUnitManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0040f430
TBOOL AUnitManager::OnCreate()
{
	m_pUnitFactory       = new AUnitFactory();
	m_pUnitHash          = new AUnitHash();
	m_pUIDMap            = new AUIDMap();
	m_pComponentManagers = new AUnitComponentManagerList();

	m_pUnitFactory->LoadUnitTypes( "Data/Units/AllUnitTypes.trz" );
	m_pUnitFactory->LoadUnitLibraries( "Data/Units" );
	m_pUIDMap->SetOwner( this );
	m_pUnitHash->InitialiseCache( this, -400.0f, -200.0f, 400.0f, 200.0f, 50, 25 );
	OnCreateUnk();

	return TTRUE;
}

// $Barnyard: FUNCTION 004117b0
TBOOL AUnitManager::Reset()
{
	RemoveAllUnits();
	UnloadUnitTypes();

	return TTRUE;
}

// $Barnyard: FUNCTION 0040fee0
AUnitType* AUnitManager::FindUnit( const TCHAR* a_szName )
{
	return m_pUnitFactory->Find( TPString8( a_szName ) );
}

// $Barnyard: FUNCTION 00411920
AUnit* AUnitManager::CreateUnit( AUnitType* a_pType, AUnitTransform* a_pTransform )
{
	AUnit* pUnit = CreateUnitFromTemplate( a_pType, a_pTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( a_pType, a_pTransform );

	return pUnit;
}

// $Barnyard: FUNCTION 00411970
AUnit* AUnitManager::SpawnUnit( const TCHAR* a_szTypeName, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	AUnitTransform oTransform( a_rcPosition, a_rcDirection, 0, -1, -1, TTRUE );
	AUnitType*     pType = FindUnit( a_szTypeName );

	AUnit* pUnit = CreateUnitFromTemplate( pType, &oTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( pType, &oTransform );

	return pUnit;
}

// $Barnyard: FUNCTION 004119d0
AUnit* AUnitManager::SpawnUnit( const TPString8& a_rcTypeName, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	AUnitTransform oTransform( a_rcPosition, a_rcDirection, 0, -1, -1, TTRUE );
	AUnitType*     pType = m_pUnitFactory->Find( a_rcTypeName );

	AUnit* pUnit = CreateUnitFromTemplate( pType, &oTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( pType, &oTransform );

	return pUnit;
}

// $Barnyard: FUNCTION 00411a30
AUnit* AUnitManager::SpawnUnit( const TPString8& a_rcTypeName, AUnitTransform* a_pTransform )
{
	AUnitType* pType = m_pUnitFactory->Find( a_rcTypeName );
	AUnit*     pUnit = CreateUnitFromTemplate( pType, a_pTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( pType, a_pTransform );

	return pUnit;
}

// $Barnyard: FUNCTION 00411a70
AUnit* AUnitManager::SpawnUnit( AUnitType* a_pType, const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	AUnitTransform oTransform( a_rcPosition, a_rcDirection, 0, -1, -1, TTRUE );
	AUnit*         pUnit = CreateUnitFromTemplate( a_pType, &oTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( a_pType, &oTransform );

	return pUnit;
}

// $Barnyard: FUNCTION 00411ac0
AUnit* AUnitManager::SpawnUnitAtGoal( AUnitType* a_pType, AUnitTransform* a_pTransform, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface )
{

	TVector4 vecGoalPos;
	TBOOL    bHasHeight = a_pGoal->GetPosition( vecGoalPos );

	if ( !a_pGoal->m_vForward.IsEqual( TVector4::VEC_ZERO ) )
		a_pTransform->m_vecDirection = a_pGoal->m_vForward;

	TFLOAT fOriginalY = a_pTransform->m_vecPosition.y;

	// The transform's position is an offset from the goal
	vecGoalPos.x += a_pTransform->m_vecPosition.x;
	vecGoalPos.y += a_pTransform->m_vecPosition.y;
	vecGoalPos.z += a_pTransform->m_vecPosition.z;

	if ( !bHasHeight )
		vecGoalPos.y = -100.0f;

	a_pTransform->m_vecPosition = vecGoalPos;

	AUnit* pUnit = CreateUnitFromTemplate( a_pType, a_pTransform );

	if ( !pUnit )
		pUnit = CreateUnitInstance( a_pType, a_pTransform );

	// Steers that walk the interior network get the network and goal directly
	if ( pUnit->m_pSteer->IsA( &TGetClass( APlayerSteer ) ) || pUnit->m_pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
	{
		ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, pUnit->m_pSteer );
		pSteer->SetNetwork( a_pNetwork, 0 );
		pSteer->SetGoal( a_pGoal );

		return pUnit;
	}

	if ( bHasHeight )
	{
		pUnit->SetSteerPosition( vecGoalPos );

		// NOTE: retail sets the position on the steer a second time
		pUnit->m_pSteer->SetPosition( vecGoalPos );

		return pUnit;
	}

	if ( a_pInterface )
		a_pNetwork->InsertGoalInterface( a_pInterface, a_pGoal );

	if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
	{
		a_pNetwork->PushGoalInterface( TSTATICCAST( AVisibleUnit, pUnit )->GetGoalInterface(), a_pGoal );
		TSTATICCAST( AVisibleUnit, pUnit )->SetGoalYOffset( fOriginalY );
	}

	return pUnit;
}

// $Barnyard: FUNCTION 00411c80
AUnit* AUnitManager::SpawnUnitAtGoal( const TPString8& a_rcTypeName, AUnitTransform* a_pTransform, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface )
{
	return SpawnUnitAtGoal( m_pUnitFactory->Find( a_rcTypeName ), a_pTransform, a_pNetwork, a_pGoal, a_pInterface );
}

// $Barnyard: FUNCTION 00411cb0
AUnit* AUnitManager::SpawnUnitAtGoal( const TPString8& a_rcTypeName, const TVector4& a_rcOffset, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface )
{
	AUnitTransform oTransform( a_rcOffset, TVector4::VEC_POSX, 0, -1, -1, TTRUE );

	return SpawnUnitAtGoal( m_pUnitFactory->Find( a_rcTypeName ), &oTransform, a_pNetwork, a_pGoal, a_pInterface );
}

// $Barnyard: FUNCTION 00411d10
AUnit* AUnitManager::SpawnUnitAtGoal( const TPString8& a_rcTypeName, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface )
{
	AUnitType*     pType = m_pUnitFactory->Find( a_rcTypeName );
	AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, -1, -1, TTRUE );

	return SpawnUnitAtGoal( pType, &oTransform, a_pNetwork, a_pGoal, a_pInterface );
}

// $Barnyard: FUNCTION 0040f570
AUnitHash::QueryResult* AUnitManager::FindUnits( const TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, TBOOL a_bAllUnits, AUnitHash::FilterFn a_fnFilter )
{
	return m_pUnitHash->FindUnits( a_rcPos, a_fRadius, a_pNumFound, a_b3D, a_iMaxNum, a_bAllUnits, a_fnFilter );
}

// $Barnyard: FUNCTION 0040f6b0
AUnit* AUnitManager::FindClosestUnit( const TVector4& a_rcPos, AUnitType* a_pType, TFLOAT a_fRadius, AUnit* a_pExclude )
{
	AUnit* pClosest   = TNULL;
	TFLOAT fClosestSq = TMath::MAXFLOAT;

	// NOTE: retail re-reads the singleton instead of using this
	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = GetSingleton()->m_pUnitHash->FindUnits( a_rcPos, a_fRadius, &iNumFound, TFALSE, AUnitHash::MAX_RESULTS, TFALSE, TNULL );

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit == a_pExclude )
			continue;

		if ( a_pType && pUnit->m_pType != a_pType )
			continue;

		TFLOAT fDistSq = TVector4::DistanceSq( *pUnit->m_pSteer->GetPosition(), a_rcPos );

		if ( !pClosest || fDistSq < fClosestSq )
		{
			if ( !pUnit->IsA( &TGetClass( AVisibleUnit ) ) || TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			{
				fClosestSq = fDistSq;
				pClosest   = pUnit;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 0040fc90
AUnit* AUnitManager::FindNextUnitOfType( AUnitType* a_pType, TNodeList<AUnit>::Iterator& a_rIt )
{
	while ( a_rIt != m_oUnits.End() )
	{
		AUnit* pUnit = a_rIt;
		a_rIt++;

		if ( !pUnit->IsDying() && pUnit->m_pType == a_pType )
			return pUnit;
	}

	a_rIt = m_oUnits.End();
	return TNULL;
}

// $Barnyard: FUNCTION 00410200
AUnit* AUnitManager::FindUnitById( TINT a_iId )
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
	{
		if ( it->m_iId == a_iId )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00410250
AUnit* AUnitManager::FindUnitByTypeName( const TPString8& a_rcTypeName )
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
	{
		if ( it->m_pType->GetName() == a_rcTypeName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00410740
AUnit* AUnitManager::FindActorBySSeqId( TINT a_iId )
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
	{
		if ( it->IsA( &TGetClass( AUnitActor ) ) )
		{
			ASSeqActorData* pData = TSTATICCAST( ASSeqActorData, it->m_pType->FindStaticData( &TGetClass( ASSeqActorData ) ) );

			// NOTE: no TNULL check in retail
			if ( pData->GetActorIndex() == a_iId )
				return it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00410160
TINT AUnitManager::GetTypeInstanceCount( const TCHAR* a_szTypeName )
{
	AUnitType* pType = m_pUnitFactory->Find( TPString8( a_szTypeName ) );

	if ( pType )
		return pType->GetInstanceCount();

	return 0;
}

// $Barnyard: FUNCTION 0040f580
TBOOL AUnitManager::ClearUIDMap()
{
	m_pUIDMap->ThrowClearEvent();
	return TTRUE;
}

// $Barnyard: FUNCTION 0040fd70
void AUnitManager::UnloadLibrary( const TCHAR* a_szLibName )
{
	UnloadUnitTypes();
}

// $Barnyard: FUNCTION 0040f590
void AUnitManager::DestroyUnitNextFrame( AUnit* a_pUnit )
{
	m_bProcessPending = TTRUE;
	a_pUnit->SetPendingRemoval( TTRUE );
}

// $Barnyard: FUNCTION 0040f660
void AUnitManager::SetUnitActive( AUnit* a_pUnit, TBOOL a_bActive )
{
	a_pUnit->SetExternalUpdate( a_bActive );
	a_pUnit->SetMovement( a_bActive );
	a_pUnit->SetAIEnabled( a_bActive );

	if ( a_pUnit->IsA( &TGetClass( AVisibleUnit ) ) )
		TSTATICCAST( AVisibleUnit, a_pUnit )->SetVisible( a_bActive );
}

// $Barnyard: FUNCTION 00410fe0
void AUnitManager::DestroyUnit( AUnit* a_pUnit )
{
	if ( !RecycleUnit( a_pUnit ) )
		DestroyUnitImmediate( a_pUnit );
}

// $Barnyard: FUNCTION 00411000
void AUnitManager::DestroyUnitsOfType( const TPString8& a_rcTypeName )
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); )
	{
		AUnit* pUnit = it;

		if ( pUnit->m_pType->GetName() == a_rcTypeName )
		{
			// Units destroyed by type lose their script name too
			TPString8 strName = AUnitNameManager::GetSingleton()->GetUnitName( pUnit->m_iId );

			if ( strName.GetPooledString() && strName.GetString8().Length() != 0 )
				AUnitNameManager::GetSingleton()->RemoveName( strName );

			if ( !RecycleUnit( pUnit ) )
				DestroyUnitImmediate( pUnit );
		}

		// NOTE: retail reads the next pointer after recycling/destroying the unit
		it = pUnit->Next();
	}
}

// $Barnyard: FUNCTION 0040ffb0
void AUnitManager::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bUpdateUnits != TTRUE )
		return;

	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
	{
		if ( !it->IsDying() && ( it->GetFlags() & AUnit::FLAGS_EXTERNAL_UPDATE ) )
			it->Update( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 0040ff60
void AUnitManager::SetUnitsQueryStamp( TUINT a_uiStamp )
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
	{
		if ( !it->IsDying() )
			it->SetQueryStamp( a_uiStamp );
	}
}

// $Barnyard: FUNCTION 00411410
TBOOL AUnitManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	ProcessPendingUnits();

	// Nothing updates while the game is paused or mid-transition
	if ( ARootTask::GetSingleton()->IsPaused() || ARootTask::GetSingleton()->IsTransitioning() == TTRUE )
		return TTRUE;

	// AI pass - players and everyone else run on their own cadence
	if ( m_bUpdateAI )
	{
		m_fAIAccumNonPlayer += a_fDeltaTime;
		m_fAIAccumPlayer += a_fDeltaTime;

		for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
		{
			if ( !( it->GetFlags() & AUnit::FLAGS_AI_ENABLED ) )
				continue;

			if ( it->IsA( &TGetClass( AUnitPlayer ) ) )
			{
				if ( ms_fAIIntervalPlayer < m_fAIAccumPlayer )
					it->UpdateAI( m_fAIAccumPlayer );
			}
			else if ( ms_fAIIntervalNonPlayer < m_fAIAccumNonPlayer )
			{
				it->UpdateAI( m_fAIAccumNonPlayer );
			}
		}

		if ( ms_fAIIntervalNonPlayer < m_fAIAccumNonPlayer )
			m_fAIAccumNonPlayer = 0.0f;

		if ( ms_fAIIntervalPlayer < m_fAIAccumPlayer )
			m_fAIAccumPlayer = 0.0f;
	}

	// Think pass - units that move during a think get re-hashed
	if ( m_bUpdateThink == TTRUE )
	{
		m_fThinkAccumNonPlayer += a_fDeltaTime;
		m_fThinkAccumPlayer += a_fDeltaTime;

		for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
		{
			AUnit* pUnit = it;

			if ( pUnit->IsDying() )
				continue;

			TFLOAT fAccum;

			if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
			{
				if ( !( ms_fThinkIntervalPlayer < m_fThinkAccumPlayer ) || !( pUnit->GetFlags() & AUnit::FLAGS_THINK_ENABLED ) )
					continue;

				fAccum = m_fThinkAccumPlayer;
			}
			else
			{
				if ( !( ms_fThinkIntervalNonPlayer < m_fThinkAccumNonPlayer ) || !( pUnit->GetFlags() & AUnit::FLAGS_THINK_ENABLED ) )
					continue;

				fAccum = m_fThinkAccumNonPlayer;
			}

			if ( pUnit->Think( fAccum ) && ( pUnit->GetFlags() & AUnit::FLAGS_HASHED ) )
				ReinsertUnitInHash( pUnit );
		}

		if ( ms_fThinkIntervalNonPlayer < m_fThinkAccumNonPlayer )
			m_fThinkAccumNonPlayer = 0.0f;

		if ( ms_fThinkIntervalPlayer < m_fThinkAccumPlayer )
			m_fThinkAccumPlayer = 0.0f;
	}

	// Per-frame update
	if ( m_bUpdateUnits == TTRUE )
	{
		for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); it++ )
		{
			if ( !( it->GetFlags() & ( AUnit::FLAGS_DYING | AUnit::FLAGS_EXTERNAL_UPDATE ) ) )
				it->OnUpdate( a_fDeltaTime );
		}
	}

	DeleteDyingUnits();

	// Drive the component managers on the global cadence
	m_fGlobalUpdateAccum += a_fDeltaTime;

	if ( ms_fGlobalUpdateInterval < m_fGlobalUpdateAccum )
	{
		m_pComponentManagers->Update( m_fGlobalUpdateAccum );
		m_fGlobalUpdateAccum = 0.0f;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00411700
AUnit* AUnitManager::CreateUnitFromTemplate( AUnitType* a_pType, AUnitTransform* a_pTransform )
{
	if ( m_iNumUnitTypes == 0 || !m_oUnitTypesTRB.GetSymbolAddress( a_pType->GetName() ) )
		return TNULL;

	// Construct the unit inside its pool slot's memory block
	UnitPoolEntry* pEntry = AcquireUnitFromPool( a_pType->GetName() );
	AUnit*         pUnit  = CreateUnitInstance( a_pType, a_pTransform );
	pEntry->pUnit         = pUnit;

	if ( --m_iMemBlockStackIndex < 0 )
		m_iMemBlockStackIndex = 0;

	g_pMemory->SetGlobalBlock( m_apMemBlockStack[ m_iMemBlockStackIndex ] );

	return pUnit;
}

// $Barnyard: FUNCTION 00410d70
AUnit* AUnitManager::CreateUnitInstance( AUnitType* a_pType, AUnitTransform* a_pTransform )
{
	AUnitType* pType = m_pUnitFactory->Find( a_pType->GetName() );
	AUnit*     pUnit = pType ? m_pUnitFactory->CreateInstance( pType ) : TNULL;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original dereferences TNULL here if the type or its class is missing
	if ( !pUnit )
		return TNULL;
#endif

	pUnit->m_iId = m_iNextUnitId++;
	m_pUIDMap->Store( pUnit, pUnit->m_oSelfProxy );
	m_oUnits.InsertTail( pUnit );

	if ( !pUnit->Create( a_pTransform ) )
	{
		m_oUnits.Remove( pUnit );
		delete pUnit;

		return TNULL;
	}

	OnUnitCreated( pUnit );

	return pUnit;
}

// $Barnyard: FUNCTION 00411200
AUnitManager::UnitPoolEntry* AUnitManager::AcquireUnitFromPool( const TCHAR* a_szTypeName )
{
	// Popped by CreateUnitFromTemplate once the unit is constructed
	m_apMemBlockStack[ m_iMemBlockStackIndex++ ] = g_pMemory->GetGlobalBlock();

	void* pSymbol = m_oUnitTypesTRB.GetSymbolAddress( a_szTypeName );

	if ( !pSymbol )
		return TNULL;

	UnitPool* pPool = GetUnitPool( pSymbol );

	if ( !pPool )
		return TNULL;

	UnitPoolEntry* pEntry = pPool->llFree.IsEmpty() ? StealPoolEntry( pPool ) : pPool->llFree.PopFront().Get();

	g_pMemory->SetGlobalBlock( pEntry->pMemBlock );
	pEntry->pUnit = TREINTERPRETCAST( AUnit*, 1 );

	return pEntry;
}

// $Barnyard: FUNCTION 004111b0
AUnitManager::UnitPoolEntry* AUnitManager::StealPoolEntry( UnitPool* a_pPool )
{
	if ( a_pPool->llUsed.IsEmpty() )
		return TNULL;

	UnitPoolEntry* pEntry = a_pPool->FindStealCandidate();

	if ( !pEntry )
		pEntry = a_pPool->llUsed.PopFront();

	AUnit* pUnit = pEntry->pUnit;

	if ( !RecycleUnit( pUnit ) )
		DestroyUnitImmediate( pUnit );

	return pEntry;
}

// $Barnyard: FUNCTION 00410830
TBOOL AUnitManager::ReleasePoolEntry( AUnit* a_pUnit )
{
	// NOTE: RecycleUnit calls this after deleting a_pUnit, so the type lookup below
	// reads freed memory (it still lives in the pool's memory block).
	void* pSymbol = m_oUnitTypesTRB.GetSymbolAddress( a_pUnit->GetType()->GetName() );

	if ( !pSymbol )
		return TFALSE;

	UnitPool* pPool = GetUnitPool( pSymbol );

	if ( !pPool )
		return TFALSE;

	SetPoolEntryUsed( pPool, a_pUnit, TFALSE );

	UnitPoolEntry* pEntry = TNULL;

	for ( TINT i = 0; i < pPool->iNumEntries; i++ )
	{
		if ( pPool->pEntries[ i ].pUnit == a_pUnit )
		{
			pEntry = &pPool->pEntries[ i ];
			break;
		}
	}

	pPool->llFree.PushFront( pEntry );
	pEntry->pUnit = TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 004107b0
void AUnitManager::SetPoolEntryUsed( UnitPool* a_pPool, AUnit* a_pUnit, TBOOL a_bUsed )
{
	UnitPoolEntry* pEntry = TNULL;

	for ( TINT i = 0; i < a_pPool->iNumEntries; i++ )
	{
		if ( a_pPool->pEntries[ i ].pUnit == a_pUnit )
		{
			pEntry = &a_pPool->pEntries[ i ];
			break;
		}
	}

	if ( !pEntry )
		return;

	if ( a_bUsed )
	{
		a_pPool->llUsed.PushBack( pEntry );
	}
	else
	{
		UnitPoolEntry* pUsed = a_pPool->FindUsedEntry( a_pUnit );

		if ( pUsed )
			a_pPool->llUsed.Erase( pUsed );
	}
}

// $Barnyard: FUNCTION 00410a30
void AUnitManager::SetUnitPoolUsed( AUnit* a_pUnit, TBOOL a_bUsed )
{
	void* pSymbol = m_oUnitTypesTRB.GetSymbolAddress( a_pUnit->m_pType->GetName() );

	if ( pSymbol )
	{
		UnitPool* pPool = GetUnitPool( pSymbol );

		if ( pPool )
			SetPoolEntryUsed( pPool, a_pUnit, a_bUsed );
	}
}

// $Barnyard: FUNCTION 0040fd80
AUnitManager::UnitPoolEntry* AUnitManager::UnitPool::FindUsedEntry( AUnit* a_pUnit )
{
	for ( auto it = llUsed.Begin(); it != llUsed.End(); it++ )
	{
		if ( it->pUnit == a_pUnit )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0040fdd0
AUnitManager::UnitPoolEntry* AUnitManager::UnitPool::FindStealCandidate()
{
	TINT iNumToCheck = llUsed.Size() / 3;

	for ( auto it = llUsed.Begin(); --iNumToCheck >= 0 && it != llUsed.End(); it++ )
	{
		AUnit* pUnit = it->pUnit;

		// Only recycle units nobody can see
		if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) && TSTATICCAST( AVisibleUnit, pUnit )->IsInCameraFrustum() )
		{
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			if ( TVector4::DistanceSq( *pUnit->GetSteer()->GetPosition(), pCamera->GetMatrix().GetTranslation() ) > 1.0f )
				return it;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00410920
void AUnitManager::OnUnitCreated( AUnit* a_pUnit )
{
	if ( ( a_pUnit->GetFlags() & AUnit::FLAGS_HASHED ) && a_pUnit->GetSteer() )
	{
		a_pUnit->IsUnk34Set();
		m_pUnitHash->AssignHash( a_pUnit );
	}

	a_pUnit->OnPostCreate();
	m_oUnitCreatedEmitter.Throw( &a_pUnit );
	RegisterCharacterUnit( a_pUnit );
}

// $Barnyard: FUNCTION 0040f3f0
void AUnitManager::ReinsertUnitInHash( AUnit* a_pUnit )
{
	a_pUnit->IsUnk34Set();
	m_pUnitHash->AssignHash( a_pUnit );
}

// $Barnyard: FUNCTION 00410f50
TBOOL AUnitManager::RecycleUnit( AUnit* a_pUnit )
{
	if ( m_iNumUnitTypes == 0 || !m_oUnitTypesTRB.GetSymbolAddress( a_pUnit->GetType()->GetName() ) )
		return TFALSE;

	a_pUnit->KillAttachedEffects();
	DestroyUnitImmediate( a_pUnit );
	m_oUnits.Remove( a_pUnit );
	delete a_pUnit;
	ReleasePoolEntry( a_pUnit );

	return TTRUE;
}

// $Barnyard: FUNCTION 00410f00
void AUnitManager::DestroyUnitImmediate( AUnit* a_pUnit )
{
	if ( m_oUnits.IsEmpty() || !a_pUnit || a_pUnit->IsDying() )
		return;

	TGlobalEmitter<AUnitDestroyEvent>::Throw( AUnitDestroyEvent{ a_pUnit } );
	a_pUnit->OnDestroy();
	FinalizeUnitRemoval( a_pUnit );
}

// $Barnyard: FUNCTION 00410e30
void AUnitManager::FinalizeUnitRemoval( AUnit* a_pUnit )
{
	UnregisterCharacterUnit( a_pUnit );
	m_pUnitHash->RemoveHash( a_pUnit );
	a_pUnit->m_oSelfProxy->SetUnit( TNULL );
	m_pUnitFactory->ReleaseInstance( a_pUnit );
}

// $Barnyard: FUNCTION 00410080
void AUnitManager::RegisterCharacterUnit( AUnit* a_pUnit )
{
	TRef<AUnitProxy> oProxy;
	m_pUIDMap->GetProxy( a_pUnit->GetId(), oProxy );

	if ( a_pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
	{
		CharacterUnitNode* pNode = new CharacterUnitNode();
		pNode->oUnit             = oProxy;

		m_aCharacterUnits[ 0 ].InsertTail( pNode );
	}
}

// $Barnyard: FUNCTION 00410970
void AUnitManager::UnregisterCharacterUnit( AUnit* a_pUnit )
{
	for ( TINT i = 0; i < TARRAYSIZE( m_aCharacterUnits ); i++ )
	{
		for ( auto it = m_aCharacterUnits[ i ].Begin(); it != m_aCharacterUnits[ i ].End(); it++ )
		{
			if ( it->oUnit->GetUnit() == a_pUnit )
			{
				m_aCharacterUnits[ i ].Delete( it );
				return;
			}
		}
	}
}

// $Barnyard: FUNCTION 00410010
void AUnitManager::DeleteDyingUnits()
{
	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); )
	{
		AUnit* pUnit = it;
		it++;

		if ( pUnit->IsDying() )
		{
			m_oUnits.Remove( pUnit );
			delete pUnit;
		}
	}
}

// $Barnyard: FUNCTION 00411120
void AUnitManager::ProcessPendingUnits()
{
	if ( !m_bProcessPending )
		return;

	m_bProcessPending = TFALSE;

	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); )
	{
		AUnit* pUnit = it;
		it++;

		if ( pUnit->GetFlags() & AUnit::FLAGS_PENDING_REMOVAL )
		{
			pUnit->SetPendingRemoval( TFALSE );

			if ( !RecycleUnit( pUnit ) )
				DestroyUnitImmediate( pUnit );
		}
	}
}

// $Barnyard: FUNCTION 00411340
void AUnitManager::RemoveAllUnits()
{
	m_oUnk1f8 = TNULL;

	for ( auto it = m_oUnits.Begin(); it != m_oUnits.End(); )
	{
		AUnit* pUnit = it;
		it++;

		// NOTE: retail inlines DestroyUnitImmediate here
		if ( !pUnit->IsDying() && !RecycleUnit( pUnit ) )
			DestroyUnitImmediate( pUnit );
	}

	m_oUnits.DeleteAll();

	TINT iUnused = 0;
	m_oUnitsRemovedEmitter.Throw( &iUnused );
}

// $Barnyard: FUNCTION 0040fcf0
void AUnitManager::UnloadUnitTypes()
{
	for ( TINT i = 0; i < m_iNumUnitTypes; i++ )
		m_pUnitTypeRecords[ i ].Destroy();

	if ( m_pUnitTypeRecords )
		delete[] m_pUnitTypeRecords;

	m_pUnitTypeRecords = TNULL;
	m_iNumUnitTypes    = 0;
	m_oUnitTypesTRB.Close();
}

// $Barnyard: FUNCTION 0040f780
void AUnitManager::UnitPool::Destroy()
{
	for ( TINT i = 0; i < iNumEntries; i++ )
		g_pMemory->DestroyMemBlock( pEntries[ i ].pMemBlock );

	llFree.Clear();
	iNumEntries = 0;

	if ( pEntries )
		delete[] pEntries;

	pEntries = TNULL;
}

// $Barnyard: FUNCTION 004102b0
void AUnitManager::SaveUnitNames( AHackedQuickSave::Buffer* a_pWriter, AUnit* a_pUnit, const TPString8& a_rcTypeName, const TPString8& a_rcUnitName )
{
	// Fall back to the name the unit is registered under
	TPString8 strUnitName = a_rcUnitName;

	if ( ( !strUnitName.GetPooledString() || strUnitName.GetString8().Length() == 0 ) && a_pUnit )
		strUnitName = AUnitNameManager::GetSingleton()->GetUnitName( a_pUnit->m_iId );

	a_pWriter->WriteString( strUnitName );
	a_pWriter->WriteString( a_rcTypeName );
}

// $Barnyard: FUNCTION 004103d0
void AUnitManager::SaveUnitRef( AHackedQuickSave::Buffer* a_pWriter, const AUnitRef* a_pRef )
{
	AUnit* pUnit = ( a_pRef->oProxy.Get() ) ? a_pRef->oProxy->GetUnit() : TNULL;

	TPString8 strTypeName;
	TPString8 strUnitName;

	if ( pUnit )
	{
		strTypeName = pUnit->m_pType->GetName();
		strUnitName = AUnitNameManager::GetSingleton()->GetUnitName( pUnit->m_iId );
	}

	if ( !pUnit || !strTypeName.GetPooledString() || strTypeName.GetString8().Length() == 0 )
	{
		if ( a_pRef->pType && strTypeName != a_pRef->pType->GetName() )
			strTypeName = a_pRef->pType->GetName();
	}

	if ( ( !strUnitName.GetPooledString() || strUnitName.GetString8().Length() == 0 ) && a_pRef->szName )
		strUnitName = TPString8( a_pRef->szName );

	SaveUnitNames( a_pWriter, pUnit, strTypeName, strUnitName );
}

// $Barnyard: FUNCTION 004105b0
void AUnitManager::LoadUnitRef( AHackedQuickSave::Buffer* a_pReader, AUnitRef* a_pRef )
{

	a_pRef->pType  = TNULL;
	a_pRef->oProxy = TNULL;

	TPString8 strUnitName;
	TPString8 strTypeName;

	a_pReader->ReadString( strUnitName );

	AUnit* pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( strUnitName );

	a_pReader->ReadString( strTypeName );

	if ( !pUnit )
	{
		if ( strTypeName.GetPooledString() )
		{
			if ( strTypeName.GetString8().Length() != 0 )
				pUnit = FindUnitByTypeName( strTypeName );

			// NOTE: retail checks the length a second time here
			if ( !pUnit && strTypeName.GetString8().Length() != 0 )
				a_pRef->pType = m_pUnitFactory->Find( strTypeName );
		}

		if ( !pUnit )
		{
			// NOTE: keeps a pointer into a pooled string that is released on return
			if ( strUnitName.GetPooledString() && strUnitName.GetString8().Length() != 0 )
				a_pRef->szName = strUnitName.GetString();

			return;
		}
	}

	m_pUIDMap->GetProxy( pUnit->m_iId, a_pRef->oProxy );
}
