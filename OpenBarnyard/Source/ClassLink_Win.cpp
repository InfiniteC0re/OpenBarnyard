#include "pch.h"
#include "ClassLink_Win.h"
#include "GameInterface/ASleepCycleChooseState.h"
#include "GameInterface/ASleepCycleState.h"
#include "MiniGame/ATreasueHuntMicroGame.h"

#include "Animation/AAnimationBreakpointAnimation.h"
#include "Animation/AAnimationBreakpointAttachment.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Animation/AAnimationBreakpointDebug.h"
#include "Animation/AAnimationBreakpointInventory.h"
#include "Animation/AAnimationBreakpointSoundParticle.h"
#include "Animation/AAnimationBreakpointUnitAction.h"
#include "Animation/AAlienAnimControllerType.h"
#include "Animation/AAttachedAnimControllerType.h"
#include "Animation/ABasicAnimControllerType.h"
#include "Animation/ABikeAnimControllerType.h"
#include "Animation/ABikeCrashAnimControllerType.h"
#include "Animation/ACharacterAnimControllerType.h"
#include "Animation/AChasingChicksAnimControllerType.h"
#include "Animation/ADartThrowAnimControllerType.h"
#include "Animation/AFallingCowAnimControllerType.h"
#include "MiniGame/AGateCrashMinigameState.h"
#include "MiniGame/ACarChaseMiniGameState.h"
#include "MiniGame/AGateCrashPlayerAi.h"
#include "MiniGame/AVeggiePatchDefenderGame.h"
#include "MiniGame/AHoneyCollectingGame.h"
#include "Units/ABeeSwarm.h"
#include "Units/AVeggiePatchThief.h"
#include "Units/AUnitCarChaseCar.h"
#include "Units/AVeggiePatchVeggie.h"
#include "MiniGame/ACowTippingMiniGameState.h"
#include "MiniGame/AStatuesMiniGameState.h"
#include "MiniGame/AStatuesMiniGameState.h"
#include "MiniGame/AWhackARaccMiniGameState.h"
#include "MiniGame/AChickenFireMinigameState.h"
#include "MiniGame/AGolfMinigameState.h"
#include "MiniGame/ABarnExploreMinigameState.h"
#include "MiniGame/ABarnYardPoolMiniGameState.h"
#include "MiniGame/ATapperMiniGameState.h"
#include "MiniGame/APrecisionSquirt.h"
#include "Units/APrecisionCoyote.h"
#include "Units/APrecisionJersyCow.h"
#include "Units/APrecisionRacoon.h"
#include "Units/APrecisionScarecrow.h"
#include "Units/APrecisionTargetItem.h"
#include "Units/ASpawnPoint.h"
#include "GameInterface/AMiniGameInstructionState.h"
#include "GameInterface/ASimState.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AAlienSteer.h"
#include "Units/APlayerControlComponent.h"
#include "UnitMovement/Steering/ASteerKinematics3D.h"
#include "UnitMovement/Steering/ABikeSteer.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "UnitMovement/Steering/ASSeqSteer.h"
#include "Units/ARumourRequirement.h"
#include "UnitMovement/Steering/AProjectileItemSteer.h"
#include "UnitMovement/Steering/ACarHeliSteer.h"
#include "Units/ARespawnComponent.h"
#include "Physics/APrim.h"
#include "Physics/AGeomIntersectResult.h"
#include "Units/ABikeControlComponent.h"
#include "Units/AMovableUnit.h"
#include "Units/AUnitPoolBall.h"
#include "Units/AUnitPoolBfa.h"
#include "Units/AUnitPoolCue.h"
#include "UnitMovement/Steering/AFallingCowSteer.h"
#include "Units/AAnimalAttributeData.h"
#include "Units/AUnitBehaviourData.h"
#include "Units/AInventoryComponentData.h"
#include "Units/AInventoryGroupComponentData.h"
#include "Units/AInventoryComponent.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AInventoryItem.h"
#include "Units/ACarryableUnit.h"
#include "Units/ACharacterUnitCarrable.h"
#include "UnitMovement/Steering/ABallSteer.h"
#include "UnitMovement/Steering/ABallSteer2.h"
#include "UnitMovement/Steering/APoolBallSteer.h"
#include "Units/AGiftComponent.h"
#include "Units/AUnitPlayerKnowledgeComponent.h"
#include "Units/AFearRequirement.h"
#include "Units/ATopicTypeRequirement.h"
#include "Units/AInteractionCoyoteEvent.h"
#include "Units/AInteractionFearEvent.h"
#include "Units/AInteractionGiftEvent.h"
#include "Units/AInteractionPlayerInsultEvent.h"
#include "Units/AInteractionQuestEvent.h"
#include "FSM/AFunctionCollection.h"
#include "Units/ANPCBehaviourComponentData.h"
#include "Units/AUnitGameEventData.h"
#include "Units/AUnitFacialComponent.h"
#include "Units/AModelAttachComponent.h"
#include "Units/AModelAttachComponentData.h"
#include "Units/ASoundComponent.h"
#include "Units/ADriveSystem.h"
#include "Units/AAnimalAttributeDriveFunction.h"
#include "Units/AHysteresisDriveFunction.h"
#include "Units/ATestDriveFunction.h"

#include <Platform/DX8/Input/TInputInterface_Win.h>
#include <Platform/DX8/TTextureFactoryHAL_DX8.h>
#include "Quests/AQuestManager.h"
#include "Quests/ADummyQuestElement.h"
#include "Quests/AGameEventQuestElement.h"
#include "Quests/AGiftQuestElement.h"
#include "Quests/ALocateQuestElement.h"
#include "GameInterface/AHintDisplayState.h"
#include "GameInterface/AMapState.h"
#include "GameInterface/ARecipeDisplayState.h"
#include "MiniGame/ACookingMiniGame.h"
#include "GameInterface/AInventoryState2.h"
#include "GameInterface/AMobilePhoneUIState2.h"
#include "GameInterface/AGamesPlayedDisplayState.h"
#include "Helpers/AHintManager.h"
#include "Helpers/AHighScoreManager.h"
#include "MiniGame/AGopherShopInterface.h"
#include "MiniGame/AGopherShopGame.h"
#include "HUD/ASimMiniMap.h"
#include "HUD/AItemCountHudElement.h"
#include "HUD/AGameTimeHudElement.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "HUD/AInteractionHudElement.h"
#include "HUD/AMenuNavHudElement.h"
#include "HUD/ATerrainSectionHudElement.h"
#include "Units/AUnitBehaviour.h"
#include "Units/AKinematicsObstructionActivityMessage.h"
#include "Units/AWorldPositionActivityMessage.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/Steering/AInteriorSteer.h"

#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Platform/DX8/TIndexFactoryResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>
#include <Platform/DX8/TVertexFactoryResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>

// NOTE: includes the physics headers (Opcode defines PI), keep it last
#include "MiniGame/ADartMiniGame.h"
#include "MiniGame/ABikeRaceMicroGame.h"
#include "MiniGame/ACoinPickupMicroGame.h"
#include "MiniGame/AFarmerTrainMicroGame.h"
#include "GUI/AGUIGameHudChange.h"
#include "HUD/AContextButtonHudElement.h"
#include "MiniGame/ASuperSquirtMicroGame.h"
#include "GameInterface/AFrontEndAttractState.h"
#include "GameInterface/AEndGameState.h"
#include "MiniGame/AShakeTreeMicroGame.h"
#include "MiniGame/AMilkCanMicroGame.h"
#include "MiniGame/AChasingChicksMiniGame.h"
#include "Units/AChasingChicksAI.h"
#include "Units/AChasingChicksChick.h"
#include "Units/AGolfGopherUnit.h"
#include "UnitMovement/Steering/AGopherMoundSteer.h"
#include "Units/AUnitGolfProjectile.h"
#include "Units/AUnitAesthetic.h"
#include "Units/AUnitDummy.h"
#include "Units/AStaticUnit.h"
#include "Units/AChasingChicksHanna.h"
#include "MiniGame/AChickenCoopDefender.h"
#include "Units/AChickenCoopDefenderHen.h"
#include "Units/AChickenCoopDefenderEgg.h"
#include "Units/AChickenCoopDefenderThief.h"
#include "Units/AManualPlayerControlComponent.h"
#include "Units/ANPCMilkLauncherComponent.h"
#include "Units/ACoinPickupCoinItem.h"
#include "Units/ANPCMilkLauncherComponentData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#define REFERENCE_CLASS( CLASS_NAME ) static TANONYMOUSVAR( const TCHAR* ) = CLASS_NAME::GetClassStatic()->GetName()

// NOTE: retail copies each TClass into a local and destroys it
// $Barnyard: FUNCTION 00606370
void ClassLink_Win::ReferenceGameClasses()
{
	REFERENCE_CLASS( AAnimalAttributeData );
	REFERENCE_CLASS( AUnitBehaviour );
	REFERENCE_CLASS( ADriveSystem );
	REFERENCE_CLASS( ATestDriveFunction );
	REFERENCE_CLASS( AAnimalAttributeDriveFunction );
	REFERENCE_CLASS( AHysteresisDriveFunction );
	REFERENCE_CLASS( AUnitAesthetic );
	REFERENCE_CLASS( AInteriorNetwork );
	REFERENCE_CLASS( AInteriorSteer );
	REFERENCE_CLASS( AKinematicsObstructionActivityMessage );
	REFERENCE_CLASS( AWorldPositionActivityMessage );
}

// NOTE: retail copies each TClass into a local and destroys it
// $Barnyard: FUNCTION 006064b0
void ClassLink_Win::ReferenceEngineClasses()
{
	REFERENCE_CLASS( TRenderD3DInterface );
	REFERENCE_CLASS( TTextureFactoryHAL );
	REFERENCE_CLASS( TTextureResourceHAL );
	REFERENCE_CLASS( TIndexFactoryResource );
	REFERENCE_CLASS( TIndexPoolResource );
	REFERENCE_CLASS( TVertexFactoryResource );
	REFERENCE_CLASS( TVertexPoolResource );
	REFERENCE_CLASS( TInputDXInterface );
}

void ClassLink_Win::ReferenceClasses()
{
	ReferenceGameClasses();
	ReferenceEngineClasses();

	REFERENCE_CLASS( TTextureFactoryHAL );
	REFERENCE_CLASS( TInputDXInterface );
	REFERENCE_CLASS( TInputInterface );

	// Animation breakpoints (created by name in AAnimationBreakpoint::CreateBreakpoint and ANamedAnimation::CreateBreakpointRuntime)
	REFERENCE_CLASS( AAnimationBreakpointAnimation );
	REFERENCE_CLASS( AAnimationBreakpointAttachment );
	REFERENCE_CLASS( AAnimationBreakpointCallback );
	REFERENCE_CLASS( AAnimationBreakpointDebug );
	REFERENCE_CLASS( AAnimationBreakpointInventory );
	REFERENCE_CLASS( AAnimationBreakpointSoundParticle );
	REFERENCE_CLASS( AAnimationBreakpointUnitAction );

	// Animation controller types (created by name in AAnimatableObjectType::SetAnimController)
	REFERENCE_CLASS( AAlienAnimControllerType );
	REFERENCE_CLASS( AAttachedAnimControllerType );
	REFERENCE_CLASS( ABasicAnimControllerType );
	REFERENCE_CLASS( ABikeAnimControllerType );
	REFERENCE_CLASS( ABikeCrashAnimControllerType );
	REFERENCE_CLASS( ACharacterAnimControllerType );
	REFERENCE_CLASS( AChasingChicksAnimControllerType );
	REFERENCE_CLASS( ADartThrowAnimControllerType );
	REFERENCE_CLASS( AFallingCowAnimControllerType );

	// Minigames (created by name through AMiniGameManager)
	REFERENCE_CLASS( ADartMiniGame );
	REFERENCE_CLASS( ABikeRaceMicroGame );
	REFERENCE_CLASS( ACoinPickupMicroGame );     // pushed by name by the lever of the coin spawners
	REFERENCE_CLASS( AFarmerTrainMicroGame );    // never created in retail, only by name
	REFERENCE_CLASS( AGUIGameHudChange );        // never created in retail, only by name
	REFERENCE_CLASS( AContextButtonHudElement ); // never created in retail, only by name
	REFERENCE_CLASS( ASuperSquirtMicroGame );    // pushed by name at the "highsquirt_NN" spots
	REFERENCE_CLASS( AEndGameState );            // AGameLoader::SetTransitionToGameState( "AEndGameState" )
	REFERENCE_CLASS( AFrontEndAttractState );    // never created in retail, only by name (-devattract)
	REFERENCE_CLASS( AGateCrashMinigameState );
	REFERENCE_CLASS( ACarChaseMiniGameState );
	REFERENCE_CLASS( AGateCrashPlayerAi );
	REFERENCE_CLASS( ACowTippingMiniGameState );
	REFERENCE_CLASS( AStatuesMiniGameState );
	REFERENCE_CLASS( AStatuesMiniGameState );
	REFERENCE_CLASS( AWhackARaccMiniGameState );
	REFERENCE_CLASS( AChickenFireMinigameState );
	REFERENCE_CLASS( AGolfMinigameState );
	REFERENCE_CLASS( ABarnExploreMinigameState );
	REFERENCE_CLASS( ABarnYardPoolMiniGameState );
	REFERENCE_CLASS( ATapperMiniGameState );
	REFERENCE_CLASS( ACookingMiniGame ); // started by name (startgamestate)
	REFERENCE_CLASS( AVeggiePatchDefenderGame );
	REFERENCE_CLASS( AHoneyCollectingGame );
	REFERENCE_CLASS( ABeeSwarm );
	REFERENCE_CLASS( AChasingChicksMiniGame );
	REFERENCE_CLASS( AChasingChicksAI );
	REFERENCE_CLASS( AChasingChicksChick );
	REFERENCE_CLASS( AChasingChicksHanna );
	REFERENCE_CLASS( AChickenCoopDefender );
	REFERENCE_CLASS( AChickenCoopDefenderHen );
	REFERENCE_CLASS( AChickenCoopDefenderEgg );
	REFERENCE_CLASS( AChickenCoopDefenderThief );
	REFERENCE_CLASS( AGolfGopherUnit );
	REFERENCE_CLASS( AUnitGolfProjectile );
	REFERENCE_CLASS( AUnitAesthetic );
	REFERENCE_CLASS( AUnitDummy );
	REFERENCE_CLASS( AStaticUnit );
	REFERENCE_CLASS( AManualPlayerControlComponent ); // control component of AChasingChicksAI
	REFERENCE_CLASS( AVeggiePatchThief );
	REFERENCE_CLASS( AUnitCarChaseCar );
	REFERENCE_CLASS( AVeggiePatchVeggie );
	REFERENCE_CLASS( APrecisionSquirt );
	REFERENCE_CLASS( APrecisionCoyote );
	REFERENCE_CLASS( APrecisionJersyCow );
	REFERENCE_CLASS( APrecisionRacoon );
	REFERENCE_CLASS( APrecisionScarecrow );
	REFERENCE_CLASS( APrecisionTargetItem );
	REFERENCE_CLASS( ACoinPickupCoinItem ); // "precisioncoin"
	REFERENCE_CLASS( ASpawnPoint );
	REFERENCE_CLASS( AMiniGameInstructionState );

	// The sim (also created by name, see AGameLoader)
	REFERENCE_CLASS( ASimState );
	REFERENCE_CLASS( APlayerSteer );
	REFERENCE_CLASS( ABallSteer );
	REFERENCE_CLASS( ABallSteer2 );
	REFERENCE_CLASS( AGopherMoundSteer );
	REFERENCE_CLASS( AAlienSteer );
	REFERENCE_CLASS( APlayerControlComponent );
	REFERENCE_CLASS( ASteerKinematics3D );
	REFERENCE_CLASS( ABikeSteer );            // the steer of the bike units (created by name)
	REFERENCE_CLASS( ARacingCarSteer );       // the steer of the Joy Ride cars (created by name)
	REFERENCE_CLASS( ABikeControlComponent ); // the control component of the player bike unit
	REFERENCE_CLASS( AFallingCowSteer );
	REFERENCE_CLASS( AProjectileItemSteer );  // created by name (unit steer type)
	REFERENCE_CLASS( ACarSteer );             // created by name (unit steer type)
	REFERENCE_CLASS( ACarHeliSteer );         // created by name (unit steer type)
	REFERENCE_CLASS( ARespawnComponentData ); // created by name (unit static data)
	REFERENCE_CLASS( APrimSphere );           // collision prims, unused in retail but registered
	REFERENCE_CLASS( APrimBox );
	REFERENCE_CLASS( APrimCapsule );
	REFERENCE_CLASS( APrimComposite );
	REFERENCE_CLASS( AGeomIntersectResult );
	REFERENCE_CLASS( AGeomRayIntersectResult );
	REFERENCE_CLASS( AMovableUnit );  // the bike racks, pick ups... (unit class created by name in AUnitType)
	REFERENCE_CLASS( AUnitPoolBall ); // Barn Pool units (unit classes created by name in AUnitType)
	REFERENCE_CLASS( AUnitPoolBfa );
	REFERENCE_CLASS( AUnitPoolCue );
	REFERENCE_CLASS( APoolBallSteer );     // the steer of the pool balls (ChangeSteer by TClass)
	REFERENCE_CLASS( ASSeqSteer );         // scripted sequence steer (created by name)
	REFERENCE_CLASS( ARumourRequirement ); // interaction requirement (created by name in AInteractionManager)

	// NPC behaviour: unit static data (created by name in AUnitType::LoadStaticData) and drive functions (created by name in ADriveFunction::CreateFromProperties)
	REFERENCE_CLASS( AAnimalAttributeData );
	REFERENCE_CLASS( AUnitBehaviourData );
	REFERENCE_CLASS( ANPCBehaviourComponentData );
	REFERENCE_CLASS( AUnitGameEventData );
	REFERENCE_CLASS( AUnitFacialComponentData );
	REFERENCE_CLASS( AUnitFacialComponent );
	REFERENCE_CLASS( AModelAttachComponentData );
	REFERENCE_CLASS( ASoundComponentData ); // created by name (unit static data)
	REFERENCE_CLASS( ASoundComponent );
	REFERENCE_CLASS( AModelAttachComponent );
	REFERENCE_CLASS( ADriveSystem );
	REFERENCE_CLASS( AAnimalAttributeDriveFunction );
	REFERENCE_CLASS( AHysteresisDriveFunction );
	REFERENCE_CLASS( ATestDriveFunction );

	// Inventories (the static data is created by name in AUnitType::LoadStaticData) and the interaction events of the quests
	REFERENCE_CLASS( AInventoryComponentData );
	REFERENCE_CLASS( AInventoryGroupComponentData );
	REFERENCE_CLASS( AInventoryComponent );
	REFERENCE_CLASS( AInventoryGroupComponent );
	REFERENCE_CLASS( AInventoryItem );
	REFERENCE_CLASS( ACarryableUnit ); // the items (unit class created by name in AUnitType)
	REFERENCE_CLASS( ACharacterUnitCarrable );
	REFERENCE_CLASS( AGiftComponentData );
	REFERENCE_CLASS( AUnitPlayerKnowledgeComponentData );
	REFERENCE_CLASS( AUnitPlayerKnowledgeComponent );
	REFERENCE_CLASS( ANPCMilkLauncherComponentData ); // Bud, Eddie and Igg squirt back
	REFERENCE_CLASS( ANPCMilkLauncherComponent );
	REFERENCE_CLASS( AFearRequirement );      // interaction requirement (created by name in AInteractionManager)
	REFERENCE_CLASS( ATopicTypeRequirement ); // interaction requirement (created by name in AInteractionManager)
	REFERENCE_CLASS( AInteractionCoyoteEvent );
	REFERENCE_CLASS( AInteractionFearEvent );
	REFERENCE_CLASS( AInteractionGiftEvent );
	REFERENCE_CLASS( AInteractionPlayerInsultEvent );
	REFERENCE_CLASS( AInteractionQuestEvent );
	REFERENCE_CLASS( AFunctionCollection );

	// Quests (the elements are created by name)
	REFERENCE_CLASS( AQuestManager );
	REFERENCE_CLASS( AQuest );
	REFERENCE_CLASS( AQuestInstance );
	REFERENCE_CLASS( ADummyQuestElement );
	REFERENCE_CLASS( AGameEventQuestElement );
	REFERENCE_CLASS( AGiftQuestElement );
	REFERENCE_CLASS( ALocateQuestElement );

	// Sim HUD and hints
	REFERENCE_CLASS( AHintManager );
	REFERENCE_CLASS( AHighScoreManager );
	REFERENCE_CLASS( AGopherShopInterface );
	REFERENCE_CLASS( AGopherShopGame );
	REFERENCE_CLASS( AHintDisplayState );
	REFERENCE_CLASS( AMapState );
	REFERENCE_CLASS( ASleepCycleChooseState ); // pushed by name by the bed
	REFERENCE_CLASS( ASleepCycleState );
	REFERENCE_CLASS( ATreasueHuntMicroGame ); // pushed by its script function "PushState"
	REFERENCE_CLASS( AShakeTreeMicroGame );   // added by its script function "PushState"
	REFERENCE_CLASS( AMilkCanMicroGame );     // added by name by the milk can
	REFERENCE_CLASS( ARecipeDisplayState );
	REFERENCE_CLASS( AInventoryState2 );
	REFERENCE_CLASS( AMobilePhoneUIState2 );
	REFERENCE_CLASS( AGamesPlayedDisplayState );
	REFERENCE_CLASS( ASimMiniMap );
	REFERENCE_CLASS( AItemCountHudElement );
	REFERENCE_CLASS( AGameTimeHudElement );
	REFERENCE_CLASS( ASimPlayerMainHudElement );
	REFERENCE_CLASS( AInteractionHudElement );
	REFERENCE_CLASS( AMenuNavHudElement );
	REFERENCE_CLASS( ATerrainSectionHudElement );
}
