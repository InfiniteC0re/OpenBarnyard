#include "pch.h"
#include <Render/TCameraObject.h>
#include "AShakeTreeMicroGame.h"
#include "ARandom.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectType.h"
#include "FSM/AFunctionPointer.h"
#include "FX/AAniEffectManager.h"
#include "FX/ADieAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "FX/AWaitAniEffect.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/ATreeManager.h"
#include "Input/AInputHandler.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnitFactory.h"
#include "Units/AUnitManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Helpers/ARegrowthManager.h"
#include "SoundBank/barnyard.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0052c960, CreateObject 0052daa0, CreateObjectInPlace 0052db00
// $Barnyard: FUNCTION 0052daa0
// $Barnyard: FUNCTION 0052db00
TDEFINE_CLASS_INIT( AShakeTreeMicroGame, 1, 0 );

TPSTRING8_DECLARE( NormalBarnyard );
TPSTRING8_DECLARE( kick );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( egg );

// DAT_0078395c
AFunctionTable AShakeTreeMicroGame::ms_oFunctionTable( &TGetClass( AShakeTreeMicroGame ), TNULL );

// ACoinSpawner.cpp
void KnockOverCrate( ARegrowthManager::GrowthObject* a_pObject, TINT a_iType );

// Where the egg of the henhouse is thrown out
static const TVector4 s_vecHenhousePosition( -13.5f, -10.0f, 30.0f, 1.0f );

// $Barnyard: FUNCTION 0052db40
void AShakeTreeMicroGame::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );
	ms_oFunctionTable.AddFunction( AFunctionPointer1Args<TINT>::Create( "PushState", &AShakeTreeMicroGame::PushState ) );
}

// $Barnyard: FUNCTION 0052c970
void AShakeTreeMicroGame::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0052d010
AShakeTreeMicroGame::AShakeTreeMicroGame()
    : m_eState( STATE_PUTTING_AWAY )
    , m_eMode( MODE_TREE )
#ifdef BARNYARD_COMMUNITY_PATCH
    // NOTE: retail never sets it (see StartKick)
    , m_fUnkB4( 0.0f )
#endif
    , m_fUnkB8( 1.5f )
    , m_iNumToSpawn( 0 )
    , m_iNumRegrow( 5 )
    , m_bSpawned( TFALSE )
    , m_bKicked( TFALSE )
    , m_iLeavesEffect( -1 )
{
	// NOTE: retail leaves m_eState and m_eMode uninitialised, they're set by OnInsertion and Setup
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( NormalBarnyard ) );
	m_oPowerBar.m_oBar.Hide();

	if ( !AGameLoader::IsBarnExploreMiniGame() )
		m_iLeavesEffect = AParticleEffectManager::FindEffect( "leavesfall" );
}

// $Barnyard: FUNCTION 0052d0f0
// $Barnyard: FUNCTION 0052db70
AShakeTreeMicroGame::~AShakeTreeMicroGame()
{
}

// $Barnyard: FUNCTION 0052d0e0
AFunctionTable* AShakeTreeMicroGame::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0052d730
void AShakeTreeMicroGame::PushState( TINT a_iType )
{
	if ( a_iType == -1 )
	{
		AAnimatedUnit*     pPlayer     = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
		APlayerSteer*      pSteer      = TSTATICCAST( APlayerSteer, pPlayer->GetSteer() );
		AAnimatableObject* pAnimObject = pPlayer->GetAnimatableObject();

		// Only with the default animation set (not while carrying something)
		TBOOL bDefaultSet = pAnimObject->GetToshiAnimInterface().GetAnimationSet().Get() == pAnimObject->GetObjectType()->GetDefaultAnimationSet().Get();

		if ( pSteer->GetState() != APlayerSteer::STATE_WALKING || !bDefaultSet )
			return;

		TVector2          vecDirection( pSteer->GetDirection()->x, pSteer->GetDirection()->z );
		ARegrowthManager* pRegrowth = ARegrowthManager::GetSingleton();

		ATreeManager::TreeInstance* pTree = ATreeManager::GetSingleton()->FindTreeInFront( *pSteer->GetPosition(), vecDirection, 2.5f );

		if ( pTree )
		{
			// A tree that has grown fruits
			TINT iType = pRegrowth->FindTypeOfGroup( pTree->uiRegrowthGroup );

			if ( pTree->uiRegrowthGroup != 0xff && ARegrowthManager::HasTypeFlag( iType, ARegrowthManager::TYPEFLAG_FRUIT ) &&
			     pRegrowth->FindRandomObjectAtStage( pTree->uiRegrowthGroup, 10000, iType ) )
			{
				TPString8 strUnitType;

				if ( iType != -1 )
					strUnitType = pRegrowth->GetUnitTypeIfInteraction( iType );

				AShakeTreeMicroGame* pState = TSTATICCAST( AShakeTreeMicroGame, TGetClass( AShakeTreeMicroGame ).CreateObject() );
				AGameStateController::GetSingleton()->GetCurrentState()->AddChildState( pState );

				if ( pTree->uiRegrowthGroup != 0xff )
					pState->Setup( pTree, strUnitType, pRegrowth->GetNumRegrow( iType ), pRegrowth->GetNumFruits( iType ), 0.0f, MODE_TREE, iType );
			}
		}
		else
		{
			// A crate
			TINT                            iType   = -1;
			ARegrowthManager::GrowthObject* pObject = pRegrowth->FindObjectInFront( *pSteer->GetPosition(), vecDirection, ARegrowthManager::TypeMask::Crates(), iType, 0.9f, 0.8f );

			if ( pObject )
			{
				TPString8 strUnitType;

				if ( iType != -1 )
					strUnitType = pRegrowth->GetUnitTypeIfInteraction( iType );

				AShakeTreeMicroGame* pState = TSTATICCAST( AShakeTreeMicroGame, TGetClass( AShakeTreeMicroGame ).CreateObject() );
				AGameStateController::GetSingleton()->GetCurrentState()->AddChildState( pState );
				pState->Setup( pObject, strUnitType, 0, 0, 0.0f, MODE_CRATE, iType );
			}
		}
	}
	else if ( a_iType == 1 )
	{
		// The henhouse
		AShakeTreeMicroGame* pState = TSTATICCAST( AShakeTreeMicroGame, TGetClass( AShakeTreeMicroGame ).CreateObject() );
		AGameStateController::GetSingleton()->GetCurrentState()->AddChildState( pState );
		pState->Setup( TNULL, TPS8( egg ), 0, 1, 0.0f, MODE_HENHOUSE, -1 );
	}
}

// $Barnyard: FUNCTION 0052d690
void AShakeTreeMicroGame::Setup( void* a_pTarget, const TPString8& a_rcUnitType, TINT a_iNumRegrow, TINT a_iNumSpawn, TFLOAT a_fTargetPower, MODE a_eMode, TINT a_iRegrowthType )
{
	m_pTarget       = a_pTarget;
	m_fUnkC4        = -3.0f;
	m_strUnitType   = a_rcUnitType;
	m_fTargetPower  = a_fTargetPower;
	m_iNumSpawn     = a_iNumSpawn;
	m_iNumRegrow    = a_iNumRegrow;
	m_eMode         = a_eMode;
	m_iRegrowthType = a_iRegrowthType;
	m_oPowerBar.SetBracketPos( a_fTargetPower );
	m_fPower = m_fTargetPower;
}

// $Barnyard: FUNCTION 0052cdd0
TBOOL AShakeTreeMicroGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( m_bKicked )
	{
		if ( a_eInputCommand == AInputCommand_Interact )
			return TTRUE;

		// The player walks away while the kick animation still plays
		AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetPlayer() );

		if ( pPlayer->GetSteer()->GetSpeed() > 0.0001f && pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( kick ), 0.0f ) )
		{
			pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( kick ) );
			pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( walk_lp ) );
		}

		return TFALSE; // RETURNS_FALSE 006528c0
	}

	switch ( a_eInputCommand )
	{
		case AInputCommand_Quit:
			AGameStateController::GetSingleton()->StartMenuIfPossible();
			return TTRUE;
		case AInputCommand_Interact:
			StartKick();
			return TTRUE;
		case AInputCommand_Shoot:
		case AInputCommand_Jump:
		case AInputCommand_Inventory:
			return TTRUE;
		default:
			return TFALSE; // RETURNS_FALSE 006528c0
	}
}

// $Barnyard: FUNCTION 0052c9f0
void AShakeTreeMicroGame::StartKick()
{
	if ( m_eState < STATE_KICKING )
	{
		// NOTE: m_fUnkB4 is never set in retail, see the constructor
		m_fPower = m_fUnkB4;
		m_eState = STATE_KICKING;
		TSTATICCAST( AAnimatedUnit, GetPlayer() )->GetAnimatableObject()->PlayAnimation( TPS8( kick ) );
	}
}

// $Barnyard: FUNCTION 0052d280
void AShakeTreeMicroGame::OnInsertion()
{
	AGameState::OnInsertion();

	TSTATICCAST( APlayerControlComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TFALSE );
	m_bKicked = TFALSE;

	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	m_pPlayer              = pPlayer->GetSelfProxy();

	if ( PutAwayHandItem() &&
	     ( pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( putaway ) ) == TTRUE || pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( putaway ), 0.0f ) == TTRUE ) )
	{
		// Kicks once the item is put away
		m_eState = STATE_PUTTING_AWAY;
	}
	else
	{
		TSTATICCAST( AAnimatedUnit, GetPlayer() )->GetAnimatableObject()->PlayAnimation( TPS8( kick ) );
		m_eState = STATE_KICKING;
	}

	if ( AGameLoader::IsBarnExploreMiniGame() )
		m_HUDParams.SetFlags( AHudElement::MASK_UNK15 | AHudElement::MASK_TIME_LEFT );
	else
		m_HUDParams.SetFlags( AHudElement::MASK_SIM );

	// NOTE: the power bar is never added to the hud manager, it isn't visible
	m_oPowerBar.SetMask( AHudElement::MASK_UNK2 );
	m_oPowerBar.Show();
	m_oPowerBar.m_oBar.Hide();
	m_oPowerBar.m_fTargetValue = 0.0f;
	m_oPowerBar.SetBracketPos( m_fTargetPower );
}

// $Barnyard: FUNCTION 0052d1b0
TBOOL AShakeTreeMicroGame::PutAwayHandItem()
{
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, GetPlayer()->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	AInventoryComponent*      pHand      = pInventory->GetCategory( TPS8D( "Hand" ) );

	if ( pHand->GetNumItems() <= 0 )
		return TFALSE;

	// NOTE: retail gets the first item of the hand here, but doesn't use it
	pHand->GetFirstItem();

	return AInventoryGroupComponent::PutAwayHandItem();
}

// $Barnyard: FUNCTION 0052c9c0
void AShakeTreeMicroGame::OnRemoval()
{
	TSTATICCAST( ACommonSteer, GetPlayer()->GetSteer() )->SetDesiredVelocity( TVector4::VEC_ZERO );
	AGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0052dc10
TBOOL AShakeTreeMicroGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	AAnimatableObject* pAnimObject = TSTATICCAST( AAnimatedUnit, GetPlayer() )->GetAnimatableObject();

	if ( ( m_eMode == MODE_HENHOUSE || m_eMode == MODE_TREE ) && m_iNumToSpawn > 0 )
	{
		m_fSpawnTimer -= a_fDeltaTime;

		while ( m_fSpawnTimer < 0.0f )
		{
			m_iNumToSpawn--;
			m_fSpawnTimer += m_fSpawnInterval;
			SpawnUnit();
		}
	}

	if ( m_eState != STATE_PUTTING_AWAY )
	{
		if ( m_fPower >= 0.0f )
			m_oPowerBar.m_fTargetValue = m_fPower;

		if ( m_eState == STATE_KICKING )
		{
			AAnimatedUnit*     pPlayer    = TSTATICCAST( AAnimatedUnit, GetPlayer() );
			AAnimatableObject* pPlayerObj = pPlayer->GetAnimatableObject();

			if ( !pPlayerObj->IsAnimationPlanned( TPS8( kick ) ) &&
			     ( pPlayerObj->IsAnimationFinished( TPS8( kick ) ) || pPlayer->GetAnimatableObject()->GetAnimationTime( TPS8( kick ) ) > 0.26f ) )
			{
				OnKick();
				m_eState = STATE_KICKED;
				TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TTRUE );
				m_bKicked = TTRUE;
			}
		}
		else if ( m_eState == STATE_KICKED )
		{
			AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, GetPlayer() );

			if ( pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( kick ) ) )
			{
				TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TTRUE );
				m_bKicked = TTRUE;
				Remove();
				return TTRUE;
			}
		}
	}
	else if ( pAnimObject->IsAnimationFinished( TPS8( putaway ) ) )
	{
		TSTATICCAST( AAnimatedUnit, GetPlayer() )->GetAnimatableObject()->PlayAnimation( TPS8( kick ) );
		m_eState = STATE_KICKING;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0052ca50
void AShakeTreeMicroGame::OnKick()
{
	AInputHandler::RumbleParams oRumble = { { 1.0f, 0.0f, 0.0f, 0.7f, 0.0f, 1.0f, 1.0f }, 2 };

	if ( m_eMode == MODE_TREE )
	{
		ATreeManager::TreeInstance* pTree = TSTATICCAST( ATreeManager::TreeInstance, m_pTarget );

		// The leaves fall from the branches
		TINT iNumFOBs = ATreeManager::GetSingleton()->GetNumFOBs( pTree );

		for ( TINT i = 0; i < iNumFOBs; i++ )
		{
			TVector4 vecPosition = ATreeManager::GetSingleton()->GetFOBPosition( pTree, i );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iLeavesEffect, vecPosition, TVector4::VEC_NEGY );
		}

		// NOTE: inlined StartSpawning, the closer the power is to the target the faster the fruits fall
		TINT iNumSpawns = m_iNumRegrow - TINT( TMath::Abs( m_fPower - m_fTargetPower ) * 4.0f * TFLOAT( m_iNumRegrow ) );

		if ( iNumSpawns < 1 )
			iNumSpawns = 1;

		iNumSpawns = TMath::Max( iNumSpawns, 3 );

		if ( iNumSpawns > 0 && ( m_iRegrowthType != -1 || m_eMode == MODE_HENHOUSE ) )
		{
			m_iNumToSpawn    = m_iNumSpawn;
			m_fSpawnInterval = 0.7f / TFLOAT( iNumSpawns );
			m_fSpawnTimer    = m_fSpawnInterval;
		}

		AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::BARNYARD_TREE_THUMP, pTree->GetPosition() );
	}
	else if ( m_eMode == MODE_CRATE )
	{
		KnockOverCrate( TSTATICCAST( ARegrowthManager::GrowthObject, m_pTarget ), m_iRegrowthType );
	}
	else if ( m_eMode == MODE_HENHOUSE )
	{
		StartSpawning( 1 );
		AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
	}
}

// $Barnyard: FUNCTION 0052c980
void AShakeTreeMicroGame::StartSpawning( TINT a_iNumSpawns )
{
	if ( a_iNumSpawns > 0 && ( m_iRegrowthType != -1 || m_eMode == MODE_HENHOUSE ) )
	{
		m_iNumToSpawn    = m_iNumSpawn;
		m_fSpawnInterval = 0.7f / TFLOAT( a_iNumSpawns );
		m_fSpawnTimer    = m_fSpawnInterval;
	}
}

// $Barnyard: FUNCTION 0052d420
void AShakeTreeMicroGame::SpawnUnit()
{
	TVector4 vecPosition;

	if ( m_eMode == MODE_HENHOUSE )
	{
		TVector4 vecEffectPosition( s_vecHenhousePosition.x, s_vecHenhousePosition.y - 0.1f, s_vecHenhousePosition.z, 1.0f );
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::BARNYARD_BARN_THUMP, vecEffectPosition );
		AParticleEffectManager::GetSingleton()->PlayEffect( AParticleEffectManager::FindEffect( "PeckFeth_explode" ), vecEffectPosition, TVector4::VEC_NEGY );

		vecPosition = s_vecHenhousePosition;
	}
	else if ( !ARegrowthManager::GetSingleton()->TakeRandomObject( TSTATICCAST( ATreeManager::TreeInstance, m_pTarget )->uiRegrowthGroup, vecPosition ) )
	{
		return;
	}

	AUnitType* pType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( m_strUnitType );
	AUnit*     pUnit = AUnitManager::GetSingleton()->SpawnUnit( pType, vecPosition, TVector4::VEC_POSX );

	TVector4 vecVelocity;

	if ( m_eMode == MODE_HENHOUSE )
	{
		vecVelocity = TVector4( 0.5f, -0.1f, -0.8f, 1.0f );
		vecVelocity.Multiply( 3.4f );
		pUnit->ChangeSteer( &TGetClass( AThrowItemSteer ) );
	}
	else
	{
		// Away from the trunk
		vecVelocity.Substract( vecPosition, TSTATICCAST( ATreeManager::TreeInstance, m_pTarget )->GetPosition() );
		vecVelocity.y = 0.0f;
		vecVelocity.Normalise();
		vecVelocity.Multiply( 1.4f );
	}

	m_bSpawned = TTRUE;
	pUnit->GetSteer()->SetVelocity( vecVelocity );

	// Waits, fades out and dies
	AWaitAniEffect* pWait = AAniEffectManager::GetSingleton()->CreateEffect<AWaitAniEffect>();
	AFadeAniEffect* pFade = AAniEffectManager::GetSingleton()->CreateEffect<AFadeAniEffect>();
	ADieAniEffect*  pDie  = AAniEffectManager::GetSingleton()->CreateEffect<ADieAniEffect>();

	pDie->SetUnit( pUnit );
	pFade->SetUnit( pUnit );
	pFade->SetNextEffect( pDie );
	pWait->SetNextEffect( pFade );

	if ( m_eMode == MODE_HENHOUSE || m_iNumToSpawn <= m_iNumRegrow )
	{
		pFade->SetSpeed( 0.5f );
		pWait->SetTime( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 15.0f, 20.0f ) );
	}
	else
	{
		// The fruits the tree doesn't regrow fade out quickly
		pFade->SetSpeed( 2.0f );
		pWait->SetTime( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.25f, 0.5f ) );
	}
}

// $Barnyard: FUNCTION 0052db90
void AShakeTreeMicroGame::OnDeactivate()
{
	AGameState::OnDeactivate();

	// Spawns the rest at once
	while ( --m_iNumToSpawn >= 0 )
		SpawnUnit();

	if ( m_eMode == MODE_HENHOUSE )
	{
		// The egg is taken
		ARegrowthManager::GetSingleton()->DecrementRespawned();
	}
	else if ( m_bSpawned == TTRUE )
	{
		// The tree regrows its fruits
		TUINT uiGroup = TSTATICCAST( ATreeManager::TreeInstance, m_pTarget )->uiRegrowthGroup;
		ARegrowthManager::GetSingleton()->EmptyGroup( uiGroup );
		ARegrowthManager::GetSingleton()->GrowFruits( uiGroup, m_iNumRegrow, 0.0f );
	}
}
