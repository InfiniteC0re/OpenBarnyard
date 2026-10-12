#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChickenCoopDefenderThief.h"
#include "AChickenCoopDefenderHen.h"
#include "AChickenCoopDefenderEgg.h"
#include "AUnitPlayer.h"
#include "AUnitType.h"
#include "Animation/AAnimatableObjectType.h"
#include "ASimplePlayerControlComponent.h"
#include "ARandom.h"
#include "Animation/AModel.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AGUI2TargetCounter.h"
#include "MiniGame/AChickenCoopDefender.h"
#include "Render/ARenderer.h"
#include "Render/AShadowManager.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "Physics/ACollisionObject.h"
#include "Physics/AObjectHash.h"
#include "Physics/AObjectHashMain.h"

#include <Math/TMathInline.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0051b610, CreateObject 0051eb90, CreateObjectInPlace 0051ebf0 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenCoopDefenderThief );

TPSTRING8_DECLARE( chickencoop_raccoon );
TPSTRING8_DECLARE( chickencoop_coyote );
TPSTRING8_DECLARE( chickencoop_evilfreddy );
TPSTRING8_DECLARE( thiefspawn1 );
TPSTRING8_DECLARE( Goal0 );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( carrysmall );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( walk_lp );
TPSTRING8_DECLARE( run_lp );
TPSTRING8_DECLARE( scared );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( yuk_start );
TPSTRING8_DECLARE( yuk_end );
TPSTRING8_DECLARE( attack );
TPSTRING8_DECLARE( howl );
TPSTRING8_DECLARE( distress_idle );
TPSTRING8_DECLARE( carry_overlay );

static constexpr ASoundId SOUND_FREDDY_LAUGH = 524; // fleeing
static constexpr ASoundId SOUND_FREDDY_TALK  = 529; // seeking/escaping
static constexpr ASoundId SOUND_HANNA_ALARM  = 535; // a thief is in the coop

// Animation of a hen carried by each kind of thief (0x00722b08)
static const TPString8* s_apCarriedHenAnims[ AChickenCoopDefenderThief::KIND_NUMOF ] = {
	&TPS8( idle_lp ),
	&TPS8( idle_lp ),
	&TPS8( idle_lp ),
};

// Where the hidden thieves are
static const TVector3 s_vFarAway( 0.0f, 0.0f, 1000000.0f );

// The thieves run away from the player around this point (0x00773df8)
static const TVector4 s_vFleeCentre( 0.0f, 0.0f, 12.0f, 1.0f );

// Removes a_rcUnit from a_rList.
// NOTE: retail erases the found iterator without checking it (see AChickenCoopDefenderGame::MoveUnit)
template <class List>
static void EraseUnit( List& a_rList, const TRef<AUnitProxy>& a_rcUnit )
{
	auto it = a_rList.Find( a_rcUnit );

	if ( it != a_rList.End() )
		a_rList.Erase( it );
}

// $Barnyard: FUNCTION 0051c300
AChickenCoopDefenderThief::AChickenCoopDefenderThief()
{
	m_eKind        = KIND_RACCOON;
	m_fTimer       = 0.0f;
	m_pNetwork     = TNULL;
	m_pGame        = TNULL;
	m_pCarried     = TNULL;
	m_pTarget      = TNULL;
	m_fScare       = 0.0f;
	m_fSoundTimer  = 0.0f;
	m_iUnkdc       = 0;
	m_pUnke0       = TNULL;
	m_eState       = STATE_GONE;
	m_eSpeed       = SPEED_STAND;
	m_iSoundCue    = -1;
	m_iHitsLeft    = -1;
	m_iMapIcon     = -1;
	m_pStars       = TNULL;
	m_iHeadBone    = -1;
	m_iStarsEffect = -1;
}

// $Barnyard: FUNCTION 0051ec30
// $Barnyard: FUNCTION 0051c3b0
AChickenCoopDefenderThief::~AChickenCoopDefenderThief()
{
}

// $Barnyard: FUNCTION 0051c460
TBOOL AChickenCoopDefenderThief::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	AShadowManager::GetSingleton()->AddUnitShadow( this, 0.5f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, 0 );

	const TPString8& rcTypeName = GetType()->GetName();

	if ( rcTypeName == TPS8( chickencoop_raccoon ) )
	{
		// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
		AModelInstanceRef pModelInstance = GetAnimatableObject()->GetModelInstance();
		m_iHeadBone                      = pModelInstance->GetSkeleton()->GetBoneID( "c_head", 0 );

		m_iStarsEffect = AParticleEffectManager::FindEffect( "KO_stars" );
		m_eKind        = KIND_RACCOON;
	}
	else if ( rcTypeName == TPS8( chickencoop_coyote ) )
	{
		m_eKind = KIND_COYOTE;
	}
	else if ( rcTypeName == TPS8( chickencoop_evilfreddy ) )
	{
		m_eKind = KIND_FREDDY;
	}

	SetHashed( TTRUE );

	ACollisionObject* pCollisionObject = GetCollisionObject();

	if ( !pCollisionObject->IsCollisionObjectLinked() )
		pCollisionObject->Activate();

	GetCollisionObject()->SetType( ACollisionObject::TYPE_SIMPLE );
	GetAnimatableObject()->GetModelInstance()->SetShadowMode( AModelInstance::SHADOWMODE_DYNAMIC );

	return TTRUE;
}

// $Barnyard: FUNCTION 0051b620
void AChickenCoopDefenderThief::OnDestroy()
{
	AChickenCoopDefenderThiefUnk* pUnk = m_pUnke0;
	m_pGame                            = TNULL;
	m_iUnkdc                           = 0;

	if ( pUnk )
	{
		pUnk->Destroy();
		delete pUnk;
	}

	if ( m_pStars )
	{
		m_pStars->GetInstance()->Kill();
		m_pStars = TNULL;
	}

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 0051c5f0
void AChickenCoopDefenderThief::SetGame( AChickenCoopDefenderGame* a_pGame )
{
	m_pGame          = a_pGame;
	m_strCarryBone   = TPString8( a_pGame->pThiefData[ m_eKind ].szCarryBone );
	m_strCarriedBone = TPString8( m_pGame->pThiefData[ m_eKind ].szCarriedBone );
}

AUnit* AChickenCoopDefenderThief::GetPlayerUnit() const
{
	return m_pGame->oPlayer ? m_pGame->oPlayer->GetUnit() : TNULL;
}

// $Barnyard: FUNCTION 0051bc70
TINT AChickenCoopDefenderThief::GetMapIcon()
{
	if ( m_iMapIcon < 0 )
	{
		ASteer* pSteer = m_pSteer;
		m_iMapIcon     = m_pGame->pMap->AddIcon( *pSteer->GetPosition(), *pSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_ENEMY, 0.1f );
		m_pGame->pMap->SetIconVisible( m_iMapIcon, TFALSE );
	}

	return m_iMapIcon;
}

// $Barnyard: FUNCTION 0051b680
void AChickenCoopDefenderThief::SetSpeed( SPEED_ a_eSpeed )
{
	if ( a_eSpeed == SPEED_WALK )
		m_pSteer->SetDesiredSpeed( ( m_eState == STATE_ESCAPE ) ? GetData().fCarryWalkSpeed : GetData().fWalkSpeed );
	else if ( a_eSpeed == SPEED_RUN )
		m_pSteer->SetDesiredSpeed( ( m_eState == STATE_ESCAPE ) ? GetData().fCarryRunSpeed : GetData().fRunSpeed );

	m_eSpeed = a_eSpeed;
}

// $Barnyard: FUNCTION 0051b720
void AChickenCoopDefenderThief::UpdateSpeed( ASteer* a_pSteer )
{
	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, a_pSteer );

	if ( !pSteer->GetCurrentArea() )
		return;

	// Runs where the network is wide enough
	if ( pSteer->GetCurrentArea()->m_fPathHeuristicScale <= 0.5f || m_eSpeed == SPEED_RUN )
	{
		if ( pSteer->GetCurrentArea()->m_fPathHeuristicScale <= 0.5f && m_eSpeed != SPEED_WALK )
			SetSpeed( SPEED_WALK );
	}
	else
	{
		m_pSteer->SetDesiredSpeed( ( m_eState == STATE_ESCAPE ) ? GetData().fCarryRunSpeed : GetData().fRunSpeed );
		m_eSpeed = SPEED_RUN;
	}
}

// $Barnyard: FUNCTION 0051b7e0
void AChickenCoopDefenderThief::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	if ( m_eSpeed == SPEED_IDLE )
		return;

	AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
	TFLOAT                    fSpeed         = m_pSteer->GetMeasuredSpeed();
	const TPString8&          rcHenAnim      = *s_apCarriedHenAnims[ m_eKind ];
	TBOOL                     bCarryingHen   = m_pCarried && m_pCarried->IsA( &TGetClass( AChickenCoopDefenderHen ) );

	if ( fSpeed < 0.5f )
	{
		if ( !rAnimInterface.IsAnimPlaying( TPS8( idle_lp ), TFALSE ) )
		{
			rAnimInterface.PlayAnim( TPS8( idle_lp ), -1.0f, TFALSE );

			// The carried hen stops moving
			if ( bCarryingHen )
			{
				AToshiAnimationInterface& rHenInterface = TSTATICCAST( AAnimatedUnit, m_pCarried )->GetAnimatableObject()->GetToshiAnimInterface();
				rHenInterface.KillAllAnimations();
				rHenInterface.SetAnimSpeed( rHenInterface.PlayAnim( rcHenAnim, -1.0f, TFALSE ), 0.0f );
			}
		}

		return;
	}

	switch ( m_eSpeed )
	{
		case SPEED_STAND:
			if ( !rAnimInterface.IsAnimPlaying( TPS8( idle_lp ), TFALSE ) )
				rAnimInterface.PlayAnim( TPS8( idle_lp ), -1.0f, TFALSE );

			break;
		case SPEED_WALK:
		case SPEED_RUN:
		{
			const TPString8&    rcAnim   = ( m_eSpeed == SPEED_WALK ) ? TPS8( walk_lp ) : TPS8( run_lp );
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), rcAnim );

			if ( bCarryingHen )
			{
				// The carried hen plays its own animation
				AToshiAnimationInterface& rHenInterface = TSTATICCAST( AAnimatedUnit, m_pCarried )->GetAnimatableObject()->GetToshiAnimInterface();
				AToshiAnimationInterface::GetAnimationRef( rHenInterface.GetAnimRefs(), rcHenAnim );

				if ( !pAnimRef )
				{
					if ( rAnimInterface.IsAnimPlaying( rcAnim, TFALSE ) )
						break;

					pAnimRef = rAnimInterface.PlayAnim( rcAnim, -1.0f, TFALSE );
					rHenInterface.KillAllAnimations();
					rHenInterface.PlayAnim( rcHenAnim, -1.0f, TFALSE );

					if ( !pAnimRef )
						break;
				}

				rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / ( ( m_eSpeed == SPEED_WALK ) ? GetData().fCarryWalkAnimSpeed : GetData().fCarryRunAnimSpeed ) );
			}
			else
			{
				if ( !pAnimRef )
				{
					if ( rAnimInterface.IsAnimPlaying( rcAnim, TFALSE ) )
						break;

					pAnimRef = rAnimInterface.PlayAnim( rcAnim, -1.0f, TFALSE );

					if ( !pAnimRef )
						break;
				}

				TFLOAT fAnimSpeed;

				if ( m_eSpeed == SPEED_WALK )
					fAnimSpeed = ( m_eState == STATE_ESCAPE ) ? GetData().fCarryWalkAnimSpeed : GetData().fWalkAnimSpeed;
				else
					fAnimSpeed = ( m_eState == STATE_ESCAPE ) ? GetData().fCarryRunAnimSpeed : GetData().fRunAnimSpeed;

				rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / fAnimSpeed );
			}

			break;
		}
		case SPEED_SCARED:
		{
			AToshiAnimationRef* pAnimRef = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( scared ) );

			if ( !pAnimRef )
			{
				if ( rAnimInterface.IsAnimPlaying( TPS8( scared ), TFALSE ) )
					break;

				pAnimRef = rAnimInterface.PlayAnim( TPS8( scared ), -1.0f, TFALSE );

				if ( !pAnimRef )
					break;
			}

			rAnimInterface.SetAnimSpeed( pAnimRef, fSpeed / GetData().fScaredAnimSpeed );
			break;
		}
		default:
			break;
	}
}

// $Barnyard: FUNCTION 0051bd00
void AChickenCoopDefenderThief::UpdateSound( TFLOAT a_fDeltaTime )
{
	m_fSoundTimer -= a_fDeltaTime;

	if ( m_iSoundCue != -1 )
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();
		ASoundId       eSound        = pSoundManager->GetCueSoundId( m_iSoundCue );

		// Follows the thief while it plays
		if ( ( eSound == SOUND_FREDDY_LAUGH || eSound == SOUND_FREDDY_TALK ) && pSoundManager->IsStreamCueActive( m_iSoundCue ) )
		{
			pSoundManager->SetCuePositionToUnit( m_iSoundCue, this );
			return;
		}

		m_iSoundCue = -1;
	}

	// Freddy talks every now and then
	if ( m_fSoundTimer < 0.0f && m_eKind == KIND_FREDDY )
	{
		ASoundId eSound;

		switch ( m_eState )
		{
			case STATE_SEEK_LOOSE:
			case STATE_SEEK_COOP:
			case STATE_ESCAPE:
				eSound = SOUND_FREDDY_TALK;
				break;
			case STATE_FLEE:
				eSound = SOUND_FREDDY_LAUGH;
				break;
			default:
				return;
		}

		m_iSoundCue   = ASoundManager::GetSingleton()->PlayCueAtPosition( eSound, *m_pSteer->GetPosition() );
		m_fSoundTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 10.0f, 40.0f );
	}
}

// $Barnyard: FUNCTION 0051be20
TBOOL AChickenCoopDefenderThief::IsLineClear( const TVector4& a_rcFrom, const TVector4& a_rcTo )
{
	TVector4 vDirection;
	vDirection.Substract( a_rcTo, a_rcFrom );

	TFLOAT fLength   = vDirection.NormaliseMag();
	TFLOAT fDistance = fLength;

	if ( ATerrainInterface::GetSingleton()->CollideRay( a_rcFrom, vDirection, fDistance, TNULL, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TFALSE ) )
		return TFALSE;

	// The collision objects around the middle of the line
	TFLOAT   fRadius = fLength * 0.5f;
	TVector4 vHalf;
	vHalf.Multiply( vDirection, fRadius );

	TVector4 vCentre( vHalf.x + a_rcFrom.x, vHalf.y + a_rcFrom.y, vHalf.z + a_rcFrom.z, a_rcFrom.w );

	TINT                      iNumObjects;
	AObjectHash::QueryResult* pObjects = AObjectHashMain::GetSingleton()->FindObjects( vCentre, fRadius, &iNumObjects, TFALSE, 20 );

	ARayData oRay( a_rcFrom, a_rcTo );

	for ( TUINT i = 0; i < TUINT( iNumObjects ); i++ )
	{
		ACollisionObject* pObject = TSTATICCAST( ACollisionObject, pObjects[ i ].pObject );

		// Not the units
		if ( !pObject->GetOwnerUnit() )
		{
			ACollisionHit oHit;
			oHit.vNormal = TVector4::VEC_ZERO;

			if ( pObject->CollideRay( oRay, oHit ) )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0051c040
void AChickenCoopDefenderThief::UpdateScare( TFLOAT a_fDeltaTime )
{
	AUnit*   pPlayer          = m_pGame->oPlayer->GetUnit();
	TVector4 vPlayerPosition  = *pPlayer->GetSteer()->GetPosition();
	TVector4 vPlayerDirection = *m_pGame->oPlayer->GetUnit()->GetSteer()->GetDirection();
	TVector4 vPosition        = *m_pSteer->GetPosition();

	TVector4 vFromPlayer;
	vFromPlayer.Substract( vPosition, vPlayerPosition );

	TFLOAT fDistance = vFromPlayer.NormaliseMag() - 1.0f;
	TFLOAT fScare;

	if ( fDistance < 0.001f || fDistance < 10.0f )
	{
		if ( fDistance < 0.001f )
			fDistance = 0.001f;

		// Scared by the player looking at it, with nothing in between
		TFLOAT fDot = TVector4::DotProduct3( vPlayerDirection, vFromPlayer );

		TVector4 vFrom( vPlayerPosition.x, vPlayerPosition.y - 2.0f, vPlayerPosition.z, vPlayerPosition.w );
		TVector4 vTo( vPosition.x, vPosition.y - 0.5f, vPosition.z, vPosition.w );

		TFLOAT fFactor;

		if ( fDot >= 0.0f )
			fFactor = IsLineClear( vFrom, vTo ) ? 1.0f : 0.0f;
		else
			fFactor = fDot * 0.5f;

		fScare = fFactor * ( 10.0f - fDistance );

		if ( fScare <= m_fScare )
			fScare = m_fScare - a_fDeltaTime;
	}
	else
	{
		fScare = m_fScare - a_fDeltaTime;
	}

	m_fScare = fScare;

	if ( m_fScare < 0.0f )
		m_fScare = 0.0f;
}

// $Barnyard: FUNCTION 0051c740
TBOOL AChickenCoopDefenderThief::SetState( STATE_ a_eState )
{
	switch ( a_eState )
	{
		case STATE_SPAWN:
		{
			m_fTimer      = 2.0f;
			m_fSoundTimer = 0.0f;

			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
				ChangeSteer( &TGetClass( AInteriorSteer ) );

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->SetPosition( TVector4::VEC_ZERO );
			pSteer->ConnectToNetwork( m_pNetwork );
			pSteer->Command_SnapToGoal( TPS8( thiefspawn1 ) );

			GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) ) );

			SetVisible( TTRUE );
			SetMovement( TTRUE );

			m_eSpeed    = SPEED_STAND;
			m_iHitsLeft = GetData().iNumHits;

			m_pGame->pMap->SetIconScale( GetMapIcon(), 1.0f );
			m_pGame->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TTRUE );

			if ( m_pStars )
			{
				m_pStars->GetInstance()->Stop();
				m_pStars = TNULL;
			}

			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_SEEK_LOOSE:
		{
			if ( m_eKind == KIND_RACCOON )
			{
				// The closest loose egg
				if ( m_pGame->oLooseEggs.Size() == 0 )
					return TFALSE;

				TVector4 vPosition = *m_pSteer->GetPosition();
				TINT     iClosest  = 0;
				TFLOAT   fClosest  = TMath::MAXFLOAT;

				for ( TINT i = 0; i < m_pGame->oLooseEggs.Size(); i++ )
				{
					AUnit* pEgg = m_pGame->oLooseEggs[ i ] ? m_pGame->oLooseEggs[ i ]->GetUnit() : TNULL;

					TFLOAT fDistanceSq = TVector4::DistanceSqXZ( vPosition, *pEgg->GetSteer()->GetPosition() );

					if ( fDistanceSq < fClosest )
					{
						fClosest = fDistanceSq;
						iClosest = i;
					}
				}

				AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
				pSteer->Command_CancelCurrentCommand();

				m_pTarget = m_pGame->oLooseEggs[ iClosest ] ? m_pGame->oLooseEggs[ iClosest ]->GetUnit() : TNULL;
				pSteer->Command_SeekUnitPoint( m_pTarget );
				SetSpeed( SPEED_WALK );
			}
			else if ( m_eKind > KIND_RACCOON && m_eKind <= KIND_FREDDY )
			{
				// The closest loose hen
				if ( m_pGame->oLooseHens.Size() == 0 )
					return TFALSE;

				m_pTarget = TNULL;

				const TVector4* pPosition = m_pSteer->GetPosition();
				TFLOAT          fClosest  = TMath::MAXFLOAT;

				for ( auto it = m_pGame->oLooseHens.Begin(); it != m_pGame->oLooseHens.End(); it++ )
				{
					AUnit* pHen = it.Value() ? it.Value()->GetUnit() : TNULL;

					TFLOAT fDistanceSq = TVector4::DistanceSqXZ( *pPosition, *pHen->GetSteer()->GetPosition() );

					if ( fDistanceSq < fClosest )
					{
						fClosest  = fDistanceSq;
						m_pTarget = it.Value() ? it.Value()->GetUnit() : TNULL;
					}
				}

				ASteer* pSteer = m_pSteer;
				pSteer->Command_CancelCurrentCommand();
				pSteer->Command_SeekUnit( m_pTarget, 0.5f );
				SetSpeed( SPEED_WALK );
			}

			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_CHASE:
			if ( !m_pTarget )
				return SetState( STATE_SEEK_COOP );

			break;
		case STATE_PICKUP:
			if ( !GetAnimatableObject()->PlayAnimation( TPS8( pickup ) ) )
				return TFALSE;

			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;
			break;
		case STATE_SEEK_COOP:
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_SeekGoal( TPS8( Goal0 ) );
			SetSpeed( SPEED_WALK );

			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_IN_COOP:
		{
			m_fTimer = 5.0f;
			ChangeSteer( &TGetClass( AStationarySteer ) );

			// Hidden far away
			m_pSteer->CopyPosition( m_vCoopPosition );
			m_pSteer->SetPosition3( s_vFarAway );
			SetVisible( TFALSE );
			m_eSpeed = SPEED_IDLE;

			m_pGame->iNumThievesAtCoop++;
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TFALSE );
			m_eFlags &= ~FLAGS_MOVING;

			// Hanna sounds the alarm
			TSTATICCAST( AAnimatedUnit, m_pGame->pHanna )->GetAnimatableObject()->PlayAnimation( TPS8( distress_idle ) );
			ASoundManager::GetSingleton()->PlayCueAtPosition( SOUND_HANNA_ALARM, *m_pGame->pHanna->GetSteer()->GetPosition() );
			break;
		}
		case STATE_ESCAPE:
		{
			AInteriorSteer* pSteer;

			if ( m_eState == STATE_IN_COOP )
			{
				// Takes a hen/egg out of the coop
				if ( m_eKind == KIND_RACCOON )
				{
					if ( m_pGame->oEggPool.Size() == 0 )
						return TFALSE;

					m_pCarried = m_pGame->oEggPool.Back().Value() ? m_pGame->oEggPool.Back().Value()->GetUnit() : TNULL;
					m_pGame->oEggPool.PopBack();
					m_pGame->oCarriedEggs.PushBack( m_pCarried->GetSelfProxy() );
					m_pGame->iNumEggsCarried++;

					AttachUnitToBone( m_pCarried, m_strCarryBone, m_strCarriedBone );
					TSTATICCAST( AChickenCoopDefenderEgg, m_pCarried )->SetMapIconVisible( TFALSE );
				}
				else
				{
					if ( m_pGame->oHenPool.Size() == 0 )
						return TFALSE;

					m_pCarried = m_pGame->oHenPool.Back().Value()->GetUnit();
					m_pGame->oHenPool.PopBack();
					m_pGame->oCarriedHens.PushBack( m_pCarried->GetSelfProxy() );
					m_pGame->iNumHensCarried++;

					TSTATICCAST( AChickenCoopDefenderHen, m_pCarried )->SetState( AChickenCoopDefenderHen::STATE_HELD );
					AttachUnitToBone( m_pCarried, m_strCarryBone, m_strCarriedBone );
				}

				GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( carrysmall ) ) );
				GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );

				// Comes out of the coop
				m_pSteer->SetPosition( m_vCoopPosition );
				ChangeSteer( &TGetClass( AInteriorSteer ) );

				pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
				pSteer->ConnectToNetwork( m_pNetwork );
				pSteer->Command_SnapToGoal( TPS8( Goal0 ) );

				m_pGame->iNumThievesAtCoop--;
			}
			else
			{
				pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			}

			pSteer->Command_SeekGoal( TPS8( thiefspawn1 ) );
			SetVisible( TTRUE );
			SetSpeed( SPEED_WALK );
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TTRUE );

			m_eFlags |= FLAGS_MOVING;
			break;
		}
		case STATE_FLEE:
		{
			AInteriorSteer* pSteer;

			if ( m_eState == STATE_IN_COOP )
			{
				// Comes out of the coop
				m_pSteer->SetPosition( m_vCoopPosition );
				ChangeSteer( &TGetClass( AInteriorSteer ) );

				pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
				pSteer->ConnectToNetwork( m_pNetwork );
				pSteer->Command_SnapToGoal( TPS8( Goal0 ) );

				m_pGame->iNumThievesAtCoop--;
			}
			else
			{
				// Not while carried or thrown by the player
				if ( m_eState > STATE_MILKED_DROP && m_eState <= STATE_THROWN )
					return TFALSE;

				pSteer = TSTATICCAST( AInteriorSteer, m_pSteer );
			}

			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_SeekGoal( TPS8( thiefspawn1 ) );

			if ( m_eKind == KIND_COYOTE )
			{
				m_eSpeed = SPEED_SCARED;
			}
			else
			{
				m_pSteer->SetDesiredSpeed( ( m_eState == STATE_ESCAPE ) ? GetData().fCarryRunSpeed : GetData().fRunSpeed );
				m_eSpeed = SPEED_RUN;
			}

			pSteer->SetDesiredSpeed( GetData().fFleeSpeed );
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TTRUE );

			m_eFlags &= ~FLAGS_MOVING;
			break;
		}
		case STATE_GONE:
		{
			if ( !m_pSteer->GetClass()->IsA( &TGetClass( AStationarySteer ) ) )
				ChangeSteer( &TGetClass( AStationarySteer ) );

			// Hidden far away
			m_pSteer->SetPosition3( s_vFarAway );
			SetVisible( TFALSE );
			m_eSpeed = SPEED_IDLE;
			SetMovement( TFALSE );

			if ( m_eState == STATE_ESCAPE )
			{
				// Got away with a hen/egg
				DetachUnit( m_pCarried, TTRUE, TTRUE );

				if ( m_pCarried->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
				{
					TSTATICCAST( AChickenCoopDefenderHen, m_pCarried )->SetState( AChickenCoopDefenderHen::STATE_HOME );
					m_pGame->iNumHensLost++;
					m_pGame->iNumHensCarried--;
				}
				else
				{
					TSTATICCAST( AAnimatedUnit, m_pCarried )->SetVisible( TFALSE );
					AChickenCoopDefenderGame::MoveUnit( m_pGame->oCarriedEggs, m_pGame->oEggPool, m_pCarried->GetSelfProxy() );
					m_pGame->iNumEggsLost++;
					m_pGame->iNumEggsCarried--;
				}

				m_pGame->iNumLeft--;

				if ( m_pGame->iNumLeft < 0 )
					m_pGame->iNumLeft = 0;

				m_pGame->pCounter->SetCount( m_pGame->iNumLeft, TFALSE );
				m_pCarried = TNULL;
			}

			if ( m_eState != STATE_GONE )
			{
				// Back in the free list of its kind
				TRef<AUnitProxy> oSelf = GetSelfProxy();

				EraseUnit( m_pGame->oThieves, oSelf );
				m_pGame->iNumThieves--;

				switch ( m_eKind )
				{
					case KIND_RACCOON:
						EraseUnit( m_pGame->oRaccoons, oSelf );
						m_pGame->oFreeRaccoons.PushBack( oSelf );
						break;
					case KIND_COYOTE:
						EraseUnit( m_pGame->oCoyotes, oSelf );
						m_pGame->oFreeCoyotes.PushBack( oSelf );
						break;
					case KIND_FREDDY:
						EraseUnit( m_pGame->oFreddies, oSelf );
						m_pGame->oFreeFreddies.PushBack( oSelf );
						break;
					default:
						break;
				}
			}

			m_pGame->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TFALSE );

			m_eFlags &= ~FLAGS_MOVING;
			break;
		}
		case STATE_MILKED:
		{
			if ( m_pCarried )
				return TFALSE;

			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;

			if ( m_eKind != KIND_RACCOON )
			{
				GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
			}
			else
			{
				// Stars around the head of the raccoon
				if ( !m_pStars )
				{
					m_pStars = AParticleEffectManager::GetSingleton()->PlayEffect( m_iStarsEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

					if ( m_pStars )
						m_pStars->AttachToBone( GetAnimatableObject()->GetModelInstance(), m_iHeadBone, TTRUE, TFALSE );
				}

				GetAnimatableObject()->PlayAnimation( TPS8( yuk_start ) );
			}

			if ( m_iHitsLeft > 0 )
				m_iHitsLeft--;

			m_pGame->pMap->SetIconAlpha( GetMapIcon(), 1.0f );
			m_pGame->pMap->SetIconScale( GetMapIcon(), 1.0f );

			m_eFlags &= ~FLAGS_MOVING;
			m_fTimer = 5.0f;
			break;
		}
		case STATE_MILKED_DROP:
		{
			if ( !m_pCarried )
				return TFALSE;

			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;

			// Drops the hen/egg
			DetachUnit( m_pCarried, TTRUE, TTRUE );
			m_pCarried->GetSteer()->SetPosition( *m_pSteer->GetPosition() );

			if ( m_pCarried->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
			{
				AChickenCoopDefenderGame::MoveUnit( m_pGame->oCarriedHens, m_pGame->oLooseHens, m_pCarried->GetSelfProxy() );
				TSTATICCAST( AChickenCoopDefenderHen, m_pCarried )->SetState( AChickenCoopDefenderHen::STATE_FLEE );
			}
			else
			{
				// NOTE: retail looks the egg up in the carried eggs and doesn't use the result
				m_pGame->oCarriedEggs.Find( m_pCarried->GetSelfProxy() );

				TVector4 vPosition = *m_pSteer->GetPosition();
				vPosition.y -= 1.0f;
				TSTATICCAST( AChickenCoopDefenderEgg, m_pCarried )->Drop( vPosition );
			}

			m_pCarried = TNULL;

			GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( default ) ) );
			GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );
			break;
		}
		case STATE_PICKED_UP:
			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;
			m_eFlags &= ~FLAGS_MOVING;

			if ( m_pStars )
			{
				m_pStars->GetInstance()->StopLooping();
				m_pStars = TNULL;
			}

			break;
		case STATE_THROWN:
			ChangeSteer( &TGetClass( AStationarySteer ) );
			m_pGame->pMap->SetIconVisible( GetMapIcon(), TFALSE );
			m_eFlags &= ~FLAGS_MOVING;
			break;
		case STATE_SCARED:
			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_SCARED;
			m_fTimer = 5.0f;
			m_eFlags |= FLAGS_MOVING;
			break;
		case STATE_STALK:
		{
			if ( m_eKind != KIND_COYOTE || m_pCarried )
				return TFALSE;

			ASteer* pSteer = m_pSteer;
			pSteer->Command_CancelCurrentCommand();
			pSteer->Command_SeekUnit( GetPlayerUnit(), 1.0f );
			SetSpeed( SPEED_WALK );

			GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
			break;
		}
		case STATE_ATTACK:
			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;
			GetAnimatableObject()->PlayAnimation( TPS8( attack ) );
			break;
		case STATE_IDLE:
			m_pSteer->Command_CancelCurrentCommand();
			m_eSpeed = SPEED_IDLE;
			m_eFlags |= FLAGS_MOVING;
			break;
		default:
			break;
	}

	m_eState = a_eState;
	return TTRUE;
}

// $Barnyard: FUNCTION 0051e540
void AChickenCoopDefenderThief::OnMilked()
{
	if ( m_eState == STATE_MILKED || m_eState == STATE_MILKED_DROP )
		return;

	if ( m_eState != STATE_FLEE )
	{
		if ( m_eKind != KIND_RACCOON )
			m_pGame->iNumHits++;
		else
			m_pGame->iNumRaccoonHits++;
	}

	SetState( m_pCarried ? STATE_MILKED_DROP : STATE_MILKED );
}

void AChickenCoopDefenderThief::SeekSomething()
{
	if ( !SetState( STATE_SEEK_LOOSE ) )
		SetState( STATE_SEEK_COOP );
}

TBOOL AChickenCoopDefenderThief::IsTargetLoose()
{
	if ( m_eKind == KIND_RACCOON )
		return m_pGame->oLooseEggs.Find( m_pTarget->GetSelfProxy() ) != m_pGame->oLooseEggs.End();

	return m_pGame->oLooseHens.Find( m_pTarget->GetSelfProxy() ) != m_pGame->oLooseHens.End();
}

// $Barnyard: FUNCTION 0051d8e0
void AChickenCoopDefenderThief::UpdateSpawn( TFLOAT a_fDeltaTime )
{
	if ( m_fTimer < 0.0f )
		SeekSomething();
}

// $Barnyard: FUNCTION 0051d910
void AChickenCoopDefenderThief::UpdateSeekLoose( TFLOAT a_fDeltaTime )
{
	// The player is too close
	if ( m_fScare > 7.0f )
	{
		if ( m_eKind == KIND_RACCOON )
		{
			SetState( STATE_SCARED );
			return;
		}

		if ( m_eKind == KIND_COYOTE )
		{
			SetState( STATE_STALK );
			return;
		}
	}

	ASteer* pSteer = m_pSteer;
	UpdateSpeed( pSteer );

	// Someone else got it
	if ( m_eKind >= KIND_RACCOON && m_eKind <= KIND_FREDDY && !IsTargetLoose() )
	{
		m_pTarget = TNULL;
		SeekSomething();
		return;
	}

	if ( pSteer->IsCommandFinished() )
	{
		TVector4        vTarget   = *m_pTarget->GetSteer()->GetPosition();
		const TVector4* pPosition = pSteer->GetPosition();

		TFLOAT fDistanceZ = vTarget.z - pPosition->z;
		TFLOAT fDistanceX = vTarget.x - pPosition->x;

		if ( fDistanceZ * fDistanceZ + fDistanceX * fDistanceX >= 1.0f )
			SetState( STATE_CHASE );
		else
			SetState( STATE_PICKUP );
	}
}

// $Barnyard: FUNCTION 0051daf0
void AChickenCoopDefenderThief::UpdateChase( TFLOAT a_fDeltaTime )
{
	// The player is too close
	if ( m_fScare > 7.0f )
	{
		if ( m_eKind == KIND_RACCOON )
		{
			SetState( STATE_SCARED );
			return;
		}

		if ( m_eKind == KIND_COYOTE )
		{
			SetState( STATE_STALK );
			return;
		}
	}

	// Someone else got it
	if ( m_eKind >= KIND_RACCOON && m_eKind <= KIND_FREDDY && !IsTargetLoose() )
	{
		m_pTarget = TNULL;
		SeekSomething();
		return;
	}

	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pSteer );

	TVector4 vToTarget;
	vToTarget.Substract( *m_pTarget->GetSteer()->GetPosition(), *pSteer->GetPosition() );
	vToTarget.y = 0.0f;

	if ( vToTarget.NormaliseMag() < 1.5f )
	{
		SetState( STATE_PICKUP );
		return;
	}

	vToTarget.Multiply( GetData().fRunSpeed );
	pSteer->SetDesiredVelocity( vToTarget );
}

// $Barnyard: FUNCTION 0051dd10
void AChickenCoopDefenderThief::UpdatePickup( TFLOAT a_fDeltaTime )
{
	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( pickup ) ) && m_pCarried )
	{
		GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( carrysmall ) ) );
		GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
		SetState( STATE_ESCAPE );
	}
}

// $Barnyard: FUNCTION 0051de00
void AChickenCoopDefenderThief::UpdateSeekCoop( TFLOAT a_fDeltaTime )
{
	// The player is too close
	if ( m_fScare > 7.0f )
	{
		if ( m_eKind == KIND_RACCOON )
		{
			SetState( STATE_SCARED );
			return;
		}

		if ( m_eKind == KIND_COYOTE )
		{
			SetState( STATE_STALK );
			return;
		}
	}

	ASteer* pSteer = m_pSteer;
	UpdateSpeed( pSteer );

	if ( pSteer->IsCommandFinished() )
		SetState( STATE_IN_COOP );
}

// $Barnyard: FUNCTION 0051de70
void AChickenCoopDefenderThief::UpdateInCoop( TFLOAT a_fDeltaTime )
{
	if ( m_fTimer < 0.0f )
		SetState( STATE_ESCAPE );
}

// $Barnyard: FUNCTION 0051dea0
void AChickenCoopDefenderThief::UpdateEscape( TFLOAT a_fDeltaTime )
{
	if ( m_fScare > 7.0f && m_eKind == KIND_RACCOON )
	{
		SetState( STATE_SCARED );
		return;
	}

	ASteer* pSteer = m_pSteer;
	UpdateSpeed( pSteer );

	// The icon of a thief carrying a hen/egg pulses
	m_pGame->pMap->SetIconAlpha( GetMapIcon(), ( TMath::Sin( TMath::TWO_PI * m_pGame->fTime ) + 1.0f ) * 0.5f );
	m_pGame->pMap->SetIconScale( GetMapIcon(), TMath::Sin( TMath::TWO_PI * m_pGame->fTime ) * 0.5f + 1.0f );

	if ( pSteer->IsCommandFinished() )
		SetState( STATE_GONE );
}

// $Barnyard: FUNCTION 0051df70
void AChickenCoopDefenderThief::UpdateFlee( TFLOAT a_fDeltaTime )
{
	if ( m_pSteer->IsCommandFinished() )
		SetState( STATE_GONE );
}

// $Barnyard: FUNCTION 0051e310
void AChickenCoopDefenderThief::UpdateMilked( TFLOAT a_fDeltaTime )
{
	if ( m_eKind != KIND_RACCOON )
	{
		if ( !GetAnimatableObject()->IsAnimationFinished( TPS8( yuk ) ) )
			return;
	}
	else
	{
		if ( !GetAnimatableObject()->IsAnimationFinished( TPS8( yuk_start ) ) || m_fTimer >= 0.0f )
			return;

		// Stunned long enough
		if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
		{
			if ( !GetAnimatableObject()->IsAnimPlaying( TPS8( yuk_end ), 0.0f ) )
			{
				if ( m_pStars )
				{
					m_pStars->GetInstance()->Stop();
					m_pStars = TNULL;
				}

				GetAnimatableObject()->PlayAnimation( TPS8( yuk_end ) );
			}

			return;
		}
	}

	if ( m_iHitsLeft != 0 )
		SetState( STATE_SCARED );
	else
		SetState( STATE_FLEE );
}

// $Barnyard: FUNCTION 0051e3f0
void AChickenCoopDefenderThief::UpdateMilkedDrop( TFLOAT a_fDeltaTime )
{
	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( yuk ) ) )
	{
		if ( m_iHitsLeft != 0 )
			SetState( STATE_SCARED );
		else
			SetState( STATE_FLEE );
	}
}

void AChickenCoopDefenderThief::UpdateThrown( TFLOAT a_fDeltaTime )
{
	// Flies (y is down)
	TVector4 vPosition;
	vPosition.Multiply( m_vVelocity, a_fDeltaTime );
	vPosition.AsVector3().Add( m_pSteer->GetPosition()->AsVector3() );

	m_vVelocity.y = a_fDeltaTime * 9.8f + m_vVelocity.y;
	m_pSteer->SetPosition( vPosition );

	// Gone once out of sight
	ASteer* pPlayerSteer = GetPlayerUnit()->GetSteer();
	TFLOAT  fFar         = ARenderer::GetSingleton()->GetFar();

	if ( TVector4::DistanceSq( *pPlayerSteer->GetPosition(), vPosition ) > fFar * fFar )
		SetState( STATE_GONE );

	// The icon gets bigger the higher it flies
	TFLOAT fScale = ( pPlayerSteer->GetPosition()->y - vPosition.y ) * 0.05f + 1.0f;
	m_pGame->pMap->SetIconScale( GetMapIcon(), fScale );
}

// $Barnyard: FUNCTION 0051df90
void AChickenCoopDefenderThief::UpdateScared( TFLOAT a_fDeltaTime )
{
	ACommonSteer* pSteer = TSTATICCAST( ACommonSteer, m_pSteer );

	// Wobbles while running away
	TVector4 vDirection;
	pSteer->CopyDirection( vDirection );
	vDirection.AsVector3().RotateY( TMath::Sin( TMath::TWO_PI * *m_pGame->pElapsedTime ) * TMath::PI );

	const TVector4* pPlayerPosition = GetPlayerUnit()->GetSteer()->GetPosition();

	TVector4 vFromPlayer;
	vFromPlayer.Substract( *pSteer->GetPosition(), *pPlayerPosition );
	vFromPlayer.y = 0.0f;

	TFLOAT fPlayerDistanceSq = vFromPlayer.MagnitudeSqXZ();

	TVector4 vFromCentre;
	vFromCentre.Substract( *pSteer->GetPosition(), s_vFleeCentre );
	vFromCentre.y = 0.0f;

	TFLOAT fCentreDistanceSq = vFromCentre.MagnitudeSqXZ();

	if ( fPlayerDistanceSq < 100.0f )
	{
		// Away from the player (and from the centre)
		vFromPlayer.Divide( TMath::Sqrt( fPlayerDistanceSq ) );

		if ( fCentreDistanceSq >= 400.0f )
		{
			vDirection = vFromPlayer;
		}
		else
		{
			vFromCentre.Divide( TMath::Sqrt( fCentreDistanceSq ) );
			vDirection.Set( vFromCentre.x + vFromPlayer.x, vFromCentre.y + vFromPlayer.y, vFromCentre.z + vFromPlayer.z, vFromPlayer.w );
			vDirection.Normalise();
		}
	}
	else if ( fCentreDistanceSq >= 400.0f )
	{
		// Sometimes turns around
		if ( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 1.0f ) > 0.8f )
		{
			vDirection.x = -vDirection.x;
			vDirection.y = -vDirection.y;
			vDirection.z = -vDirection.z;
		}
	}
	else
	{
		// Away from the centre
		vFromCentre.Divide( TMath::Sqrt( fCentreDistanceSq ) );
		vDirection = vFromCentre;
	}

	vDirection.Multiply( GetData().fFleeSpeed );
	pSteer->SetDesiredVelocity( vDirection );

	AToshiAnimationInterface& rAnimInterface = GetAnimatableObject()->GetToshiAnimInterface();
	AToshiAnimationRef*       pAnimRef       = AToshiAnimationInterface::GetAnimationRef( rAnimInterface.GetAnimRefs(), TPS8( scared ) );

	if ( pAnimRef )
		rAnimInterface.SetAnimSpeed( pAnimRef, GetData().fFleeSpeed / GetData().fScaredAnimSpeed );

	// The icon of a thief carrying a hen/egg pulses
	if ( m_pCarried )
	{
		m_pGame->pMap->SetIconAlpha( GetMapIcon(), ( TMath::Sin( TMath::TWO_PI * m_pGame->fTime ) + 1.0f ) * 0.5f );
		m_pGame->pMap->SetIconScale( GetMapIcon(), TMath::Sin( TMath::TWO_PI * m_pGame->fTime ) * 0.5f + 1.0f );
	}

	if ( m_fTimer < 0.0f && m_fScare < 2.0f )
	{
		if ( m_pCarried )
			SetState( STATE_ESCAPE );
		else
			SeekSomething();
	}
}

// $Barnyard: FUNCTION 0051e430
void AChickenCoopDefenderThief::UpdateStalk( TFLOAT a_fDeltaTime )
{
	if ( m_pSteer->IsCommandFinished() )
		SetState( STATE_ATTACK );

	if ( m_fScare < 4.0f )
		SeekSomething();
}

// $Barnyard: FUNCTION 0051e480
void AChickenCoopDefenderThief::UpdateAttack( TFLOAT a_fDeltaTime )
{
	// Faces the player
	TVector4 vToPlayer = *GetPlayerUnit()->GetSteer()->GetPosition();
	vToPlayer.Substract( *m_pSteer->GetPosition() );
	AChickenCoopDefenderNormaliseXZ( vToPlayer );
	m_pSteer->SetDirection( vToPlayer );

	if ( GetAnimatableObject()->IsAnimationFinished( TPS8( attack ) ) )
		SeekSomething();
}

// $Barnyard: FUNCTION 0051e590
void AChickenCoopDefenderThief::OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent )
{
	if ( a_pEvent->eAction != AAnimationBreakpointUnitAction::ACTION_PICKUP )
		return;

	if ( m_eState == STATE_PICKUP )
	{
		// Picks the hen/egg up, unless someone was faster
		TBOOL bTaken = TFALSE;

		if ( m_eKind == KIND_RACCOON )
		{
			if ( AChickenCoopDefenderGame::MoveUnit( m_pGame->oLooseEggs, m_pGame->oCarriedEggs, m_pTarget->GetSelfProxy() ) )
			{
				TSTATICCAST( AChickenCoopDefenderEgg, m_pTarget )->SetMapIconVisible( TFALSE );
				bTaken = TTRUE;
			}
		}
		else if ( m_eKind > KIND_RACCOON && m_eKind <= KIND_FREDDY )
		{
			if ( AChickenCoopDefenderGame::MoveUnit( m_pGame->oLooseHens, m_pGame->oCarriedHens, m_pTarget->GetSelfProxy() ) )
			{
				TSTATICCAST( AChickenCoopDefenderHen, m_pTarget )->SetState( AChickenCoopDefenderHen::STATE_HELD );
				bTaken = TTRUE;
			}
		}
		else
		{
			bTaken = TTRUE;
		}

		if ( bTaken )
		{
			m_pCarried = m_pTarget;
			m_pTarget  = TNULL;
			AttachUnitToBone( m_pCarried, m_strCarryBone, m_strCarriedBone );
			GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( carrysmall ) ) );
			return;
		}

		m_pTarget = TNULL;
		SeekSomething();
		return;
	}

	if ( m_eState != STATE_ATTACK || m_eKind != KIND_COYOTE )
		return;

	// The coyote bites the player
	AUnitPlayer* pPlayer = TSTATICCAST( AUnitPlayer, m_pGame->oPlayer->GetUnit() );

	if ( !pPlayer )
		return;

	APlayerSteer* pPlayerSteer = TSTATICCAST( APlayerSteer, pPlayer->GetSteer() );

	TVector4 vToPlayer;
	vToPlayer.Substract( *pPlayerSteer->GetPosition(), *m_pSteer->GetPosition() );

	if ( vToPlayer.NormaliseMag() >= 2.0f || m_pGame->fHitCooldown >= 0.0f || pPlayerSteer->GetState() != APlayerSteer::STATE_WALKING )
		return;

	if ( TVector4::DotProduct3( vToPlayer, *m_pSteer->GetDirection() ) <= 0.0f || m_pGame->iLives <= 0 )
		return;

	m_pGame->iLives--;
	m_pGame->pLifeHud->AddLives( -1 );

	m_fTimer              = 0.0f;
	m_pGame->fHitCooldown = 1.0f;

	switch ( m_pGame->ePlayerState )
	{
		case AChickenCoopDefender::PLAYERSTATE_THROW_HEN:
			// Hit after the throw
			m_pGame->bPlayerHit = TTRUE;
			return;
		case AChickenCoopDefender::PLAYERSTATE_CARRY_HEN:
		case AChickenCoopDefender::PLAYERSTATE_AIM_HEN:
		{
			// Drops the hen
			AUnit* pItem = m_pGame->pItem;

			if ( pItem && pItem->IsA( &TGetClass( AChickenCoopDefenderHen ) ) )
			{
				pPlayer->DetachUnit( pItem, TTRUE, TTRUE );
				TSTATICCAST( AChickenCoopDefenderHen, pItem )->SetState( AChickenCoopDefenderHen::STATE_FLEE );
				AChickenCoopDefenderGame::MoveUnit( m_pGame->oCarriedHens, m_pGame->oLooseHens, pItem->GetSelfProxy() );
				pItem->GetSteer()->SetPosition( *pPlayerSteer->GetPosition() );
				m_pGame->pItem = TNULL;

				AToshiAnimationInterface& rInterface = pPlayer->GetAnimatableObject()->GetToshiAnimInterface();

				if ( rInterface.IsOverlayAnimPlaying( TPS8( carry_overlay ) ) )
					rInterface.StopAnim( TPS8( carry_overlay ) );
			}

			break;
		}
		default:
			break;
	}

	// The player is stunned
	m_pGame->ePlayerState = AChickenCoopDefender::PLAYERSTATE_HIT;
	pPlayer->GetAnimatableObject()->KillAllAnimations();
	pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( yuk ) );

	ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

	if ( pPlayer->GetActiveLauncher() != pPlayer->GetLauncher( 0 ) )
		pPlayer->SelectLauncher( TFALSE );

	pControl->SetMovementEnabled( TFALSE );
	pControl->SetLauncherInput( TFALSE );
	pControl->SetSecondaryLauncherInput( TFALSE );

	m_pGame->bPlayerHit = TFALSE;
}

// $Barnyard: FUNCTION 0051ec50
TBOOL AChickenCoopDefenderThief::Think( TFLOAT a_fDeltaTime )
{
	ASteer* pSteer = m_pSteer;
	m_pGame->pMap->SetIconTransform( GetMapIcon(), *pSteer->GetPosition(), *pSteer->GetDirection() );

	TINT  ePhase  = m_pGame->ePhase;
	TBOOL bUpdate = TFALSE;

	if ( ePhase != AChickenCoopDefender::PHASE_FOLLOW )
	{
		if ( ePhase != AChickenCoopDefender::PHASE_ENDED || !m_pSteer->GetClass()->IsA( &TGetClass( AInteriorSteer ) ) )
		{
			bUpdate = TTRUE;
		}
		else if ( m_eKind == KIND_COYOTE && m_eState == STATE_ATTACK && m_pGame->iLives == 0 )
		{
			// The coyote howls over the defeated player
			if ( GetAnimatableObject()->IsAnimationFinished( TPS8( howl ) ) )
			{
				if ( m_fTimer > 0.0f )
				{
					m_fTimer -= a_fDeltaTime;
				}
				else
				{
					GetAnimatableObject()->PlayAnimation( TPS8( howl ) );
					m_fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 1.0f ) + 1.0f;
				}
			}

			bUpdate = TTRUE;
		}
		else if ( m_pGame->iNumLeft > 0 && m_pGame->iLives != 0 )
		{
			// Won: the thieves keep going
			bUpdate = TTRUE;
		}
	}

	if ( !bUpdate )
	{
		// Stops while the camera follows a throw, or once the game is lost
		m_pSteer->Command_CancelCurrentCommand();
		GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
		return AVisibleUnit::Think( a_fDeltaTime );
	}

	m_fTimer -= a_fDeltaTime;

	switch ( m_eState )
	{
		case STATE_SPAWN:
			UpdateSpawn( a_fDeltaTime );
			break;
		case STATE_SEEK_LOOSE:
			UpdateSeekLoose( a_fDeltaTime );
			break;
		case STATE_CHASE:
			UpdateChase( a_fDeltaTime );
			break;
		case STATE_PICKUP:
			UpdatePickup( a_fDeltaTime );
			break;
		case STATE_SEEK_COOP:
			UpdateSeekCoop( a_fDeltaTime );
			break;
		case STATE_IN_COOP:
			UpdateInCoop( a_fDeltaTime );
			break;
		case STATE_ESCAPE:
			UpdateEscape( a_fDeltaTime );
			break;
		case STATE_FLEE:
			UpdateFlee( a_fDeltaTime );
			break;
		case STATE_MILKED:
			UpdateMilked( a_fDeltaTime );
			break;
		case STATE_MILKED_DROP:
			UpdateMilkedDrop( a_fDeltaTime );
			break;
		case STATE_THROWN:
			UpdateThrown( a_fDeltaTime );
			break;
		case STATE_SCARED:
			UpdateScared( a_fDeltaTime );
			break;
		case STATE_STALK:
			UpdateStalk( a_fDeltaTime );
			break;
		case STATE_ATTACK:
			UpdateAttack( a_fDeltaTime );
			break;
		default:
			break;
	}

	UpdateScare( a_fDeltaTime );
	UpdateSound( a_fDeltaTime );
	UpdateAnimation( a_fDeltaTime );

	return AVisibleUnit::Think( a_fDeltaTime );
}
