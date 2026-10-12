#include "pch.h"
#include "AAnimationBreakpointSoundParticle.h"
#include "AAnimatableObject.h"
#include "FX/AParticleEffectManager.h"
#include "Helpers/AWeatherManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "Helpers/AGameLoader.h"
#include "GameInterface/AGameStateController.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionModelInstance.h"
#include "MiniGame/ABarnExploreMinigameState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00584ab0, CreateObject 00584ed0, CreateObjectInPlace 00584f60 (the constructor is inlined in both)
TDEFINE_CLASS( AAnimationBreakpointSoundParticle );

AAnimationBreakpointSoundParticle::AAnimationBreakpointSoundParticle()
    : m_eSoundId( -1 )
    , m_fVolume( 1.0f )
    , m_bFootstep( TFALSE )
    , m_iWaterRunEffect1( -1 )
    , m_iWaterRunEffect2( -1 )
    , m_bVOStream( TFALSE )
{
}

// $Barnyard: FUNCTION 00585000
// $Barnyard: FUNCTION 00584fe0
AAnimationBreakpointSoundParticle::~AAnimationBreakpointSoundParticle()
{
}

// $Barnyard: FUNCTION 00584ac0
TBOOL AAnimationBreakpointSoundParticle::SetSoundBreakpoint( const ASoundBreakpoint* a_pSoundBreakpoint )
{
	AAnimationBreakpoint::SetTime( a_pSoundBreakpoint->fTime );

	m_eSoundId    = a_pSoundBreakpoint->eSoundId;
	m_bFootstep   = ( a_pSoundBreakpoint->eFlags & ASoundBreakpoint::FLAGS_FOOTSTEP ) != 0;
	m_bVOStream   = ( a_pSoundBreakpoint->eFlags & ASoundBreakpoint::FLAGS_VO_STREAM ) != 0;
	m_strBoneName = TPString8( a_pSoundBreakpoint->pEffects->pszBoneName );
	m_bEffectDown = ( a_pSoundBreakpoint->eFlags & ASoundBreakpoint::FLAGS_EFFECT_DOWN ) != 0;

	for ( TINT i = 0; i < ASoundBreakpointEffects::NUM_EFFECTS; i++ )
	{
		const TCHAR* pszEffect = a_pSoundBreakpoint->pEffects->apszParticleEffects[ i ];
		m_aEffects[ i ]        = pszEffect ? AParticleEffectManager::FindEffect( pszEffect ) : -1;
	}

	if ( m_bFootstep && !AGameLoader::IsBarnExploreMiniGame() )
	{
		m_iWaterRunEffect1 = AParticleEffectManager::FindEffect( "waterrun" );
		m_iWaterRunEffect2 = AParticleEffectManager::FindEffect( "waterrun" );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00584cc0
void AAnimationBreakpointSoundParticle::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	AModelInstance* pModelInstance = a_pObject->GetModelInstance();

	if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() )
		return;

	// Get transform of the bone (or of the model if there's no such bone)
	TINT      iBone = -1;
	TMatrix44 matTransform;

	if ( m_strBoneName.GetPooledString() && ( iBone = pModelInstance->GetBoneID( m_strBoneName ) ) != -1 )
		pModelInstance->GetBoneTransform( iBone, matTransform );
	else
		pModelInstance->GetLocalTransform( matTransform );

	SURFACE eSurface = SURFACE_DEFAULT;

	if ( m_bFootstep )
	{
		eSurface = GetFootstepSurface( matTransform );

		if ( eSurface == SURFACE_NONE )
			return;
	}

	if ( m_eSoundId != -1 )
	{
		if ( m_bVOStream )
		{
			if ( !AVOStream::GetSingleton()->IsPlaying() )
				AVOStream::GetSingleton()->Play( m_eSoundId, TFALSE );
		}
		else
		{
			ASoundManager::GetSingleton()->PlayCueAtPositionEx( m_eSoundId, matTransform.GetTranslation(), m_fVolume, TFALSE, 0.0f, eSurface );
		}
	}

	if ( !m_strBoneName.GetPooledString() )
		return;

	TINT iEffect = m_aEffects[ eSurface + 1 ];

	if ( iEffect < 0 || iBone == -1 )
		return;

	TVector4 vecPosition = matTransform.GetTranslation();
	TVector4 vecDirection;

	if ( m_bEffectDown )
		vecDirection = TVector4::VEC_NEGY;
	else
		vecDirection = TVector4( matTransform.AsBasisVector3( BASISVECTOR_UP ) );

	if ( m_bFootstep && eSurface != SURFACE_WATER && eSurface != SURFACE_MUD && !AGameLoader::IsBarnExploreMiniGame() )
	{
		// Footsteps splash water while it rains
		AWeatherManager* pWeather = AWeatherManager::GetSingleton();

		if ( pWeather->IsBadWeather() )
		{
			if ( pWeather->IsHeavyRain() )
				iEffect = m_iWaterRunEffect1;
			else
				iEffect = m_iWaterRunEffect2;
		}
	}

	AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vecPosition, vecDirection );
}

// $Barnyard: FUNCTION 0057fe40
AAnimationBreakpointSoundParticle::SURFACE AAnimationBreakpointSoundParticle::GetFootstepSurface( TMatrix44& a_rTransform )
{
	// The barn has a wooden floor
	if ( AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ABarnExploreMinigameState ) ) )
		return SURFACE_TIMBER;

	// Cast a ray up, then down from the translation
	TVector4                  vecOrigin = a_rTransform.GetTranslation();
	TFLOAT                    fMaxDist  = 1.0f;
	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( vecOrigin, TVector4::VEC_POSY, fMaxDist, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TTRUE );

	TBOOL bDown = TFALSE;

	if ( oHitList.iNumHits == 0 )
	{
		ATerrainInterface::GetSingleton()->CollideRay( vecOrigin, TVector4::VEC_NEGY, fMaxDist, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE );
		bDown = TTRUE;
	}

	if ( oHitList.iNumHits < 1 )
		return SURFACE_NONE;

	// Find the closest hit
	TINT   iClosest = -1;
	TFLOAT fClosest = 50.0f;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		if ( oHitList.aHits[ i ].fDistance < fClosest )
		{
			iClosest = i;
			fClosest = oHitList.aHits[ i ].fDistance;
		}
	}

	if ( iClosest < 0 )
		return SURFACE_NONE;

	ACollisionModelRayHitList::Hit& rHit       = oHitList.aHits[ iClosest ];
	TModelCollisionData*            pCollData  = rHit.pInstance->GetCollisionData( rHit.iModel );
	TINT                            eCollGroup = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup;

	// Move the translation onto the surface
	if ( bDown )
		a_rTransform.m_f42 -= fClosest;
	else
		a_rTransform.m_f42 += fClosest;

	if ( eCollGroup == TCollisionCommon::TOSHICGROUP_WATER || eCollGroup == TCollisionCommon::TOSHICGROUP_DEEPWATER )
		a_rTransform.m_f42 -= 0.1f;

	switch ( eCollGroup )
	{
		case TCollisionCommon::TOSHICGROUP_GRASS: return SURFACE_GRASS;
		case TCollisionCommon::TOSHICGROUP_TIN: return SURFACE_TIN;
		case TCollisionCommon::TOSHICGROUP_TIMBER: return SURFACE_TIMBER;
		case TCollisionCommon::TOSHICGROUP_STONE: return SURFACE_STONE;
		case TCollisionCommon::TOSHICGROUP_DIRT: return SURFACE_DIRT;
		case TCollisionCommon::TOSHICGROUP_WATER:
		case TCollisionCommon::TOSHICGROUP_DEEPWATER: return SURFACE_WATER;
		case TCollisionCommon::TOSHICGROUP_SAND: return SURFACE_SAND;
		case TCollisionCommon::TOSHICGROUP_MUD: return SURFACE_MUD;
		case TCollisionCommon::TOSHICGROUP_HAY: return SURFACE_HAY;
		default: return SURFACE_NONE;
	}
}
