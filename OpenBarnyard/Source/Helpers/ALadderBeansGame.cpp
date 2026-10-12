#include "pch.h"
#include "ALadderBeansGame.h"
#include "APlayerProgress.h"
#include "FSM/AMethodPointer.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASimState.h"
#include "Locale/ALocaleManager.h"
#include "Tasks/ARootTask.h"
#include "Player/APlayerManager.h"
#include "Cameras/ACameraManager.h"
#include "Animation/AModel.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/AStaticUnit.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AInventoryComponent.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "Physics/ACollisionObjectMesh.h"

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( milkweed );
TPSTRING8_DECLARE( milkweed_seed );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( Equip );

// NOTE: CreateObject 00537a40, CreateObjectInPlace 00537aa0
TDEFINE_CLASS_INIT( ALadderBeansGame );

// 0x00783b88
AFunctionTable ALadderBeansGame::ms_oFunctionTable( &TGetClass( ALadderBeansGame ), TNULL );

// FSM fallback: the ladder beans game of the sim
// $Barnyard: FUNCTION 00537440
static TObject* __stdcall ALadderBeansGameFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

	if ( pSimState )
		return pSimState->GetLadderBeansGame();

	return TNULL;
}

// $Barnyard: FUNCTION 00537f70
void ALadderBeansGame::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ALadderBeansGameFunctionFallback ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<ALadderBeansGame>::Create( "EnableMilkWeedForCurrentArea", &ALadderBeansGame::EnableMilkweedForCurrentArea ) );
}

// $Barnyard: FUNCTION 00537370
void ALadderBeansGame::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00537470
ALadderBeansGame::ALadderBeansGame()
    : m_bChanged( TFALSE )
    , m_pLadders( TNULL )
{
	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		m_apMilkweeds[ i ]        = TNULL;
		m_apCollisionObjects[ i ] = TNULL;
	}
}

// $Barnyard: FUNCTION 00537b40
// $Barnyard: FUNCTION 00537ae0
ALadderBeansGame::~ALadderBeansGame()
{
	Destroy();
	m_oPopupListener.Disconnect();
}

// $Barnyard: FUNCTION 005374f0
AFunctionTable* ALadderBeansGame::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// Area of the patch of ladder a_uiIndex in the default network
static AInteriorArea* GetPatchArea( TUINT a_uiIndex, TCHAR ( &a_rBuffer )[ 8 ] )
{
	T2String8::Format( a_rBuffer, "mweed%02d", ALadderBeansGame::GetLadderId( a_uiIndex ) );

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	TINT              iArea    = pNetwork->FindAreaIndex( TPString8( a_rBuffer ) );
	TASSERT( iArea >= 0 );

	// NOTE: retail doesn't check the index
	return pNetwork->GetArea( iArea );
}

// Makes the milkweed of a patch something the milk can hit (planted and still growing) or not
static void SetMilkweedSquirtable( AUnit* a_pMilkweed, TBOOL a_bSquirtable )
{
	ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, a_pMilkweed )->GetCollisionObject();

	if ( pCollisionObject->IsCollisionObjectLinked() )
		pCollisionObject->UnlinkCollisionObject();

	if ( a_bSquirtable )
	{
		pCollisionObject->Activate();
		a_pMilkweed->AddFlags( AUnit::FLAGS_MILK_TARGET );
	}
	else
	{
		a_pMilkweed->RemoveFlags( AUnit::FLAGS_MILK_TARGET );
	}
}

// $Barnyard: FUNCTION 00537fa0
TBOOL ALadderBeansGame::Create()
{
	m_pLadders = ASimPersistData::GetSingleton()->GetLadderBeans();
	m_bChanged = TFALSE;

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		ASimPersistData::LadderBeans& rLadder = m_pLadders->aLadders[ i ];

		// NOTE: retail stops the growth but keeps fHeight: a milkweed left (mini game, barn...) while it was growing
		// stays short of its stage until it's squirted again (a save reload recomputes the height)
		rLadder.uiFlags &= ~ASimPersistData::LadderBeans::LADDERFLAGS_GROWING;

		TCHAR szArea[ 8 ];
		T2String8::Format( szArea, "mweed%02d", GetLadderId( i ) );

		const TBOOL bGrown = rLadder.iStage == 8;
		SetPatchGrown( szArea, bGrown );

		AInteriorArea* pArea = GetPatchArea( i, szArea );

		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ, 0, -1, -1, TTRUE );
		m_apMilkweeds[ i ] = AUnitManager::GetSingleton()->SpawnUnit( TPS8( milkweed ), &oTransform );

		AVisibleUnit* pMilkweed = TSTATICCAST( AVisibleUnit, m_apMilkweeds[ i ] );
		pMilkweed->GetModelInstance()->SetUnlit();
		pMilkweed->ChangeSteer( &TGetClass( AStationarySteer ) );

		AModelInstance* pModelInstance = pMilkweed->GetModelInstance();
		pModelInstance->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_POSZ );

		if ( pModelInstance->GetSceneObject()->GetInstance()->GetModel()->GetNumCollisionMeshes() > 0 )
		{
			m_apCollisionObjects[ i ] = new ACollisionObjectMesh();
			m_apCollisionObjects[ i ]->Create( pModelInstance->GetSceneObject() );
		}

		UpdateMilkweed( pArea );

		// Only a planted milkweed that isn't fully grown can be squirted
		SetMilkweedSquirtable( m_apMilkweeds[ i ], rLadder.iStage != 0 && !bGrown );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00537500
void ALadderBeansGame::Destroy()
{
	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		if ( m_apMilkweeds[ i ] )
		{
			AUnitManager::GetSingleton()->DestroyUnit( m_apMilkweeds[ i ] );
			m_apMilkweeds[ i ] = TNULL;
		}

		if ( m_apCollisionObjects[ i ] )
		{
			delete m_apCollisionObjects[ i ];
			m_apCollisionObjects[ i ] = TNULL;
		}
	}

	m_pLadders = TNULL;
}

// $Barnyard: FUNCTION 005382a0
TBOOL ALadderBeansGame::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bChanged )
		return TFALSE;

	TBOOL bDone = TTRUE;

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		ASimPersistData::LadderBeans& rLadder = m_pLadders->aLadders[ i ];

		if ( !( rLadder.uiFlags & ASimPersistData::LadderBeans::LADDERFLAGS_GROWING ) )
			continue;

		TCHAR szArea[ 8 ];
		T2String8::Format( szArea, "mweed%02d", GetLadderId( i ) );

		// Grow one unit per second (the Y axis points down)
		rLadder.fHeight -= a_fDeltaTime;

		if ( rLadder.fHeight >= rLadder.fTargetHeight )
		{
			bDone = TFALSE;
		}
		else
		{
			rLadder.fHeight = rLadder.fTargetHeight;
			rLadder.uiFlags &= ~ASimPersistData::LadderBeans::LADDERFLAGS_GROWING;

			if ( rLadder.iStage == 8 )
			{
				// The ladder can be climbed now and the milk doesn't hit it anymore
				SetPatchGrown( szArea, TTRUE );
				SetMilkweedSquirtable( m_apMilkweeds[ i ], TFALSE );
			}
		}

		UpdateMilkweed( GetPatchArea( i, szArea ) );
	}

	if ( bDone )
		m_bChanged = TFALSE;

	return bDone;
}

// $Barnyard: FUNCTION 00537550
void ALadderBeansGame::Render()
{
	for ( TINT i = ASimPersistData::NUM_LADDER_BEANS - 1; i >= 0; i-- )
	{
		AVisibleUnit* pMilkweed = TSTATICCAST( AVisibleUnit, m_apMilkweeds[ i ] );

		if ( !pMilkweed )
			continue;

		const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
		const TVector4* pPosition   = pMilkweed->GetSteer()->GetPosition();

		TFLOAT fDistanceSq = ( rcCameraPos.z - pPosition->z ) * ( rcCameraPos.z - pPosition->z ) + ( rcCameraPos.x - pPosition->x ) * ( rcCameraPos.x - pPosition->x );

		// Fade out between 50 and 75 units
		if ( fDistanceSq > 5625.0f )
			pMilkweed->SetOpacity( 0.0f );
		else if ( fDistanceSq < 2500.0f )
			pMilkweed->SetOpacity( 1.0f );
		else
			pMilkweed->SetOpacity( 1.0f - ( fDistanceSq - 2500.0f ) * 0.00032f );
	}
}

// $Barnyard: FUNCTION 00537380
TBOOL ALadderBeansGame::IsPlanted( TUINT a_uiIndex ) const
{
	return a_uiIndex < ASimPersistData::NUM_LADDER_BEANS && m_pLadders->aLadders[ a_uiIndex ].iStage > 0;
}

// $Barnyard: FUNCTION 005373e0
TINT ALadderBeansGame::FindMilkweed( AUnit* a_pUnit ) const
{
	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		if ( m_apMilkweeds[ i ] == a_pUnit )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00537d40
void ALadderBeansGame::PlantMilkweed( TUINT a_uiIndex )
{
	if ( a_uiIndex >= ASimPersistData::NUM_LADDER_BEANS )
		return;

	ASimPersistData::LadderBeans& rLadder = m_pLadders->aLadders[ a_uiIndex ];

	if ( rLadder.iStage != 0 )
		return;

	rLadder.iStage = 1;
	rLadder.uiFlags |= ASimPersistData::LadderBeans::LADDERFLAGS_GROWING;
	rLadder.fTargetHeight = ASimPersistData::ms_aLadderBeansInfo[ a_uiIndex ].afStageHeight[ 1 ] + rLadder.fTargetHeight;

	SetMilkweedSquirtable( m_apMilkweeds[ a_uiIndex ], TTRUE );
	m_bChanged = TTRUE;

	// Take a seed from the player (from the hand, the equipped items or any category)
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer )
		return;

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	if ( !pInventory )
		return;

	AUnit*               pSeed = TNULL;
	AInventoryComponent* pHand = pInventory->GetCategory( TPS8( Hand ) );

	if ( pHand )
		pSeed = pHand->DropItemOfType( TPS8( milkweed_seed ) );

	if ( !pSeed )
	{
		AInventoryComponent* pEquip = pInventory->GetCategory( TPS8( Equip ) );

		if ( pEquip )
			pSeed = pEquip->DropItemOfType( TPS8( milkweed_seed ) );
	}

	if ( !pSeed )
		pSeed = pInventory->DropItemOfType( TPS8( milkweed_seed ) );

	if ( pSeed )
		AUnitManager::GetSingleton()->DestroyUnit( pSeed );
}

// $Barnyard: FUNCTION 00537e50
void ALadderBeansGame::EnableMilkweedForCurrentArea()
{
	ASteer* pSteer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer();

	if ( !pSteer->IsA( &TGetClass( APlayerSteer ) ) )
		return;

	AInteriorArea* pArea = TSTATICCAST( APlayerSteer, pSteer )->GetCurrentArea();

	if ( pArea && HASANYFLAG( pArea->m_eFlags, AInteriorArea::FLAGS_SPECIAL_GROUP_AREA ) && HASANYFLAG( pArea->m_eFlags, AInteriorArea::FLAGS_RUNTIME_SPECIAL ) )
		PlantMilkweed( FindLadder( pArea->m_strName->GetString() ) );
}

// $Barnyard: FUNCTION 00537b60
void ALadderBeansGame::OnMilkweedSquirted( TUINT a_uiIndex )
{
	if ( a_uiIndex >= ASimPersistData::NUM_LADDER_BEANS )
		return;

	ASimPersistData::LadderBeans& rLadder = m_pLadders->aLadders[ a_uiIndex ];

	// Only planted ladders that aren't fully grown nor growing
	if ( rLadder.iStage <= 0 || rLadder.iStage >= 8 || ( rLadder.uiFlags & ASimPersistData::LadderBeans::LADDERFLAGS_GROWING ) )
		return;

	rLadder.iStage += 1;
	rLadder.uiFlags |= ASimPersistData::LadderBeans::LADDERFLAGS_GROWING;
	rLadder.fTargetHeight = ASimPersistData::ms_aLadderBeansInfo[ a_uiIndex ].afStageHeight[ rLadder.iStage ] + rLadder.fTargetHeight;
	m_bChanged            = TTRUE;

	if ( rLadder.iStage != 8 )
		return;

	// Count the fully grown ladders
	TINT8 iNumGrown = 0;

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		if ( m_pLadders->aLadders[ i ].iStage == 8 )
			iNumGrown++;
	}

	APlayerProgress* pProgress = APlayerProgress::GetSingleton();
	TUINT            uiFlags   = pProgress->GetUnlockedFlags();

	if ( ( uiFlags & ( APlayerProgress::UNLOCKFLAGS_ART0 << iNumGrown ) ) && ( uiFlags & ( APlayerProgress::UNLOCKFLAGS_MOVIE0 << iNumGrown ) ) )
		return;

	// A new reward is unlocked
	AMessagePopupState* pPopup = new AMessagePopupState(
	    ALocaleManager::Interface()->GetString( 326 ),
	    ALocaleManager::Interface()->GetString( 204 ),
	    TPString8(),
	    1,
	    TFALSE,
	    AInputCommand_OK,
	    AInputCommand_Empty,
	    AInputCommand_Empty,
	    AInputCommand_Empty,
	    TFALSE,
	    60.0f,
	    0
	);

	m_oPopupListener.Connect( pPopup->GetResponseEmitter(), this, OnRewardPopupResponse, 0 );
	pProgress->UnlockLadderBeansFlags( iNumGrown );
	ARootTask::GetSingleton()->SetPaused( TTRUE );
	AGameStateController::GetSingleton()->PushState( pPopup );
}

// $Barnyard: FUNCTION 00537920
TBOOL __stdcall ALadderBeansGame::OnRewardPopupResponse( ALadderBeansGame* a_pGame, AMessagePopupState* a_pPopup, AMessagePopupState::ResponseEvent* a_pEvent )
{
	a_pPopup->Remove();
	ARootTask::GetSingleton()->SetPaused( TFALSE );

	return TTRUE;
}

// $Barnyard: FUNCTION 005373b0
TINT ALadderBeansGame::FindLadder( const TCHAR* a_szAreaName )
{
	// The digit after "mweed0"
	const TINT iId = a_szAreaName[ 6 ] - '0';

	for ( TINT i = 0; i < ASimPersistData::NUM_LADDER_BEANS; i++ )
	{
		if ( ASimPersistData::ms_aLadderBeansInfo[ i ].iId == iId )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00537410
TINT ALadderBeansGame::GetLadderId( TUINT a_uiIndex )
{
	if ( a_uiIndex >= ASimPersistData::NUM_LADDER_BEANS )
		return -1;

	return ASimPersistData::ms_aLadderBeansInfo[ a_uiIndex ].iId;
}

// Direction (XZ, normalised) from the patch to its special point 1, where the milkweed leans to
static void GetPatchDirection( AInteriorArea* a_pArea, const TVector4& a_rcPosition, TVector4& a_rOutDirection )
{
	const TVector4& rcPoint = a_pArea->GetSpecialPoint( 1 );

	a_rOutDirection.x = rcPoint.x - a_rcPosition.x;
	a_rOutDirection.y = 0.0f;
	a_rOutDirection.z = rcPoint.z - a_rcPosition.z;
	a_rOutDirection.w = rcPoint.w;
	a_rOutDirection.Normalise();
}

// $Barnyard: FUNCTION 00537610
TBOOL ALadderBeansGame::GetMilkweedPosition( AInteriorArea* a_pArea, TVector4& a_rOutPosition )
{
	const TINT iLadder = FindLadder( a_pArea->m_strName->GetString() );

	if ( iLadder < 0 )
		return TFALSE;

	a_pArea->GetPosition( a_rOutPosition );
	a_rOutPosition.y = a_pArea->GetSpecialHeight();

	TVector4 vecDirection;
	GetPatchDirection( a_pArea, a_rOutPosition, vecDirection );

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, ASimPersistData::ms_aLadderBeansInfo[ iLadder ].fEndOffset + 0.5f );

	a_rOutPosition.x += vecOffset.x;
	a_rOutPosition.y += vecOffset.y;
	a_rOutPosition.z += vecOffset.z;

	return TTRUE;
}

// $Barnyard: FUNCTION 00537750
void ALadderBeansGame::UpdateMilkweed( AInteriorArea* a_pArea )
{
	const TINT iLadder = FindLadder( a_pArea->m_strName->GetString() );

	if ( iLadder < 0 )
		return;

	const ASimPersistData::LadderBeans&     rcLadder  = m_pLadders->aLadders[ iLadder ];
	const ASimPersistData::LadderBeansInfo& rcInfo    = ASimPersistData::ms_aLadderBeansInfo[ iLadder ];
	AStaticUnit*                            pMilkweed = TSTATICCAST( AStaticUnit, m_apMilkweeds[ iLadder ] );

	pMilkweed->SetModelYOffset( rcLadder.fHeight );

	TVector4 vecPosition;
	a_pArea->GetPosition( vecPosition );
	vecPosition.y = a_pArea->GetSpecialHeight();

	TVector4 vecDirection;
	GetPatchDirection( a_pArea, vecPosition, vecDirection );

	// The milkweed leans further from the patch as it grows
	const TFLOAT fGrowth = ( rcLadder.fHeight - rcInfo.afStageHeight[ 0 ] ) / ( rcLadder.fMaxHeight - rcInfo.afStageHeight[ 0 ] );

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, fGrowth * ( rcInfo.fEndOffset - rcInfo.fStartOffset ) + rcInfo.fStartOffset );

	vecPosition.x += vecOffset.x;
	vecPosition.y += vecOffset.y;
	vecPosition.z += vecOffset.z;

	pMilkweed->GetSteer()->SetPosition( vecPosition );
	pMilkweed->GetSteer()->SetDirection( vecDirection );

	// The model is moved by the height of the stage
	vecPosition.y += rcLadder.fHeight;
	pMilkweed->GetModelInstance()->SetTransform( vecPosition, vecDirection );

	// NOTE: retail doesn't check if the model has a collision mesh
	m_apCollisionObjects[ iLadder ]->UpdateWorldSphere();
}

// $Barnyard: FUNCTION 00537ec0
void ALadderBeansGame::SetPatchGrown( const TCHAR* a_szAreaName, TBOOL a_bGrown )
{
	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
	AInteriorArea*    pArea    = pNetwork->GetArea( pNetwork->FindAreaIndex( TPString8( a_szAreaName ) ) );

	if ( a_bGrown )
		pArea->m_eFlags &= ~AInteriorArea::FLAGS_RUNTIME_SPECIAL;
	else
		pArea->m_eFlags |= AInteriorArea::FLAGS_RUNTIME_SPECIAL;
}
