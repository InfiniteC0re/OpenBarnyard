#include "pch.h"
#include "ASpeechSeqActivity.h"
#include "Helpers/ASpeechDialogManager.h"
#include "ASpeechActivity.h"
#include "AAnimalPopulationManager.h"
#include "AAnimatedUnit.h"
#include "ACharacterUnit.h"
#include "AInteractionSequence.h"
#include "AInteractionComponent.h"
#include "AUnitFacialComponent.h"
#include "AUnitNameManager.h"
#include "AUnitPlayer.h"
#include "AUnitType.h"
#include "ALoadScreen.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ASpeechCameraHelper.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Quests/AQuestManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Physics/ACollisionManager.h"
#include "MiniGame/ABarnExploreMinigameState.h"

#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c0a10, CreateObject 005c1890, CreateObjectInPlace 005c1910
TDEFINE_CLASS( ASpeechSeqActivity );

TPSTRING8_DECLARE( action );
TPSTRING8_DECLARE( convpoint_01 );
TPSTRING8_DECLARE( camera );
TPSTRING8_DECLARE( model );
TPSTRING8_DECLARE( goal );
TPSTRING8_DECLARE( swapsetupactors );
TPSTRING8_DECLARE( setupactors );
TPSTRING8_DECLARE( setupactorsskipposition );
TPSTRING8_DECLARE( setupactorsfromlocatorskippos );
TPSTRING8_DECLARE( setupactorsfromlocator );
TPSTRING8_DECLARE( resetactors );
TPSTRING8_DECLARE( bubble );
TPSTRING8_DECLARE( value );
TPSTRING8_DECLARE( unsetactors );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_aux );
TPSTRING8_DECLARE( root );
TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( ABarnExploreMinigameState );

// NOTE: inlined in CreateObject
ASpeechSeqActivity::ASpeechSeqActivity()
    : m_uiFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 005c0c60
// $Barnyard: FUNCTION 005c1980
ASpeechSeqActivity::~ASpeechSeqActivity()
{
}

// $Barnyard: FUNCTION 005c0d00
TBOOL ASpeechSeqActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strAction;
	a_pProperties->GetOptionalPropertyValue( strAction, TPS8( action ) );

	m_strModel = TPS8( convpoint_01 );

	if ( strAction == TPS8( camera ) )
	{
		m_uiFlags |= FLAGS_CAMERA;
		a_pProperties->GetOptionalPropertyValue( m_strModel, TPS8( model ) );
	}
	else if ( strAction == TPS8( swapsetupactors ) )
	{
		m_uiFlags |= FLAGS_SWAP | FLAGS_SETUP_ACTORS;
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8( goal ) );
	}
	else if ( strAction == TPS8( setupactors ) )
	{
		m_uiFlags |= FLAGS_SETUP_ACTORS;
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8( goal ) );
	}
	else if ( strAction == TPS8( setupactorsskipposition ) )
	{
		m_uiFlags |= FLAGS_SKIP_POSITION | FLAGS_SETUP_ACTORS;
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8( goal ) );
	}
	else if ( strAction == TPS8( setupactorsfromlocatorskippos ) )
	{
		m_uiFlags |= FLAGS_FROM_LOCATOR | FLAGS_SKIP_POSITION | FLAGS_SETUP_ACTORS;
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8( goal ) );
	}
	else if ( strAction == TPS8( setupactorsfromlocator ) )
	{
		m_uiFlags |= FLAGS_FROM_LOCATOR | FLAGS_SETUP_ACTORS;
		a_pProperties->GetOptionalPropertyValue( m_strGoal, TPS8( goal ) );
	}
	else if ( strAction == TPS8( resetactors ) )
	{
		m_uiFlags |= FLAGS_RESET_ACTORS;
	}
	else if ( strAction == TPS8( bubble ) )
	{
		TBOOL bValue = TFALSE;
		a_pProperties->GetOptionalPropertyValue( bValue, TPS8( value ) );

		if ( bValue )
			m_uiFlags |= FLAGS_BUBBLE_ON;
		else
			m_uiFlags |= FLAGS_BUBBLE_OFF;
	}
	else if ( strAction == TPS8( unsetactors ) )
	{
		m_uiFlags |= FLAGS_UNSET_ACTORS;
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c0a20
AInteractionActivity::RESULT ASpeechSeqActivity::OnStart( AInteractionSequence* a_pSequence )
{
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();

	if ( m_uiFlags & FLAGS_CAMERA )
	{
		// Switch to the speech camera
		pCameraManager->SetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH );
		ASpeechCameraHelper* pSpeechHelper = TSTATICCAST( ASpeechCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH ) );

		pSpeechHelper->SetModel( m_strModel.GetString() );
		pSpeechHelper->ResetCamera();
		pSpeechHelper->HideHUD();

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
			AAnimalPopulationManager::GetSingleton()->SetMarkerActive( it->GetOwner(), TFALSE );

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_RESET_ACTORS )
	{
		// Move the actors back to where they were
		ASpeechCameraHelper* pSpeechHelper = TSTATICCAST( ASpeechCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH ) );

		if ( !pSpeechHelper )
			return RESULT_DONE;

		pSpeechHelper->Stop( TFALSE );
		pSpeechHelper->RestoreHUD();

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();
			AAnimalPopulationManager::GetSingleton()->SetMarkerActive( pUnit, TTRUE );

			if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
				TSTATICCAST( APlayerSteer, pUnit->GetSteer() )->SetState( APlayerSteer::STATE_WALKING );
			else if ( ASpeechActivity::HasAnim( pUnit, TPS8( idle_lp ) ) )
				ASpeechActivity::PlayAnim( pUnit, TPS8( idle_lp ), TTRUE );
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_UNSET_ACTORS )
	{
		// Leave the actors where they are
		ASpeechCameraHelper* pSpeechHelper = TSTATICCAST( ASpeechCameraHelper, pCameraManager->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH ) );

		if ( pSpeechHelper )
		{
			pSpeechHelper->ReleaseSpeakers();
			pSpeechHelper->RestoreHUD();

			for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
			{
				AUnit* pUnit = it->GetOwner();
				AAnimalPopulationManager::GetSingleton()->SetMarkerActive( pUnit, TTRUE );

				if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
					TSTATICCAST( APlayerSteer, pUnit->GetSteer() )->SetState( APlayerSteer::STATE_WALKING );
			}
		}
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005c19a0
AInteractionActivity::RESULT ASpeechSeqActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( m_uiFlags & FLAGS_BUBBLE_ON )
	{
		// Wait until the bubble is open
		if ( !ASpeechDialogManager::GetSingleton()->PrepareSpeech() )
			return RESULT_RUNNING;
	}
	else if ( m_uiFlags & FLAGS_BUBBLE_OFF )
	{
		// Wait until the bubbles are closed
		if ( !ASpeechDialogManager::GetSingleton()->EndAllSpeeches() )
			return RESULT_RUNNING;
	}
	else if ( m_uiFlags & FLAGS_SETUP_ACTORS )
	{
		return SetupActors( a_pSequence );
	}

	return RESULT_DONE;
}

// $Barnyard: FUNCTION 005c0eb0
AInteractionActivity::RESULT ASpeechSeqActivity::SetupActors( AInteractionSequence* a_pSequence )
{
	static TFLOAT s_fLoadingTime = 0.0f; // 0x0078bb60

	if ( !( m_uiFlags & FLAGS_LOADING ) )
	{
		if ( !ATerrainInterface::GetSingleton()->IsLoaded() )
		{
			// Wait for the terrain behind the loading screen
			m_uiFlags |= FLAGS_LOADING;
			s_fLoadingTime = 0.0f;
			g_oLoadScreen.StartLoading( 100, TTRUE );

			return RESULT_RUNNING;
		}
	}
	else
	{
		s_fLoadingTime += g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

		if ( s_fLoadingTime > 0.5f && ATerrainInterface::GetSingleton()->IsLoaded() )
		{
			m_uiFlags &= ~FLAGS_LOADING;
			g_oLoadScreen.Reset();
			g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
			AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TFALSE, 1.0f, TFALSE );
		}

		if ( g_oLoadScreen.IsLoadingScreen() )
		{
			g_oLoadScreen.Update( 1.0f, TTRUE );
			return RESULT_RUNNING;
		}
	}

	ASpeechCameraHelper* pSpeechHelper = TSTATICCAST( ASpeechCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_SPEECH ) );
	pSpeechHelper->ClearSpeakers();

	TVector4 vecPosition     = TVector4::VEC_ZERO;
	TINT     iArea           = -1;
	TBOOL    bAtQuestLocator = TFALSE;

	if ( !( m_uiFlags & FLAGS_FROM_LOCATOR ) )
	{
		// The quest locator of one of the units
		TMatrix44 matLocator;
		TINT      iNumUnits = a_pSequence->GetComponents().Size();
		TBOOL     bFound    = TFALSE;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			AUnit* pUnit = a_pSequence->GetUnit( i );

			if ( !pUnit->IsA( &TGetClass( AUnitPlayer ) ) &&
			     AAnimalPopulationManager::FindQuestLocator( matLocator, 1, pUnit->GetType()->GetName().GetString(), AQuestManager::GetSingleton()->GetAct(), AQuestManager::GetSingleton()->GetTier() ) )
			{
				bFound = TTRUE;
				break;
			}
		}

		if ( bFound )
		{
			TVector4 vecLocator = matLocator.GetTranslation();
			pSpeechHelper->SetPosition( vecLocator );

			TVector4 vecDirection( -matLocator.m_f21, 0.0f, -matLocator.m_f23, 1.0f );
			vecDirection.Normalise();
			pSpeechHelper->SetDirection( vecDirection );

			bAtQuestLocator = TTRUE;
		}
		else if ( m_strGoal.IsEmpty() )
		{
			// The quest goal of one of the units
			TBOOL bAtGoal = TFALSE;
			iNumUnits     = a_pSequence->GetComponents().Size();

			for ( TINT i = 0; i < iNumUnits; i++ )
			{
				AUnit* pUnit = a_pSequence->GetUnit( i );

				if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
					continue;

				if ( AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() && pUnit->GetType()->GetName() == TPS8( root ) )
				{
					// Root in the night barn
					ABarnExploreMinigameState* pBarnState = TSTATICCAST( ABarnExploreMinigameState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) ) );

					if ( pBarnState )
					{
						// The goal of root on the floor he's on
						AInteriorGoal* pRootGoal = pBarnState->GetRootGoal();

						if ( pRootGoal )
						{
							pRootGoal->GetPosition( vecPosition );
							iArea = pRootGoal->m_iAreaIndex;
							pSpeechHelper->SetPosition( vecPosition );

							// Done placing the camera rig
							bAtGoal = TTRUE;
							break;
						}
					}
				}
				else
				{
					const TCHAR* szGoal = AAnimalPopulationManager::GetSingleton()->GetQuestTierGoal( pUnit->GetType()->GetName().GetString() );

					if ( szGoal && szGoal[ 0 ] != '\0' )
					{
						AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoal( TPString8( szGoal ) );

						if ( pGoal->GetPosition( vecPosition ) == TTRUE )
							pGoal->UpdateHeight();

						iArea   = pGoal->m_iAreaIndex;
						bAtGoal = TTRUE;
						pSpeechHelper->SetPosition( vecPosition );
					}
				}
			}

			if ( !bAtGoal )
				pSpeechHelper->SetPosition( TVector4::VEC_ZERO );
		}
		else
		{
			// The goal of the activity
			AInteriorGoal* pGoal = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork()->FindGoal( m_strGoal );

			if ( pGoal->GetPosition( vecPosition ) == TTRUE )
				pGoal->UpdateHeight();

			iArea = pGoal->m_iAreaIndex;
			pSpeechHelper->SetPosition( vecPosition );
		}
	}
	else
	{
		// The locator of the activity
		TMatrix44   matLocator;
		TVector4    vecLocatorPos;
		TQuaternion quatLocatorRot;
		ATerrainInterface::GetSingleton()->GetLocatorTransform( m_strGoal.GetString(), matLocator );
		ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( m_strGoal.GetString(), vecLocatorPos, quatLocatorRot );

		pSpeechHelper->SetTransform( matLocator );
		vecPosition = matLocator.GetTranslation();
	}

	// The actors: pip first at the pip post, then the player and the other characters
	// NOTE: retail keeps them in a 12 dword buffer on the stack (the locator matrix)
	AUnit* apUnits[ 12 ];
	TINT   iNumActors = 0;
	TINT   iPlayer    = -1;
	TINT   iSpeaker   = -1;

	if ( m_uiFlags & FLAGS_SWAP )
	{
		// NOTE: retail walks the units of the category here without doing anything
	}

	if ( pSpeechHelper->GetModelIndex() == 2 )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit* pUnit = it->GetOwner();

			if ( pUnit->GetId() == AUnitNameManager::GetSingleton()->GetUnitId( TPS8( pip ) ) )
			{
				iSpeaker     = 0;
				apUnits[ 0 ] = pUnit;
				iNumActors   = 1;
				break;
			}
		}
	}

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
		{
			iPlayer                 = iNumActors;
			apUnits[ iNumActors++ ] = pUnit;
		}
	}

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AUnit* pUnit = it->GetOwner();

		if ( !pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
			continue;

		TBOOL bAdded = TFALSE;

		for ( TINT i = 0; i < iNumActors; i++ )
		{
			if ( pUnit == apUnits[ i ] )
				bAdded = TTRUE;
		}

		if ( !bAdded )
		{
			if ( iSpeaker < 0 )
				iSpeaker = iNumActors;

			apUnits[ iNumActors++ ] = pUnit;
		}
	}

	if ( !( m_uiFlags & FLAGS_FROM_LOCATOR ) && !bAtQuestLocator )
	{
		// Turn the camera rig the way the player (or the speaker) looks
		if ( pSpeechHelper->GetModelIndex() == 0 )
			pSpeechHelper->SetDirection( *apUnits[ iPlayer ]->GetSteer()->GetDirection() );
		else if ( pSpeechHelper->GetModelIndex() == 1 )
			pSpeechHelper->SetDirection( *apUnits[ iSpeaker ]->GetSteer()->GetDirection() );
	}

	for ( TINT i = 0; i < iNumActors; i++ )
	{
		AUnit* pUnit = apUnits[ i ];
		pSpeechHelper->AddSpeaker( pUnit );

		if ( ( m_uiFlags & FLAGS_SKIP_POSITION ) && !pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
			continue;

		ASteer* pSteer = pUnit->GetSteer();
		pSteer->SetInteracting( 1 );
		pSteer->Halt();

		TVector4 vecSpot;
		TVector4 vecDirection;
		pSpeechHelper->GetSpeakerPosition( i, vecSpot );
		pSpeechHelper->GetSpeakerDirection( i, vecDirection );

		if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
		{
			// Put the player on the ground
			TVector4 vecStart( vecSpot.x, vecSpot.y - 1.5f, vecSpot.z, vecSpot.w );
			ARayData oRay( vecStart, TVector4::VEC_POSY, 40.0f );

			TFLOAT fDistance = -1.0f;

			if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) )
				vecSpot.y = fDistance * 0.9f + vecStart.y;
		}

		pSteer->HaltAtPosition( TVector3( vecSpot.x, vecSpot.y, vecSpot.z ) );

		if ( !pSteer->IsA( &TGetClass( APlayerSteer ) ) )
			pSteer->Command_CancelCurrentCommand();

		if ( iArea != -1 && pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
			TSTATICCAST( AInteriorSteer, pSteer )->SetStartArea( iArea );

		if ( pUnit->IsA( &TGetClass( AUnitPlayer ) ) )
			TSTATICCAST( APlayerSteer, pSteer )->SetState( APlayerSteer::STATE_STILL );

		pSteer->HaltFacing( TVector3( vecDirection.x, vecDirection.y, vecDirection.z ) );

		if ( !pUnit->IsPlayer() )
		{
			ASpeechActivity::PlayAnim( pUnit, ASpeechActivity::HasAnim( pUnit, TPS8( idle_aux ) ) ? TPS8( idle_aux ) : TPS8( idle_lp ), TTRUE );

			AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject();

			if ( pAnimatableObject->GetAttachmentInfo().pLocator )
			{
				AModelInstanceRef pModelInstance = pAnimatableObject->GetModelInstance();
				pModelInstance->SetTranslation( TVector3( vecSpot.x, vecSpot.y, vecSpot.z ) );
			}
		}
	}

	// Show the speaker and make everyone look at each other
	AUnit* pSpeaker = apUnits[ iSpeaker ];
	pSpeechHelper->ShowSpeaker( 1, pSpeaker->GetId() );

	for ( TINT i = 0; i < iNumActors; i++ )
	{
		AUnit*                pUnit   = apUnits[ i ];
		AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, pUnit->FindComponentExact( &TGetClass( AUnitFacialComponent ) ) );

		if ( pFacial && ( !pUnit->IsA( &TGetClass( AAnimatedUnit ) ) || !TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetParentObject() ) )
		{
			if ( i == iPlayer )
				pFacial->LookAtUnitHead( pSpeaker, -1.0f );
			else
				pFacial->LookAtUnitHead( apUnits[ iPlayer ], -1.0f );
		}
	}

	return RESULT_DONE;
}
