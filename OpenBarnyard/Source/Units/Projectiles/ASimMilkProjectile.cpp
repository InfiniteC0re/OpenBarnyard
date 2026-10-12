#include "pch.h"
#include "ASimMilkProjectile.h"
#include "AProjectileLauncher.h"
#include "GameInterface/ASimState.h"
#include "GameInterface/AGameStateController.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/ALadderBeansGame.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include "Units/AInteractionManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitBehaviour.h"
#include "Units/AUnitType.h"
#include "Units/AUnitPlayer.h"
#include "Units/ANPCMilkLauncherComponent.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( trigger_onmilkhit );
TPSTRING8_DECLARE( trigger_onmilkhit_barn );
TPSTRING8_DECLARE( player );
TPSTRING8_DECLARE( player_wet );
TPSTRING8_DECLARE( milkweed );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( bud );
TPSTRING8_DECLARE( eddie );
TPSTRING8_DECLARE( igg );

// NOTE: GetClass 0066c690, CreateObject 0066c740, CreateObjectInPlace 0066c7a0
TDEFINE_CLASS( ASimMilkProjectile );

// $Barnyard: FUNCTION 0066c6c0
ASimMilkProjectile::ASimMilkProjectile()
{
	m_oHitUnit = TNULL;
}

// $Barnyard: FUNCTION 0066c720
// $Barnyard: FUNCTION 0066c6a0
ASimMilkProjectile::~ASimMilkProjectile()
{
}

// $Barnyard: FUNCTION 0066c7e0
void ASimMilkProjectile::OnHit()
{
	// At night in the barn the owner of the launcher reacts to every hit
	if ( AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() )
		AInteractionManager::GetSingleton()->AssignInteractionForced( m_pLauncher->GetOwner()->GetUnit(), TPS8( trigger_onmilkhit_barn ) );

	if ( m_oHitUnit )
	{
		AUnit* pUnit = m_oHitUnit->GetUnit();

		// The milk doesn't hit the one who squirted it
		if ( pUnit != m_pLauncher->GetOwner()->GetUnit() )
		{
			TPString8 strUnitName = AUnitNameManager::GetSingleton()->GetUnitName( pUnit->GetId() );
			APlayerProfileManager::GetSingleton()->GetMainProfile()->AddSquirtedUnit( strUnitName );

			m_pLauncher->OnUnitHit( pUnit );

			const TPString8& rcTypeName = pUnit->GetType()->GetName();

			if ( rcTypeName == TPS8( player ) )
			{
				AUnitBehaviour* pBehaviour = TSTATICCAST( AUnitBehaviour, pUnit->FindComponentExact( &TGetClass( AUnitBehaviour ) ) );

				if ( pBehaviour->IsFSMFinished() )
					pBehaviour->SetCommand( TPS8( player_wet ), TNULL, TTRUE );
			}
			else if ( rcTypeName == TPS8( milkweed ) )
			{
				// Squirting a milkweed grows its ladder
				ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

				if ( pSimState )
				{
					ALadderBeansGame* pLadderBeansGame = pSimState->GetLadderBeansGame();

					if ( pLadderBeansGame )
						pLadderBeansGame->OnMilkweedSquirted( pLadderBeansGame->FindMilkweed( pUnit ) );
				}
			}
			else if ( pUnit->CanInteract() )
			{
				TBOOL bCanReact    = TTRUE;
				TBOOL bSquirtsBack = rcTypeName == TPS8( bud ) || rcTypeName == TPS8( eddie ) || rcTypeName == TPS8( igg );

				if ( AQuestManager::GetSingleton() )
				{
					// While the Sleepy Cows quest is assigned, nobody squirts back and nobody reacts on the bike
					if ( AQuestManager::GetSingleton()->FindAssignedQuest( TPS8D( "Sleepy Cows" ), 1 ) )
					{
						bSquirtsBack = TFALSE;

						if ( ABikeManager::GetSingleton() && ABikeManager::GetSingleton()->IsPlayerOnBike() )
							bCanReact = TFALSE;
					}
				}

				if ( bSquirtsBack )
				{
					if ( pUnit->IsA( &TGetClass( AAnimatedUnit ) ) )
					{
						AUnit* pOwner = ( m_pLauncher && m_pLauncher->GetOwner() ) ? m_pLauncher->GetOwner()->GetUnit() : TNULL;

						if ( pOwner && pOwner->IsA( &TGetClass( AUnitPlayer ) ) )
						{
							// Squirts back when the player is behind
							TVector4 vToPlayer;
							vToPlayer.Substract( *pOwner->GetSteer()->GetPosition(), *pUnit->GetSteer()->GetPosition() );
							vToPlayer.NormaliseXZ();

							TMatrix44 matTransform;
							TSTATICCAST( AVisibleUnit, pUnit )->GetModelInstance()->GetLocalTransform( matTransform );

							// NOTE: the second basis vector of the model is its facing direction
							if ( TVector4::DotProduct3( vToPlayer, matTransform.AsBasisVector4( BASISVECTOR_UP ) ) < -0.707f )
							{
								ANPCMilkLauncherComponent* pLauncherComponent = TSTATICCAST( ANPCMilkLauncherComponent, pUnit->FindComponentExact( &TGetClass( ANPCMilkLauncherComponent ) ) );

								if ( pLauncherComponent )
									pLauncherComponent->FireAtTarget( pOwner, -0.8f );
							}
						}
					}
				}
				else if ( bCanReact )
				{
					AInteractionManager::GetSingleton()->AssignInteractionForced( pUnit, TPS8( trigger_onmilkhit ) );
				}
			}
		}
	}

	AMilkProjectile::OnHit();
}
