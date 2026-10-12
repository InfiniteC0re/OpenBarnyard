#include "pch.h"
#include "AMilkReticleManager.h"
#include "GUI/AGUI2.h"
#include "Render/ARenderer.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AAnimatedUnit.h"
#include "Units/Projectiles/AAimedSquirtProjectileLauncher.h"
#include "UnitMovement/Steering/AAlienSteer.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( c_pelvis );
TPSTRING8_DECLARE( c_pip_pelvis );

// The reticles stay for a while after the player stops aiming
// NOTE: retail keeps it in a global, shared by every AMilkReticleManager
static TFLOAT s_fShowTime = 0.0f; // 0x00780ea4

// $Barnyard: FUNCTION 00420c10
AMilkReticleManager::AMilkReticleManager()
    : m_vSearchCentre( TVector4::VEC_ZERO )
    , m_pCentreUnit( TNULL )
    , m_pCurrentTarget( TNULL )
    , m_pPrevTarget( TNULL )
    , m_fSearchRadius( 50.0f )
    , m_fSearchInterval( 1.0f )
    , m_iNumTargets( 0 )
    , m_iLockingTarget( -1 )
    , m_iPickIndex( -1 )
    , m_bActive( TFALSE )
    , m_bAiming( TFALSE )
    , m_bIgnoreScreen( TFALSE )
    , m_iTargetSound( -1 )
    , m_fPickTimer( 0.0f )
    , m_fSearchTimer( -1.0f )
    , m_fForgetTimer( 1.0f )
    , m_bTargetPicked( TFALSE )
{
	m_oCandidates.iNumCandidates = 0;
}

// $Barnyard: FUNCTION 00420890
AMilkReticleManager::~AMilkReticleManager()
{
	for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
		m_aTargets[ i ].oUnit = TNULL;
}

// $Barnyard: FUNCTION 004206a0
void AMilkReticleManager::Create( AUnit* a_pCentreUnit, TFLOAT a_fSearchRadius, Toshi::T2GUIElement* a_pParent )
{
	m_pCentreUnit   = a_pCentreUnit;
	m_fSearchRadius = a_fSearchRadius;
	m_bActive       = TFALSE;
	m_pPrevTarget   = TNULL;

	for ( TINT i = MAX_NUM_TARGETS - 1; i >= 0; i-- )
	{
		m_aTargets[ i ].oReticle.Create( a_pParent );
		m_aTargets[ i ].oReticle.SetActive( TFALSE, TFALSE, 0.0f );
	}
}

// $Barnyard: FUNCTION 00420580
void AMilkReticleManager::SetActive( TBOOL a_bActive )
{
	m_bActive = a_bActive;
	m_bAiming = TFALSE;

	if ( !a_bActive )
	{
		for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
		{
			m_aTargets[ i ].eState = STATE_HIDDEN;
			m_aTargets[ i ].oReticle.SetActive( TFALSE, TFALSE, 0.0f );
		}
	}
}

// $Barnyard: FUNCTION 00421870
void AMilkReticleManager::UpdateSim( TFLOAT a_fDeltaTime )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer || !pPlayer->IsA( &TGetClass( AUnitPlayer ) ) )
		return;

	AProjectileLauncher* pActiveLauncher = TSTATICCAST( AUnitPlayer, pPlayer )->GetActiveLauncher();

	if ( !pActiveLauncher )
		return;

	TBOOL bAiming = pActiveLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) );
	Update( a_fDeltaTime, bAiming );

	if ( !bAiming )
	{
		m_fPickTimer = 0.0f;

		if ( pPlayer->GetSteer()->IsA( &TGetClass( AAlienSteer ) ) )
			TSTATICCAST( AAlienSteer, pPlayer->GetSteer() )->SetStrafing( TFALSE );

		return;
	}

	AAimedSquirtProjectileLauncher* pLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pActiveLauncher );
	pLauncher->Update( 0.0f );

	ATargetReticle& rReticle = pLauncher->GetReticle();
	AUnit*          pTarget  = pLauncher->GetLockedTarget();
	TBOOL           bPick    = !pTarget;

	// Unlock if the milk doesn't get to the target
	if ( pTarget && pTarget != pLauncher->GetUnitUnderCursor() && TVector4::DistanceSq( pLauncher->GetTracePoint(), pLauncher->GetAimPoint() ) > 1.0f )
	{
		rReticle.SetMode( ATargetReticle::MODE_IDLE );
		pLauncher->LockOn( TNULL );
		SetCurrentTarget( TNULL );
		pTarget = TNULL;
		bPick   = TTRUE;
	}

	if ( bPick )
	{
		m_fPickTimer -= a_fDeltaTime;

		if ( m_fPickTimer <= 0.0f )
		{
			m_fPickTimer = 0.25f;

			pTarget = PickTarget( pLauncher, 1 );
			pLauncher->LockOn( pTarget );
			SetCurrentTarget( pTarget );

			rReticle.SetMode( pTarget ? ATargetReticle::MODE_LOCKED : ATargetReticle::MODE_IDLE );
		}
	}

	if ( pPlayer->GetSteer()->IsA( &TGetClass( AAlienSteer ) ) )
	{
		AAlienSteer* pSteer = TSTATICCAST( AAlienSteer, pPlayer->GetSteer() );
		pSteer->SetTarget( pLauncher->GetAimPoint() );
		pSteer->SetStrafing( pTarget != TNULL );
	}

	pLauncher->UpdateLockedCursor();
	rReticle.UpdateRectangles();
}

// $Barnyard: FUNCTION 00421a50
void AMilkReticleManager::UpdateMiniGame( TFLOAT a_fDeltaTime )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer || !pPlayer->IsA( &TGetClass( AUnitPlayer ) ) )
		return;

	AProjectileLauncher* pActiveLauncher = TSTATICCAST( AUnitPlayer, pPlayer )->GetActiveLauncher();
	TBOOL                bAiming         = pActiveLauncher->IsA( &TGetClass( AAimedSquirtProjectileLauncher ) );
	Update( a_fDeltaTime, bAiming );

	if ( !bAiming )
	{
		m_bTargetPicked = TFALSE;

		if ( pPlayer->GetSteer()->IsA( &TGetClass( AAlienSteer ) ) )
			TSTATICCAST( AAlienSteer, pPlayer->GetSteer() )->SetStrafing( TFALSE );

		return;
	}

	AAimedSquirtProjectileLauncher* pLauncher = TSTATICCAST( AAimedSquirtProjectileLauncher, pActiveLauncher );
	ATargetReticle&                 rReticle  = pLauncher->GetReticle();
	AUnit*                          pTarget   = pLauncher->GetLockedTarget();

	if ( !pTarget )
		m_bTargetPicked = TFALSE;

	if ( !m_bTargetPicked )
	{
		m_fPickTimer -= a_fDeltaTime;

		if ( m_fPickTimer <= 0.0f )
		{
			pTarget = PickTarget( pLauncher, 10 );
			pLauncher->LockOn( pTarget );

			if ( pTarget )
			{
				m_fPickTimer    = 0.0f;
				m_bTargetPicked = TTRUE;
			}
			else
			{
				m_fPickTimer = 0.5f;
			}
		}
	}

	SetCurrentTarget( pTarget );

	if ( pPlayer->GetSteer()->IsA( &TGetClass( AAlienSteer ) ) )
	{
		AAlienSteer* pSteer = TSTATICCAST( AAlienSteer, pPlayer->GetSteer() );
		pSteer->SetTarget( pLauncher->GetAimPoint() );
		pSteer->SetStrafing( m_bTargetPicked );
	}

	pLauncher->UpdateLockedCursor();

	if ( pTarget )
	{
		// Yellow while the milk doesn't get to the target
		if ( pTarget == pLauncher->GetUnitUnderCursor() || TVector4::DistanceSq( pLauncher->GetAimPoint(), pLauncher->GetTracePoint() ) <= 1.0f )
			rReticle.SetMode( ATargetReticle::MODE_LOCKED );
		else
			rReticle.SetMode( ATargetReticle::MODE_BLOCKED );
	}
	else
	{
		rReticle.SetMode( ATargetReticle::MODE_IDLE );
	}

	rReticle.UpdateRectangles();
}

// $Barnyard: FUNCTION 00421760
void AMilkReticleManager::Update( TFLOAT a_fDeltaTime, TBOOL a_bAiming )
{
	if ( !m_bAiming && a_bAiming == TTRUE )
		FindTargets();

	m_bAiming = a_bAiming;
	m_fSearchTimer -= a_fDeltaTime;

	if ( m_fSearchTimer < 0.0f )
	{
		FindTargets();
		m_fSearchTimer = m_fSearchInterval;
	}

	if ( a_bAiming )
	{
		m_fForgetTimer = 1.0f;
	}
	else
	{
		m_pCurrentTarget = TNULL;

		if ( m_fForgetTimer >= 0.0f )
		{
			m_fForgetTimer -= a_fDeltaTime;
		}
		else
		{
			m_pPrevTarget = TNULL;
			m_iPickIndex  = -1;
		}
	}

	if ( m_iNumTargets != 0 )
	{
		if ( a_bAiming )
		{
			m_bActive   = TTRUE;
			s_fShowTime = 4.0f;
		}
		else
		{
			s_fShowTime -= a_fDeltaTime;

			if ( s_fShowTime <= 0.0f )
				m_bActive = TFALSE;
		}

		for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
		{
			UpdateTargetPosition( i );
			UpdateReticle( i, a_fDeltaTime );
		}
	}
}

// $Barnyard: FUNCTION 00421680
void AMilkReticleManager::FindTargets()
{
	for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
		m_aTargets[ i ].bFound = TFALSE;

	if ( m_pCentreUnit )
		m_vSearchCentre = *m_pCentreUnit->GetSteer()->GetPosition();

	TINT                    iNumFound;
	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( m_vSearchCentre, m_fSearchRadius, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	for ( TINT i = iNumFound - 1; i >= 0; i-- )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit->IsA( &TGetClass( AVisibleUnit ) ) && TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() && ( pUnit->GetFlags() & UNIT_FLAG_MILK_TARGET ) )
			AddTarget( pUnit );
	}
}

// $Barnyard: FUNCTION 00421580
void AMilkReticleManager::AddTarget( AUnit* a_pUnit )
{
	TINT iFree    = -1;
	TINT iReplace = -1;

	if ( m_iNumTargets != 0 )
	{
		TINT iOffScreen = -1;

		for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
		{
			AUnitProxy* pProxy = m_aTargets[ i ].oUnit;

			if ( !pProxy || !pProxy->GetUnit() )
			{
				iFree = i;
				continue;
			}

			AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, pProxy->GetUnit() );

			if ( pUnit == a_pUnit )
			{
				// It already has a reticle
				m_aTargets[ i ].bFound = TTRUE;
				return;
			}

			if ( !pUnit->IsVisible() || !( pUnit->GetFlags() & UNIT_FLAG_MILK_TARGET ) )
				iFree = i;
			else if ( m_aTargets[ i ].bOffScreen )
				iOffScreen = i;
		}

		if ( iFree != -1 )
		{
			SetTarget( a_pUnit, iFree );
			return;
		}

		iReplace = iOffScreen;
	}

	if ( m_iNumTargets < MAX_NUM_TARGETS )
		SetTarget( a_pUnit, m_iNumTargets );
	else if ( iReplace != -1 )
		SetTarget( a_pUnit, iReplace );
}

// $Barnyard: FUNCTION 00420cd0
void AMilkReticleManager::SetTarget( AUnit* a_pUnit, TINT a_iTarget )
{
	Target& rTarget = m_aTargets[ a_iTarget ];

	rTarget.oUnit      = TNULL;
	rTarget.oUnit      = a_pUnit->GetSelfProxy();
	rTarget.vPosition  = *a_pUnit->GetSteer()->GetPosition();
	rTarget.bOffScreen = TFALSE;
	rTarget.bFound     = TTRUE;
	rTarget.eState     = STATE_HIDDEN;
	rTarget.iBone      = -1;

	if ( a_iTarget == m_iNumTargets )
	{
		m_iNumTargets++;

		if ( m_iLockingTarget == -1 )
			m_iLockingTarget = 0;
	}

	rTarget.oReticle.SetMode( ATargetReticle::MODE_TARGETS );
	rTarget.oReticle.SetActive( TFALSE, TTRUE, AGUI2::GetRootElement()->GetWidth() * 2.0f );
	rTarget.oReticle.UpdateSize( 0.0f );

	// The middle of the unit
	rTarget.vPosition = *a_pUnit->GetSteer()->GetPosition();
	rTarget.vPosition.y -= TSTATICCAST( AVisibleUnit, a_pUnit )->GetCollisionObject()->GetHeightScale() * 0.5f;

	// Or its pelvis
	if ( a_pUnit->IsA( &TGetClass( AAnimatedUnit ) ) && TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject() )
	{
		AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject();
		rTarget.iBone                        = pAnimatableObject->GetModelInstance()->GetBoneID( TPS8( c_pelvis ) );

		if ( rTarget.iBone < 0 )
			rTarget.iBone = pAnimatableObject->GetModelInstance()->GetBoneID( TPS8( c_pip_pelvis ) );
	}
}

// $Barnyard: FUNCTION 00420700
void AMilkReticleManager::SetCurrentTarget( AUnit* a_pUnit )
{
	if ( m_pCurrentTarget != a_pUnit && a_pUnit )
	{
		if ( m_iTargetSound != -1 )
			ASoundManager::GetSingleton()->StopCue( m_iTargetSound );

		m_iTargetSound = ASoundManager::GetSingleton()->PlayCueAtPositionEx( soundbank::UI_TARGET, *a_pUnit->GetSteer()->GetPosition(), 1.0f, TFALSE, 0.0f, -1 );
	}

	AUnit* pOldTarget = m_pCurrentTarget;
	m_pCurrentTarget  = a_pUnit;

	if ( pOldTarget && a_pUnit )
		m_pPrevTarget = pOldTarget;
}

// $Barnyard: FUNCTION 00421290
AUnit* AMilkReticleManager::PickTarget( AAimedSquirtProjectileLauncher* a_pLauncher, TINT a_iWeight )
{
	m_iPickIndex++;

	const TMatrix44& rcCameraMatrix  = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	TVector4         vCameraForward  = rcCameraMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4         vCameraPosition = rcCameraMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	m_oCandidates.iNumCandidates = 0;

	for ( TINT i = m_iNumTargets - 1; i >= 0; i-- )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( TUINT( rTarget.eState ) > STATE_LOCKED )
			continue;

		if ( !rTarget.oUnit || !rTarget.oUnit->GetUnit() )
			continue;

		// Is there anything between the camera and the target?
		TVector4 vDirection( rTarget.vPosition.x - vCameraPosition.x, rTarget.vPosition.y - vCameraPosition.y, rTarget.vPosition.z - vCameraPosition.z, rTarget.vPosition.w );
		TFLOAT   fDistance = vDirection.NormaliseMag();

		ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, rTarget.oUnit->GetUnit() )->GetCollisionObject();

		if ( pCollisionObject && pCollisionObject->GetObjectRadius() < fDistance )
			fDistance -= pCollisionObject->GetObjectRadius();

		ARayData oRay( vCameraPosition, vDirection, fDistance );

		if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, 0x80000 ) )
			continue;

		TINT iWeight;

		if ( !a_pLauncher || a_pLauncher->CanHit( rTarget.oUnit->GetUnit() ) )
			iWeight = 1;
		else if ( a_iWeight > 1 )
			iWeight = a_iWeight;
		else
			continue;

		InsertCandidate( i, vCameraForward.z * vDirection.z + vCameraForward.x * vDirection.x, fDistance, iWeight );
	}

	if ( m_oCandidates.iNumCandidates == 0 )
	{
		m_iPickIndex = 0;
		return TNULL;
	}

	// Go through the candidates, but not back to the previous target
	m_iPickIndex %= m_oCandidates.iNumCandidates;

	if ( m_pPrevTarget == m_aTargets[ m_oCandidates.aCandidates[ m_iPickIndex ].iTarget ].oUnit->GetUnit() )
		m_iPickIndex = ( m_iPickIndex + 1 ) % m_oCandidates.iNumCandidates;

	return m_aTargets[ m_oCandidates.aCandidates[ m_iPickIndex ].iTarget ].oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00420ad0
void AMilkReticleManager::InsertCandidate( TINT a_iTarget, TFLOAT a_fDot, TFLOAT a_fDistance, TINT a_iWeight )
{
	Candidate oCandidate;
	oCandidate.fDot      = a_fDot;
	oCandidate.fDistance = a_fDistance;
	oCandidate.fScore    = ( a_fDot * 100.0f + 1200.0f / a_fDistance ) / TFLOAT( a_iWeight );
	oCandidate.iTarget   = a_iTarget;

	// Sorted by the score, highest first
	for ( TINT i = 0; i < m_oCandidates.iNumCandidates; i++ )
	{
		if ( oCandidate.fScore > m_oCandidates.aCandidates[ i ].fScore )
		{
			m_oCandidates.InsertAt( i, oCandidate );
			return;
		}
	}

	m_oCandidates.aCandidates[ m_oCandidates.iNumCandidates++ ] = oCandidate;
}

// $Barnyard: FUNCTION 00420790
void AMilkReticleManager::CandidateList::InsertAt( TINT a_iIndex, const Candidate& a_rcCandidate )
{
	MakeRoom( a_iIndex, 1, TFALSE );
	aCandidates[ a_iIndex ] = a_rcCandidate;
}

// $Barnyard: FUNCTION 004205e0
void AMilkReticleManager::CandidateList::MakeRoom( TINT a_iIndex, TINT a_iCount, TBOOL a_bMove )
{
	if ( a_iIndex < iNumCandidates )
	{
		if ( a_bMove )
		{
			TUtil::MemMove( &aCandidates[ a_iIndex + a_iCount ], &aCandidates[ a_iIndex ], sizeof( Candidate ) * ( iNumCandidates - a_iIndex ) );
		}
		else
		{
			for ( TINT i = iNumCandidates - 1; i >= a_iIndex; i-- )
				aCandidates[ i + a_iCount ] = aCandidates[ i ];
		}
	}

	iNumCandidates += a_iCount;
}

// $Barnyard: FUNCTION 004204c0
void AMilkReticleManager::NextLockingTarget()
{
	// The next waiting one after the current one...
	for ( TINT i = m_iLockingTarget + 1; i < m_iNumTargets; i++ )
	{
		if ( m_aTargets[ i ].eState == STATE_WAITING )
		{
			m_aTargets[ i ].eState = STATE_LOCKING;
			m_iLockingTarget       = i;
			return;
		}
	}

	// ...or before it
	for ( TINT i = 0; i < m_iLockingTarget; i++ )
	{
		if ( m_aTargets[ i ].eState == STATE_WAITING )
		{
			m_aTargets[ i ].eState = STATE_LOCKING;
			m_iLockingTarget       = i;
			return;
		}
	}

	m_iLockingTarget = -1;
}

// $Barnyard: FUNCTION 00420f50
void AMilkReticleManager::UpdateTargetPosition( TINT a_iTarget )
{
	Target& rTarget = m_aTargets[ a_iTarget ];
	TBOOL   bValid  = TFALSE;

	if ( rTarget.oUnit && rTarget.oUnit->GetUnit() )
	{
		AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, rTarget.oUnit->GetUnit() );

		if ( pUnit->IsVisible() && ( pUnit->GetFlags() & UNIT_FLAG_MILK_TARGET ) && rTarget.bFound )
		{
			if ( pUnit->IsA( &TGetClass( AAnimatedUnit ) ) && TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject() && rTarget.iBone >= 0 )
			{
				TMatrix44 matBone;
				TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( rTarget.iBone, matBone );

				matBone.GetTranslation( rTarget.vPosition );
			}
			else
			{
				rTarget.vPosition = *pUnit->GetSteer()->GetPosition();
				rTarget.vPosition.y -= pUnit->GetCollisionObject()->GetHeightScale() * 0.5f;
			}

			bValid = TTRUE;
		}
	}

	TVector2 vScreenPos;
	TBOOL    bOnScreen = TTRUE;

	if ( !m_bIgnoreScreen )
		bOnScreen = ARenderer::GetSingleton()->GetScreenPosition( rTarget.vPosition, vScreenPos );

	TVector4 vGUIPos = rTarget.vPosition;
	ARenderer::GetSingleton()->WorldToGUI( vGUIPos );
	vScreenPos.x = vGUIPos.x;
	vScreenPos.y = vGUIPos.y;

	if ( !bOnScreen )
	{
		rTarget.bOffScreen = TTRUE;
		rTarget.eState     = STATE_HIDDEN;
		return;
	}

	rTarget.oReticle.SetPosition( vScreenPos );

	if ( bValid )
	{
		rTarget.bOffScreen = TFALSE;

		switch ( rTarget.eState )
		{
			case STATE_WAITING:
				if ( !m_bActive )
				{
					rTarget.eState = STATE_HIDDEN;
				}
				else if ( m_iLockingTarget == -1 || m_iLockingTarget == a_iTarget )
				{
					rTarget.eState   = STATE_LOCKING;
					m_iLockingTarget = a_iTarget;
				}

				break;
			case STATE_LOCKING:
			case STATE_LOCKED:
				if ( !m_bActive )
					rTarget.eState = STATE_RELEASING;

				break;
			case STATE_RELEASING:
				if ( rTarget.oReticle.IsLocked() )
					rTarget.eState = STATE_HIDDEN;

				break;
			case STATE_HIDDEN:
				if ( m_bActive )
					rTarget.eState = STATE_WAITING;

				break;
		}
	}
	else
	{
		// The target is gone
		switch ( rTarget.eState )
		{
			case STATE_WAITING:
				rTarget.eState = STATE_HIDDEN;
				break;
			case STATE_LOCKING:
			case STATE_LOCKED:
				rTarget.eState = STATE_RELEASING;
				break;
			case STATE_RELEASING:
				if ( rTarget.oReticle.IsLocked() )
					rTarget.eState = STATE_HIDDEN;

				break;
		}
	}
}

// $Barnyard: FUNCTION 00420930
void AMilkReticleManager::UpdateReticle( TINT a_iTarget, TFLOAT a_fDeltaTime )
{
	Target&         rTarget  = m_aTargets[ a_iTarget ];
	ATargetReticle& rReticle = rTarget.oReticle;

	switch ( rTarget.eState )
	{
		case STATE_LOCKING:
		{
			if ( !rReticle.IsActive() )
			{
				if ( rReticle.GetMode() != ATargetReticle::MODE_TARGETS )
				{
					rReticle.SetMode( ATargetReticle::MODE_TARGETS );
					rReticle.UpdateSize( 0.0f );
				}

				rReticle.SetActive( m_bActive, TTRUE, AGUI2::GetRootElement()->GetWidth() * 0.25f );
			}

			// The current target is red
			ATargetReticle::MODE iMode = ( m_pCurrentTarget == rTarget.oUnit->GetUnit() ) ? ATargetReticle::MODE_CURRENT_TARGET : ATargetReticle::MODE_TARGETS;

			if ( rReticle.GetMode() != iMode )
			{
				rReticle.SetMode( iMode );
				rReticle.UpdateSize( 0.0f );
			}

			rReticle.Update( a_fDeltaTime );

			if ( rReticle.IsLocked() )
				rTarget.eState = STATE_LOCKED;

			break;
		}
		case STATE_LOCKED:
		{
			ATargetReticle::MODE iMode = ( m_pCurrentTarget == rTarget.oUnit->GetUnit() ) ? ATargetReticle::MODE_CURRENT_TARGET : ATargetReticle::MODE_TARGETS;

			if ( rReticle.GetMode() != iMode )
			{
				rReticle.SetMode( iMode );
				rReticle.UpdateSize( 0.0f );
			}

			rReticle.Update( a_fDeltaTime );
			break;
		}
		case STATE_RELEASING:
			if ( rReticle.GetMode() != ATargetReticle::MODE_RELEASING )
				rReticle.SetMode( ATargetReticle::MODE_RELEASING );

			rReticle.Update( a_fDeltaTime );
			break;
		case STATE_HIDDEN:
			if ( rReticle.IsActive() )
				rReticle.SetActive( TFALSE, TFALSE, 0.0f );

			break;
	}

	if ( a_iTarget == m_iLockingTarget && rTarget.eState != STATE_LOCKING )
		NextLockingTarget();
}
