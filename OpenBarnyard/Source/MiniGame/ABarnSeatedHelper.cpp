#include "pch.h"
#include "ABarnSeatedHelper.h"
#include "ABarnExploreMinigameState.h"
#include "ARandom.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitType.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Units/AUnitFacialComponent.h"
#include "Animation/AAnimatableObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// off_00773a50
static const TCHAR* s_aAnimNames[ ABarnSeatedHelper::ANIM_NUMOF ] = {
	"drink",
	"idle",
	"talk",
	"lookl",
	"lookr",
};

// $Barnyard: FUNCTION 004a68c0
ABarnSeatedHelper::ABarnSeatedHelper( AUnit* a_pPlayer )
{
	m_pPlayer = a_pPlayer;

	for ( TINT i = 0; i < ANIM_NUMOF; i++ )
		m_aAnims[ i ] = TPString8( s_aAnimNames[ i ] );
}

// $Barnyard: FUNCTION 004a6730
void ABarnSeatedHelper::Reset()
{
	m_vecGround.Clear();
	m_vecLoft.Clear();
}

// $Barnyard: FUNCTION 004a6690
void ABarnSeatedHelper::AddUnit( AUnit* a_pUnit )
{
	Seated oSeated;
	oSeated.iState  = Seated::STATE_START;
	oSeated.pUnit   = a_pUnit;
	oSeated.fTimer  = 0.0f;
	oSeated.uiFlags = 0;

	if ( a_pUnit->GetSteer()->GetPosition()->y <= 0.1f )
		m_vecGround.PushBack( oSeated );
	else
		m_vecLoft.PushBack( oSeated );
}

// $Barnyard: FUNCTION 004a69b0
void ABarnSeatedHelper::Update( TFLOAT a_fDeltaTime )
{
	// Only the ones of the player's floor
	T2Vector<Seated, MAX_NUM_SEATED>& rSeated = ( ABarnExploreMinigameState::ms_iFloor != ABarnExploreMinigameState::FLOOR_LOFT ) ? m_vecGround : m_vecLoft;

	for ( TINT i = 0; i < rSeated.Size(); i++ )
	{
		TFLOAT fDistanceSq = TVector4::DistanceSq( *m_pPlayer->GetSteer()->GetPosition(), *rSeated[ i ].pUnit->GetSteer()->GetPosition() );
		UpdateSeated( a_fDeltaTime, rSeated[ i ], fDistanceSq < 6.0f );
	}
}

// $Barnyard: FUNCTION 004a6750
void ABarnSeatedHelper::UpdateSeated( TFLOAT a_fDeltaTime, Seated& a_rSeated, TBOOL a_bNear )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	switch ( a_rSeated.iState )
	{
		case Seated::STATE_START:
			TSTATICCAST( AAnimatedUnit, a_rSeated.pUnit )->GetAnimatableObject()->PlayAnimation( m_aAnims[ ANIM_IDLE ] );
			a_rSeated.iState = Seated::STATE_IDLE;
			a_rSeated.fTimer = rRandom.GetFloatMinMax( 2.0f, 5.0f );
			break;
		case Seated::STATE_IDLE:
		{
			a_rSeated.fTimer -= a_fDeltaTime;
			AUnitFacialComponent* pFacial = TSTATICCAST( AUnitFacialComponent, a_rSeated.pUnit->FindComponent( &TGetClass( AUnitFacialComponent ) ) );

			if ( a_rSeated.fTimer >= 0.0f )
			{
				// Looks at the player once
				if ( a_bNear && !pFacial->IsLooking() && !( a_rSeated.uiFlags & Seated::FLAGS_LOOKED_AT_PLAYER ) )
				{
					a_rSeated.uiFlags |= Seated::FLAGS_LOOKED_AT_PLAYER;

					if ( rRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.3f )
					{
						pFacial->LookAtUnitHead( m_pPlayer, -1.0f );
						a_rSeated.fTimer = 5.0f;
					}
				}
			}
			else
			{
				// Drinks or talks
				const TPString8* pAnim;

				if ( rRandom.GetInt( 2 ) != 0 )
				{
					a_rSeated.iState = Seated::STATE_TALKING;
					pAnim            = &m_aAnims[ ANIM_TALK ];
				}
				else
				{
					a_rSeated.iState = Seated::STATE_DRINKING;
					pAnim            = &m_aAnims[ ANIM_DRINK ];
				}

				TSTATICCAST( AAnimatedUnit, a_rSeated.pUnit )->GetAnimatableObject()->PlayAnimation( *pAnim );
				pFacial->StopLook();
				a_rSeated.uiFlags &= ~Seated::FLAGS_LOOKED_AT_PLAYER;
			}
			break;
		}
		case Seated::STATE_DRINKING:
			if ( TSTATICCAST( AAnimatedUnit, a_rSeated.pUnit )->GetAnimatableObject()->IsAnimationFinished( m_aAnims[ ANIM_DRINK ] ) )
				a_rSeated.iState = Seated::STATE_START;
			break;
		case Seated::STATE_TALKING:
			if ( TSTATICCAST( AAnimatedUnit, a_rSeated.pUnit )->GetAnimatableObject()->IsAnimationFinished( m_aAnims[ ANIM_TALK ] ) )
				a_rSeated.iState = Seated::STATE_START;
			break;
	}
}
