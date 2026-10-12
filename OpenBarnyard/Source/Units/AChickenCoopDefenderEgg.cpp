#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChickenCoopDefenderEgg.h"
#include "ARandom.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "GUI/AGUI2MiniMap.h"
#include "MiniGame/AChickenCoopDefender.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00513cd0, CreateObject 00513f50, CreateObjectInPlace 00513fb0 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenCoopDefenderEgg );

TPSTRING8_DECLARE( distress_pleased );

// $Barnyard: FUNCTION 00513ce0
AChickenCoopDefenderEgg::AChickenCoopDefenderEgg()
{
	m_iMapIcon  = -1;
	m_pGame     = TNULL;
	m_eState    = STATE_IDLE;
	m_vVelocity = TVector4::VEC_ZERO;
	m_iValue    = 0;
}

// $Barnyard: FUNCTION 00513ff0
// $Barnyard: FUNCTION 00513d40
AChickenCoopDefenderEgg::~AChickenCoopDefenderEgg()
{
}

TBOOL AChickenCoopDefenderEgg::IsFollowedByCamera()
{
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
	AUnitProxy*            pTarget = pHelper->GetTarget();

	return ( pTarget ? pTarget->GetUnit() : TNULL ) == this;
}

void AChickenCoopDefenderEgg::MoveFromCarried( AChickenCoopDefenderGame::UnitList15& a_rTo )
{
	AChickenCoopDefenderGame::MoveUnit( m_pGame->oCarriedEggs, a_rTo, GetSelfProxy() );
}

// $Barnyard: FUNCTION 00513d60
void AChickenCoopDefenderEgg::SetMapIconVisible( TBOOL a_bVisible )
{
	AGUI2MiniMap* pMap = m_pGame->pMap;

	if ( m_iMapIcon < 0 )
	{
		m_iMapIcon = pMap->AddIcon( *m_pSteer->GetPosition(), *m_pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_DOT_WHITE, 0.1f );
		m_pGame->pMap->SetIconColour( m_iMapIcon, 0xFFFFC800 );
	}

	m_pGame->pMap->SetIconVisible( m_iMapIcon, a_bVisible );

	ASteer* pSteer = m_pSteer;
	m_pGame->pMap->SetIconTransform( m_iMapIcon, *pSteer->GetPosition(), *pSteer->GetDirection() );
}

// $Barnyard: FUNCTION 00513e20
void AChickenCoopDefenderEgg::Throw( const TVector4& a_rcVelocity )
{
	m_eState    = STATE_THROWN;
	m_vVelocity = a_rcVelocity;
}

// $Barnyard: FUNCTION 00514010
void AChickenCoopDefenderEgg::Drop( const TVector4& a_rcPosition )
{
	m_eState = STATE_BOUNCING;
	m_pSteer->SetPosition( a_rcPosition );

	// Bounces away in a random direction
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;
	TFLOAT   fAngle  = TFLOAT( rRandom.RandRaw() >> 1 ) * TMath::TWO_PI * 4.6566129e-10f;
	TFLOAT   fSpeed  = rRandom.GetFloat() + 2.0f;

	m_vVelocity.y = -3.0f;
	m_vVelocity.w = 1.0f;
	m_vVelocity.x = TMath::Sin( fAngle ) * fSpeed;
	m_vVelocity.z = TMath::Cos( fAngle ) * fSpeed;
}

// $Barnyard: FUNCTION 00514fa0
TBOOL AChickenCoopDefenderEgg::Think( TFLOAT a_fDeltaTime )
{
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );

	if ( m_eState == STATE_IDLE )
	{
		// The camera looked at the coop the egg landed in long enough
		if ( IsFollowedByCamera() )
		{
			m_fTimer -= a_fDeltaTime;

			if ( m_fTimer < 0.0f )
			{
				MoveFromCarried( m_pGame->oEggPool );
				m_pGame->ResetCamera();
			}
		}

		return AVisibleUnit::Think( a_fDeltaTime );
	}

	if ( m_eState == STATE_THROWN )
	{
		if ( TVector4::DistanceSq( *m_pSteer->GetPosition(), AChickenCoopDefenderGame::ms_aGoals[ 1 ] ) < 1.0f )
		{
			// In the coop
			m_eState = STATE_IDLE;
			SetVisible( TFALSE );
			m_pSteer->SetPosition( TVector4::VEC_ZERO );

			m_pGame->iNumEggsCarried--;
			m_pGame->iBonus += m_iValue;
			TSTATICCAST( AAnimatedUnit, m_pGame->pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( distress_pleased ) );

			if ( IsFollowedByCamera() )
			{
				// The camera keeps looking at the coop for a second
				SetVisible( TFALSE );
				m_pSteer->SetPosition( AChickenCoopDefenderGame::ms_aGoals[ 1 ] );
				m_pSteer->SetDirection( AChickenCoopDefenderGame::ms_aGoals[ 1 ] );
				m_fTimer = 1.0f;
			}
			else
			{
				MoveFromCarried( m_pGame->oEggPool );
			}

			return AVisibleUnit::Think( a_fDeltaTime );
		}

		// The camera following the egg looks down at it
		if ( IsFollowedByCamera() && IsFollowedByCamera() && m_vVelocity.MagnitudeSqXZ() > 0.0f )
		{
			TFLOAT fPitch = TMath::ATan2( ( m_vVelocity.y + m_vVelocity.y ) / m_vVelocity.MagnitudeXZ(), 1.0f ) * ( 180.0f / TMath::PI );

			if ( fPitch > 0.0f )
			{
				pHelper->SetTargetPitch( fPitch + 11.0f );
				pHelper->SetDefaultPitch( fPitch + 11.0f );
			}

			pHelper->SetDefaultDistance( ( TMath::Abs( m_vVelocity.y ) * 0.1f + 1.0f ) * 5.0f );
		}
	}
	else if ( m_eState != STATE_BOUNCING )
	{
		return AVisibleUnit::Think( a_fDeltaTime );
	}

	// Flies (y is down)
	TVector4 vNewPosition;
	vNewPosition.Multiply( m_vVelocity, a_fDeltaTime );

	const TVector4* pPosition = m_pSteer->GetPosition();
	vNewPosition.x += pPosition->x;
	vNewPosition.y += pPosition->y;
	vNewPosition.z += pPosition->z;
	vNewPosition.y = a_fDeltaTime * a_fDeltaTime * 4.9f + vNewPosition.y;
	m_vVelocity.y  = a_fDeltaTime * 9.8f + m_vVelocity.y;

	pPosition = m_pSteer->GetPosition();

	TVector4 vDirection;
	vDirection.Substract( vNewPosition, *pPosition );

	TFLOAT fDistance = vDirection.NormaliseMag();

	ACollisionModelRayHitList oHitList;

	if ( ATerrainInterface::GetSingleton()->CollideRay( *m_pSteer->GetPosition(), vDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG( 0x80000 ), TFALSE ) )
	{
		TVector4 vNormal = oHitList.aHits[ 0 ].GetNormal();

		if ( -0.70710677f <= vNormal.y )
		{
			// A wall: bounces off it
			vNormal.Multiply( TVector4::DotProduct3( vNormal, m_vVelocity ) * -2.0f );
			m_vVelocity.x += vNormal.x;
			m_vVelocity.y += vNormal.y;
			m_vVelocity.z += vNormal.z;
		}
		else
		{
			// The ground: lies there, loose
			m_eState    = STATE_IDLE;
			m_vVelocity = TVector4::VEC_ZERO;
			MoveFromCarried( m_pGame->oLooseEggs );
			SetMapIconVisible( TTRUE );
		}

		vDirection.Multiply( fDistance );

		pPosition = m_pSteer->GetPosition();
		vNewPosition.Set( vDirection.x + pPosition->x, vDirection.y + pPosition->y, vDirection.z + pPosition->z, pPosition->w );
	}

	m_pSteer->SetPosition( vNewPosition );
	return AVisibleUnit::Think( a_fDeltaTime );
}
