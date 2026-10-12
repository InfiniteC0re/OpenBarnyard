#include "pch.h"
#include "ATreasureManager.h"
#include "ARegrowthManager.h"
#include "AGameTimeManager.h"
#include "FSM/AMethodPointer.h"
#include "Quests/AQuestManager.h"
#include "FX/AParticleEffectManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Terrain/ATerrainLocator.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "ARandom.h"

#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( flowers );
TPSTRING8_DECLARE( cans );
TPSTRING8_DECLARE( traps );
TPSTRING8_DECLARE( none );

// NOTE: GetClass 00530db0, CreateObject 005322c0, CreateObjectInPlace 00532320
TDEFINE_CLASS_INIT( ATreasureManager, 1, 0 );

// DAT_00783a20
AFunctionTable ATreasureManager::ms_oFunctionTable( &TGetClass( ATreasureManager ), TNULL );

// The interior networks the treasures of each type are placed in (PTR_g_str_flowers_00773e94)
static const TPString8* s_apTreasureNetworks[ ATreasureManager::TYPE_NUMOF ] = {
	&TPS8( flowers ),
	&TPS8( cans ),
	&TPS8( traps ),
	&TPS8( none ),
};

// The regrowth object type the patches of each treasure type count, 0xff uses the number in the name of the area (DAT_00773eb4)
static TINT s_aiTreasureRegrowthTypes[ ATreasureManager::TYPE_NUMOF ] = { -1, -1, -1, -1 };

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 00530e10
static TObject* __stdcall ATreasureManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return ATreasureManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 00532360
void ATreasureManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ATreasureManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer4Args<ATreasureManager, const TPString8&, TINT, const TPString8&, TINT>::Create( "AddTreasure", &ATreasureManager::AddTreasureFromScript ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ATreasureManager>::Create( "AddGopherTraps", &ATreasureManager::AddGopherTraps ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ATreasureManager>::Create( "RemoveGopherTraps", &ATreasureManager::RemoveGopherTraps ) );
}

// $Barnyard: FUNCTION 00530dc0
void ATreasureManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00531ba0
ATreasureManager::ATreasureManager()
{
	m_bEnabled                      = TFALSE;
	m_iSabotageStage                = SABOTAGE_NONE;
	m_iCurrentLeak                  = 1;
	m_pLeakEffect                   = TNULL;
	m_fRespawnTime                  = AGameTimeManager::DAY_LENGTH_UNITS + AGameTimeManager::DAY_LENGTH_UNITS;
	m_oPatches.iNumPatches          = 0;
	m_uiEnabledTypes                = 0;
	m_fQuestCheckTimer              = 10.0f;
	m_aTypeSettings[ 0 ].fNumPerDay = -1.0f; // not loaded

	for ( TINT i = 0; i < TYPE_NUMOF; i++ )
	{
		m_afLastSpawnTime[ i ] = -1200.0f;
		m_aiNumAdded[ i ]      = 0;
		m_uiEnabledTypes &= ~( 1 << i );
	}

	m_uiEnabledTypes &= ~FLAG_LEAKS_ACTIVE;
}

// $Barnyard: FUNCTION 00531cd0
// $Barnyard: FUNCTION 005323d0
ATreasureManager::~ATreasureManager()
{
	if ( m_bEnabled )
	{
		Reset();
		m_bEnabled = TFALSE;
	}

	m_llFreeTreasures.Clear();
}

// $Barnyard: FUNCTION 00531cc0
AFunctionTable* ATreasureManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00530e20
TBOOL ATreasureManager::Initialise()
{
	m_bEnabled = TFALSE;

	for ( TINT i = 0; i < MAX_NUM_TREASURES; i++ )
	{
		m_llFreeTreasures.PushFront( &m_aTreasures[ i ] );
		m_aTreasures[ i ].eType = TYPE_NONE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00531da0
void ATreasureManager::Enable()
{
	m_bEnabled       = TTRUE;
	m_iSabotageStage = SABOTAGE_NONE;
	LoadSettings();

	m_fRespawnTime = AGameTimeManager::DAY_LENGTH_UNITS + AGameTimeManager::DAY_LENGTH_UNITS;

	for ( TINT i = 0; i < TYPE_NUMOF; i++ )
	{
		m_afLastSpawnTime[ i ] = -1200.0f;
		m_aiNumAdded[ i ]      = 0;
		m_uiEnabledTypes &= ~( 1 << i );
	}

	m_uiEnabledTypes &= ~FLAG_LEAKS_ACTIVE;
}

// $Barnyard: FUNCTION 00531620
void ATreasureManager::Disable()
{
	if ( m_bEnabled )
	{
		Reset();
		m_bEnabled = TFALSE;
	}
}

// $Barnyard: FUNCTION 005310e0
void ATreasureManager::Reset()
{
	m_llTreasures.Clear();
	m_llFreeTreasures.Clear();

	for ( TINT i = 0; i < MAX_NUM_TREASURES; i++ )
	{
		m_llFreeTreasures.PushFront( &m_aTreasures[ i ] );
		m_aTreasures[ i ].eType = TYPE_NONE;
	}

	m_pLeakEffect = TNULL;
}

// $Barnyard: FUNCTION 00530de0
void ATreasureManager::PatchList::Clear()
{
	for ( TINT i = 0; i < iNumPatches; i++ )
	{
		aPatches[ i ].iMaxTreasures = 0;
		aPatches[ i ].iMinRegrowths = 0;
	}

	iNumPatches = 0;
}

// $Barnyard: FUNCTION 00530e60
TBOOL ATreasureManager::LoadSettings()
{
	// Already loaded?
	if ( m_aTypeSettings[ 0 ].fNumPerDay != -1.0f )
		return TTRUE;

	if ( m_oTRB.Load( "Data/Treasure/Treasure.trb" ) != TTRB::ERROR_OK )
		return TFALSE;

	TUtil::MemCopy( m_aTypeSettings, m_oTRB.GetSymbolAddress( "Other" ), sizeof( m_aTypeSettings ) );

	Patch* pPatches    = m_oTRB.CastSymbol<Patch>( "Patches" );
	TINT   iNumPatches = *m_oTRB.CastSymbol<TINT>( "NumbOfPatches" );

	m_oPatches.Clear();

	// The patches are stored in the ascending order
	for ( TINT i = iNumPatches - 1; i >= 0; i-- )
		m_oPatches.aPatches[ m_oPatches.iNumPatches++ ] = pPatches[ i ];

	// Terminator
	m_oPatches.aPatches[ m_oPatches.iNumPatches ].iMaxTreasures = 0;
	m_oPatches.aPatches[ m_oPatches.iNumPatches ].iMinRegrowths = -1;
	m_oPatches.iNumPatches++;

	m_oTRB.Close();

	return TTRUE;
}

// $Barnyard: FUNCTION 005323f0
void ATreasureManager::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return;

	m_fQuestCheckTimer -= a_fDeltaTime;

	if ( m_fQuestCheckTimer >= 0.0f )
		return;

	// Add or remove the leaks of the sabotage quests
	if ( m_iSabotageStage == SABOTAGE_NONE )
	{
		TPString8 strQuest( "Miles Raccoon Sabotage" );

		if ( AQuestManager::GetSingleton()->FindAssignedQuest( strQuest, 1 ) )
		{
			m_iCurrentLeak   = 1;
			m_iSabotageStage = SABOTAGE_FIRST;
			AddLeaks();
			m_uiEnabledTypes |= FLAG_LEAKS_ACTIVE;
		}
		else
		{
			strQuest = TPString8( "Miles Raccoon Sabotage 2" );

			if ( AQuestManager::GetSingleton()->FindAssignedQuest( strQuest, 1 ) )
			{
				m_iCurrentLeak   = 1;
				m_iSabotageStage = SABOTAGE_FIRST;
			}
		}
	}
	else if ( m_iSabotageStage <= SABOTAGE_FIRST )
	{
		TPString8 strQuest2( "Miles Raccoon Sabotage 2" );

		if ( AQuestManager::GetSingleton()->FindAssignedQuest( strQuest2, 1 ) )
		{
			m_iCurrentLeak   = 1;
			m_iSabotageStage = SABOTAGE_SECOND;
			AddLeaks();
			m_uiEnabledTypes |= FLAG_LEAKS_ACTIVE;
		}
		else if ( m_uiEnabledTypes & FLAG_LEAKS_ACTIVE )
		{
			TPString8 strQuest( "Miles Raccoon Sabotage" );

			// The quest is over, remove the leaks
			if ( !AQuestManager::GetSingleton()->FindAssignedQuest( strQuest, 1 ) && !AQuestManager::GetSingleton()->IsQuestComplete( strQuest, 1 ) )
			{
				m_iCurrentLeak   = 1;
				m_iSabotageStage = SABOTAGE_NONE;
				RemoveAllOfType( TYPE_LEAK );
				m_uiEnabledTypes &= ~FLAG_LEAKS_ACTIVE;
			}
		}
	}
	else if ( m_iSabotageStage == SABOTAGE_SECOND && ( m_uiEnabledTypes & FLAG_LEAKS_ACTIVE ) )
	{
		TPString8 strQuest2( "Miles Raccoon Sabotage 2" );

		// The quest is over, remove the leaks
		if ( !AQuestManager::GetSingleton()->FindAssignedQuest( strQuest2, 1 ) && !AQuestManager::GetSingleton()->IsQuestComplete( strQuest2, 1 ) )
		{
			m_iCurrentLeak   = 1;
			m_iSabotageStage = SABOTAGE_FIRST;
			RemoveAllOfType( TYPE_LEAK );
			m_uiEnabledTypes &= ~FLAG_LEAKS_ACTIVE;
		}
	}

	m_fQuestCheckTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 1.0f ) + 3.0f;

	// The current leak sprays water
	if ( ( m_uiEnabledTypes & FLAG_LEAKS_ACTIVE ) && m_pLeakEffect == TNULL )
	{
		T2_FOREACH( m_llTreasures, it )
		{
			if ( it->eType == TYPE_LEAK && m_iCurrentLeak == it->iLeakId )
			{
				m_iLeakEffect = AParticleEffectManager::FindEffect( "pipeleak" );
				m_pLeakEffect = AParticleEffectManager::GetSingleton()->PlayEffect( m_iLeakEffect, it->vecPosition, TVector4::VEC_NEGY );
			}
		}
	}
}

// $Barnyard: FUNCTION 00531190
TBOOL ATreasureManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "ATMan" );
	a_pBuffer->WriteInt( m_iSabotageStage );
	a_pBuffer->WriteInt( m_iCurrentLeak );
	a_pBuffer->WriteUInt8( m_uiEnabledTypes );

	for ( TINT i = 0; i < TYPE_NUMOF; i++ )
	{
		a_pBuffer->WriteInt( m_aiNumAdded[ i ] );
		a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_afLastSpawnTime[ i ] ) );
	}

	for ( TINT i = 0; i < MAX_NUM_TREASURES; i++ )
	{
		Treasure* pTreasure = &m_aTreasures[ i ];

		a_pBuffer->WriteVector3( pTreasure->vecPosition );
		a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &pTreasure->fDugTime ) );
		a_pBuffer->WriteUInt8( pTreasure->eType );

		if ( pTreasure->eType == TYPE_LEAK )
			a_pBuffer->WriteUInt8( pTreasure->iLeakId );
	}

	a_pBuffer->WriteString( "ATManEnd" );

	return TTRUE;
}

// $Barnyard: FUNCTION 00531260
TBOOL ATreasureManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "ATMan" );
	a_pBuffer->ReadInt( m_iSabotageStage );
	a_pBuffer->ReadInt( m_iCurrentLeak );

	TUINT8 uiEnabledTypes = 0;
	a_pBuffer->ReadUInt8( uiEnabledTypes );
	m_uiEnabledTypes = uiEnabledTypes;

	for ( TINT i = 0; i < TYPE_NUMOF; i++ )
	{
		a_pBuffer->ReadInt( m_aiNumAdded[ i ] );
		a_pBuffer->ReadFloat( m_afLastSpawnTime[ i ] );
	}

	m_llTreasures.Clear();
	m_llFreeTreasures.Clear();

	for ( TINT i = 0; i < MAX_NUM_TREASURES; i++ )
	{
		Treasure* pTreasure = &m_aTreasures[ i ];

		TVector4 vecPosition;
		a_pBuffer->ReadVector3( vecPosition );

		pTreasure->vecPosition.x = vecPosition.x;
		pTreasure->vecPosition.y = vecPosition.y;
		pTreasure->vecPosition.z = vecPosition.z;
		pTreasure->vecPosition.w = 1.0f;

		a_pBuffer->ReadFloat( pTreasure->fDugTime );
		a_pBuffer->ReadUInt8( *TREINTERPRETCAST( TUINT8*, &pTreasure->eType ) );

		if ( pTreasure->eType == TYPE_LEAK )
			a_pBuffer->ReadUInt8( *TREINTERPRETCAST( TUINT8*, &pTreasure->iLeakId ) );

		if ( pTreasure->eType == TYPE_NONE )
			m_llFreeTreasures.PushFront( pTreasure );
		else
			m_llTreasures.PushFront( pTreasure );
	}

	a_pBuffer->CheckString( "ATManEnd" );

	return TTRUE;
}

// $Barnyard: FUNCTION 00531140
TINT ATreasureManager::Treasure::GetNumBucks()
{
	ATreasureManager* pManager = ATreasureManager::GetSingleton();

	TINT iBucks = TINT( AGameTimeManager::GetSingleton()->GetTotalTime() - fDugTime - ( pManager->m_fRespawnTime + pManager->m_fRespawnTime ) );
	TMath::Clip( iBucks, 0, 75 );

	return iBucks + ARandom::GetSingleton()->m_oRandom.GetInt( 25 ) + 25;
}

// $Barnyard: FUNCTION 00531e10
void ATreasureManager::AddTreasure( const TVector4& a_rcPosition, TYPE a_eType )
{
	if ( m_aiNumAdded[ a_eType ] >= m_aTypeSettings[ a_eType ].iMaxAdded )
		return;

	if ( CountType( a_eType ) >= m_aTypeSettings[ a_eType ].iMaxInWorld )
		return;

	// NOTE: retail doesn't check if the free list is empty
	Treasure* pTreasure    = m_llFreeTreasures.PopFront();
	pTreasure->fDugTime    = TREASURE_NEVER_DUG;
	pTreasure->vecPosition = a_rcPosition;
	pTreasure->eType       = TINT8( a_eType );

	m_aiNumAdded[ a_eType ]++;
	m_llTreasures.PushFront( pTreasure );
}

// $Barnyard: FUNCTION 00531e90
void ATreasureManager::AddTreasuresToNetwork( const TPString8& a_rcNetwork, TFLOAT a_fHeightOffset, TINT a_iCount, TUINT8 a_uiRegrowthType, TYPE a_eType )
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( a_rcNetwork );

	if ( !pNetwork )
		return;

	TINT aiAreas[ 65 ];
	aiAreas[ 0 ] = 0;
	pNetwork->GetShuffledAreas( aiAreas );

	for ( TINT i = 0; i < aiAreas[ 0 ] && a_iCount > 0; i++ )
	{
		TINT iArea = aiAreas[ i + 1 ];

		if ( iArea < 0 )
			continue;

		AInteriorArea* pArea         = &pNetwork->m_pINetwork->m_pAreas[ iArea ];
		TINT           iNumTreasures = CountInArea( pArea, a_eType );
		TINT           iMaxTreasures = 1;

		if ( a_uiRegrowthType == 0xff )
		{
			// The name of the area is the max number of treasures
			TINT iNumber = T2String8::StringToInt( pArea->m_strName->GetString() );

			if ( iNumber != 0 )
				iMaxTreasures = iNumber;
		}
		else
		{
			// The more regrowth objects there are in the area, the more treasures it can have
			TINT iNumRegrowths = ARegrowthManager::GetSingleton()->CountInArea( pArea, a_uiRegrowthType );

			TINT iPatch = 0;
			while ( iNumRegrowths < m_oPatches.aPatches[ iPatch ].iMinRegrowths )
				iPatch++;

			iMaxTreasures = m_oPatches.aPatches[ iPatch ].iMaxTreasures;
		}

		if ( iNumTreasures < iMaxTreasures )
		{
			a_iCount--;

			TVector4 vecPosition;
			pArea->GetRandomPosition( vecPosition, 0.0f );
			vecPosition.y += a_fHeightOffset;

			AddTreasure( vecPosition, a_eType );
		}
	}
}

// $Barnyard: FUNCTION 005320b0
void ATreasureManager::AddTreasureFromScript( const TPString8& a_rcNetwork, TINT a_iCount, const TPString8& a_rcRegrowthType, TINT a_eType )
{
	TUINT8 uiRegrowthType = TUINT8( ARegrowthManager::GetSingleton()->GetTypeIndex( a_rcRegrowthType ) );
	AddTreasuresToNetwork( a_rcNetwork, 0.0f, a_iCount, uiRegrowthType, a_eType );
}

// $Barnyard: FUNCTION 005320e0
void ATreasureManager::UpdateRespawns()
{
	if ( !m_bEnabled )
		return;

	s_aiTreasureRegrowthTypes[ TYPE_FLOWERS ] = ARegrowthManager::GetTypes()->FindEnum( "treasurebells" );

	// Seconds since the first day
	TFLOAT fTime = AGameTimeManager::GAME_TIME_TO_REAL_TIME * AGameTimeManager::GetSingleton()->GetTotalTime() + TFLOAT( AGameTimeManager::SECONDS_PER_DAY ) + AGameTimeManager::DAY_START_SECONDS;

	for ( TINT i = 0; i < TYPE_NUMOF; i++ )
	{
		if ( *s_apTreasureNetworks[ i ] == TPS8( none ) )
			continue;

		TypeSettings& rSettings = m_aTypeSettings[ i ];

		if ( rSettings.fNumPerDay == 0.0f )
		{
			// Added once
			if ( m_aiNumAdded[ i ] <= 0 )
			{
				for ( TFLOAT fNumLeft = TFLOAT( rSettings.iMaxAdded ); fNumLeft >= 1.0f; )
				{
					fNumLeft -= 1.0f;
					AddTreasuresToNetwork( *s_apTreasureNetworks[ i ], 0.0f, 1, TUINT8( s_aiTreasureRegrowthTypes[ i ] ), i );
				}
			}

			m_afLastSpawnTime[ i ] = -1.0f;
		}
		else
		{
			TFLOAT fNumToAdd = ( fTime - m_afLastSpawnTime[ i ] ) / TFLOAT( AGameTimeManager::SECONDS_PER_DAY ) * rSettings.fNumPerDay;

			while ( fNumToAdd >= 1.0f )
			{
				fNumToAdd -= 1.0f;
				AddTreasuresToNetwork( *s_apTreasureNetworks[ i ], 0.0f, 1, TUINT8( s_aiTreasureRegrowthTypes[ i ] ), i );
			}

			// Keep the leftover
			m_afLastSpawnTime[ i ] = fTime - TFLOAT( AGameTimeManager::SECONDS_PER_DAY ) / rSettings.fNumPerDay * fNumToAdd;
		}
	}
}

// $Barnyard: FUNCTION 00530f30
void ATreasureManager::AddTreasuresFromLocators( ATerrainLocatorList* a_pLocatorList, const TMatrix44& a_rcMatrix, TYPE a_eType, const TCHAR* a_szPrefix )
{
	if ( !m_bEnabled )
		return;

	TINT iNumLocators = a_pLocatorList->GetNumLocators();

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		const TCHAR* szName = a_pLocatorList->GetLocatorName( i );

		if ( T2String8::CompareNoCase( a_szPrefix, szName, T2String8::Length( a_szPrefix ) ) != 0 )
			continue;

		TINT8        iLeakId = 0;
		const TCHAR* pLeak   = T2String8::FindString( szName, "leak" );

		if ( pLeak )
		{
			const TCHAR* pNumber = pLeak + 4;
			iLeakId              = TINT8( T2String8::StringToInt( pNumber ) );

			if ( T2String8::Compare( "leak03B02", szName ) == 0 )
				iLeakId = 5;

			// "leakNNB..." are the leaks of the first quest, the others are of the second one
			if ( pNumber[ 2 ] == 'B' ? m_iSabotageStage != SABOTAGE_FIRST : m_iSabotageStage != SABOTAGE_SECOND )
				continue;
		}

		if ( m_llFreeTreasures.IsEmpty() )
			return;

		Treasure* pTreasure = m_llFreeTreasures.PopFront();
		pTreasure->iLeakId  = iLeakId;

		TMatrix44 matLocator;
		a_pLocatorList->GetLocator( i )->GetMatrix( matLocator );

		TMatrix44 matTransform;
		matTransform.Multiply( a_rcMatrix, matLocator );

		pTreasure->vecPosition = matTransform.GetTranslation();
		pTreasure->fDugTime    = TREASURE_NEVER_DUG;
		pTreasure->eType       = TINT8( a_eType );

		m_aiNumAdded[ a_eType ]++;
		m_llTreasures.PushFront( pTreasure );
	}
}

// $Barnyard: FUNCTION 00531780
void ATreasureManager::AddTreasuresFromLocators( ATerrainLocatorList* a_pLocatorList, TYPE a_eType, const TCHAR* a_szPrefix )
{
	if ( !m_bEnabled )
		return;

	// The locators are Z up
	TMatrix44 matTransform;
	matTransform.m_f11 = 1.0f;
	matTransform.m_f12 = 0.0f;
	matTransform.m_f13 = 0.0f;
	matTransform.m_f14 = 0.0f;
	matTransform.m_f21 = 0.0f;
	matTransform.m_f22 = 0.0f;
	matTransform.m_f23 = 1.0f;
	matTransform.m_f24 = 0.0f;
	matTransform.m_f31 = 0.0f;
	matTransform.m_f32 = -1.0f;
	matTransform.m_f33 = 0.0f;
	matTransform.m_f34 = 0.0f;
	matTransform.m_f41 = 0.0f;
	matTransform.m_f42 = 0.0f;
	matTransform.m_f43 = 0.0f;
	matTransform.m_f44 = 1.0f;

	AddTreasuresFromLocators( a_pLocatorList, matTransform, a_eType, a_szPrefix );
}

// $Barnyard: FUNCTION 00532290
void ATreasureManager::AddLeaks()
{
	AddTreasuresFromLocators( ATerrainInterface::GetSingleton()->GetLocatorList(), TYPE_LEAK, "leak" );
}

// $Barnyard: FUNCTION 00532000
void ATreasureManager::AddGopherTraps()
{
	RemoveGopherTraps();

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( traps ) );

	// A trap in the middle of every area
	for ( TINT i = 0; i < pNetwork->m_pINetwork->m_iNumAreas; i++ )
	{
		TVector4 vecPosition;
		pNetwork->m_pINetwork->m_pAreas[ i ].GetPosition( vecPosition );

		Treasure* pTreasure    = m_llFreeTreasures.PopFront();
		pTreasure->fDugTime    = TREASURE_NEVER_DUG;
		pTreasure->vecPosition = vecPosition;
		pTreasure->eType       = TYPE_TRAPS;

		m_llTreasures.PushFront( pTreasure );
	}
}

// $Barnyard: FUNCTION 005316b0
void ATreasureManager::RemoveGopherTraps()
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( TPS8( traps ) );

	for ( auto it = m_llTreasures.Begin(); it != m_llTreasures.End(); )
	{
		Treasure* pTreasure = it;
		it                  = it->Next();

		if ( pTreasure->eType != TYPE_TRAPS )
			continue;

		TBOOL bRemoved = TFALSE;

		for ( TINT i = 0; i < pNetwork->m_pINetwork->m_iNumAreas; i++ )
		{
			if ( bRemoved )
				break;

			if ( pNetwork->m_pINetwork->m_pAreas[ i ].CheckPositionInsideArea( pTreasure->vecPosition ) )
			{
				// NOTE: the type isn't reset to TYPE_NONE
				m_llTreasures.Transfer( pTreasure, m_llFreeTreasures );
				bRemoved = TTRUE;
			}
		}
	}
}

// $Barnyard: FUNCTION 00531830
TBOOL ATreasureManager::RemoveTreasure( Treasure* a_pTreasure )
{
	m_llTreasures.Transfer( a_pTreasure, m_llFreeTreasures );

	if ( a_pTreasure->eType == TYPE_LEAK )
	{
		// The leak is fixed, the next one starts to leak
		a_pTreasure->eType = TYPE_NONE;
		m_iCurrentLeak++;

		if ( m_pLeakEffect )
		{
			m_pLeakEffect->GetInstance()->Kill();
			m_pLeakEffect = TNULL;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00531b30
void ATreasureManager::RemoveAllOfType( TYPE a_eType )
{
	for ( auto it = m_llTreasures.Begin(); it != m_llTreasures.End(); )
	{
		Treasure* pTreasure = it;
		it                  = it->Next();

		if ( a_eType == pTreasure->eType )
			RemoveTreasure( pTreasure );
	}

	if ( a_eType == TYPE_LEAK )
		m_iCurrentLeak = 0;
}

// $Barnyard: FUNCTION 00531ae0
TINT ATreasureManager::CountType( TYPE a_eType )
{
	TINT iCount = 0;

	T2_FOREACH( m_llTreasures, it )
	{
		if ( it->eType == a_eType )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 00531640
TINT ATreasureManager::CountInArea( AInteriorArea* a_pArea, TYPE a_eType )
{
	TINT iCount = 0;

	T2_FOREACH( m_llTreasures, it )
	{
		if ( it->eType == a_eType && a_pArea->CheckPositionInsideArea( it->vecPosition ) )
			iCount++;
	}

	return iCount;
}

// $Barnyard: FUNCTION 00531ab0
void ATreasureManager::SetTypeEnabled( TYPE a_eType, TBOOL a_bEnabled )
{
	if ( a_bEnabled )
		m_uiEnabledTypes |= ( 1 << a_eType );
	else
		m_uiEnabledTypes &= ~( 1 << a_eType );
}

// $Barnyard: FUNCTION 005319e0
ATreasureManager::Treasure* ATreasureManager::FindClosestTreasure( const TVector4& a_rcPosition, TFLOAT a_fRadius )
{
	TFLOAT    fClosestDistanceSq = a_fRadius * a_fRadius;
	TFLOAT    fTime              = AGameTimeManager::GetSingleton()->GetTotalTime();
	Treasure* pClosest           = TNULL;

	T2_FOREACH( m_llTreasures, it )
	{
		// Not respawned yet?
		if ( fTime - it->fDugTime < m_fRespawnTime )
			continue;

		TINT eType = it->eType;

		if ( ( ( 1 << eType ) & m_uiEnabledTypes ) && ( eType != TYPE_LEAK || m_iCurrentLeak == it->iLeakId ) )
		{
			TFLOAT fDZ         = a_rcPosition.z - it->vecPosition.z;
			TFLOAT fDX         = a_rcPosition.x - it->vecPosition.x;
			TFLOAT fDistanceSq = fDX * fDX + fDZ * fDZ;

			if ( fDistanceSq <= fClosestDistanceSq )
			{
				fClosestDistanceSq = fDistanceSq;
				pClosest           = it;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 00531890
ATreasureManager::Treasure* ATreasureManager::FindTreasureInFront( const TVector4& a_rcPosition, const TVector2& a_rcDirection, TFLOAT a_fMaxDistance, TYPE a_eType )
{
	Treasure* pFound = TNULL;
	TFLOAT    fTime  = AGameTimeManager::GetSingleton()->GetTotalTime();

	T2_FOREACH( m_llTreasures, it )
	{
		// Not respawned yet?
		if ( fTime - it->fDugTime < m_fRespawnTime )
			continue;

		TINT eType = it->eType;

		if ( !( ( 1 << eType ) & m_uiEnabledTypes ) )
			continue;

		if ( a_eType != TYPE_NONE && eType != a_eType )
			continue;

		if ( eType == TYPE_LEAK && m_iCurrentLeak != it->iLeakId )
			continue;

		TVector2 vecToTreasure( it->vecPosition.x - a_rcPosition.x, it->vecPosition.z - a_rcPosition.z );

		if ( vecToTreasure.x * a_rcDirection.x + vecToTreasure.y * a_rcDirection.y > 0.0f )
		{
			TFLOAT fDistance = vecToTreasure.NormalizeMag();

			if ( fDistance <= a_fMaxDistance && vecToTreasure.x * a_rcDirection.x + vecToTreasure.y * a_rcDirection.y > 0.8f )
			{
				a_fMaxDistance = fDistance;
				pFound         = it;
			}
		}
	}

	return pFound;
}
