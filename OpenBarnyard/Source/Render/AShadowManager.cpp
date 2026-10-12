#include "pch.h"
#include "AShadowManager.h"
#include "Animation/AAnimatableObject.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Tasks/ARootTask.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FX/AParticleEffectManager.h"
#include "Terrain/ATerrainInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0060cb50, CreateObject 0060dba0, CreateObjectInPlace 0060dc00
TDEFINE_CLASS( AShadowManager );

// Shadows further than this from the camera (squared, 30 units) have their effect killed.
static constexpr TFLOAT SHADOW_MAX_DISTANCE_SQ = 900.0f;

// $Barnyard: FUNCTION 0060d300
AShadowManager::AShadowManager()
{
	Reset();
}

// $Barnyard: FUNCTION 0060cb80
// $Barnyard: FUNCTION 0060cc00
AShadowManager::~AShadowManager()
{
}

// $Barnyard: FUNCTION 0060cb60
void __stdcall AShadowManager::OnShadowEffectFinished( AParticleEffectHandle* a_pEffect, void* a_pUserData )
{
	TSTATICCAST( Shadow, a_pUserData )->pEffect = TNULL;
}

// $Barnyard: FUNCTION 0060d080
void AShadowManager::Reset()
{
	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pEffect )
			it->pEffect->GetInstance()->Kill();
	}

	m_oUsedShadows.RemoveAll();
	m_oFreeShadows.RemoveAll();

	for ( TINT i = 0; i < MAX_NUM_SHADOWS; i++ )
		m_oFreeShadows.InsertTail( &m_aShadows[ i ] );
}

// Sets the scale of the shadow effect a_pEffect (inlined in the original)
static void SetShadowEffectScale( AParticleEffectHandle* a_pEffect, const TVector2& a_rcScale, AShadowManager::FLAGS a_eFlags )
{
	TMatrix44& rTransform = a_pEffect->GetInstance()->GetTransform();

	if ( a_eFlags & AShadowManager::FLAGS_UNIFORM_SCALE )
	{
		rTransform.Scale( a_rcScale.x, a_rcScale.x, a_rcScale.x );
	}
	else
	{
		TVector4 vecScaled;
		vecScaled.Multiply( rTransform.AsBasisVector4( BASISVECTOR_RIGHT ), a_rcScale.x );
		rTransform.AsBasisVector4( BASISVECTOR_RIGHT ) = vecScaled;

		vecScaled.Multiply( rTransform.AsBasisVector4( BASISVECTOR_FORWARD ), a_rcScale.y );
		rTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vecScaled;
	}
}

// $Barnyard: FUNCTION 0060cc20
void AShadowManager::AddUnitShadow( AUnit* a_pUnit, const TVector2& a_rcScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType )
{
	// Only one shadow per owner
	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pUnit )
			return;
	}

	m_iEffectIndex                 = AParticleEffectManager::FindEffect( ( a_eType == TYPE_CAR ) ? "shadow_car" : "shadow" );
	AParticleEffectHandle* pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( m_iEffectIndex, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

	if ( pEffect )
	{
		const TVector4* pPosition  = a_pUnit->GetSteer()->GetPosition();
		TVector4&       rEffectPos = pEffect->GetInstance()->GetTransform().GetTranslation();

		rEffectPos.x = pPosition->x;
		rEffectPos.y = pPosition->y + a_fHeightOffset;
		rEffectPos.z = pPosition->z;
		rEffectPos.w = pPosition->w;

		SetShadowEffectScale( pEffect, a_rcScale, a_eFlags );

		// NOTE: the free list isn't checked for being empty
		Shadow* pShadow = m_oFreeShadows.Tail();
		m_oFreeShadows.Remove( pShadow );

		pShadow->iBone         = a_iBone;
		pShadow->pOwner        = a_pUnit;
		pShadow->pEffect       = pEffect;
		pShadow->vecScale      = a_rcScale;
		pShadow->fHeightOffset = a_fHeightOffset;
		pShadow->eType         = a_eType;
		pShadow->eFlags        = a_eFlags | FLAGS_UNIT | FLAGS_ENABLED;
		m_oUsedShadows.InsertTail( pShadow );

		pEffect->SetOnFinishedCallback( OnShadowEffectFinished, pShadow );
	}
}

// $Barnyard: FUNCTION 0060cf90
void AShadowManager::AddUnitShadow( AUnit* a_pUnit, TFLOAT a_fScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType )
{
	AddUnitShadow( a_pUnit, TVector2( a_fScale, a_fScale ), a_iBone, a_fHeightOffset, a_eFlags, a_eType );
}

// $Barnyard: FUNCTION 0060cde0
void AShadowManager::AddModelShadow( AModelInstance* a_pModelInstance, const TVector2& a_rcScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType )
{
	// Only one shadow per owner
	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pModelInstance )
			return;
	}

	m_iEffectIndex                 = AParticleEffectManager::FindEffect( ( a_eType == TYPE_CAR ) ? "shadow_car" : "shadow" );
	AParticleEffectHandle* pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( m_iEffectIndex, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

	if ( pEffect )
	{
		// NOTE: no height offset here
		TVector4 vecPosition;
		a_pModelInstance->GetPosition( vecPosition );
		pEffect->GetInstance()->GetTransform().GetTranslation() = vecPosition;

		SetShadowEffectScale( pEffect, a_rcScale, a_eFlags );

		// NOTE: the free list isn't checked for being empty
		Shadow* pShadow = m_oFreeShadows.Tail();
		m_oFreeShadows.Remove( pShadow );

		pShadow->iBone         = a_iBone;
		pShadow->pOwner        = a_pModelInstance;
		pShadow->pEffect       = pEffect;
		pShadow->vecScale      = a_rcScale;
		pShadow->fHeightOffset = a_fHeightOffset;
		pShadow->eType         = a_eType;
		pShadow->eFlags        = a_eFlags | FLAGS_MODEL | FLAGS_ENABLED;
		m_oUsedShadows.InsertTail( pShadow );

		pEffect->SetOnFinishedCallback( OnShadowEffectFinished, pShadow );
	}
}

// $Barnyard: FUNCTION 0060cfd0
void AShadowManager::AddModelShadow( AModelInstance* a_pModelInstance, TFLOAT a_fScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType )
{
	AddModelShadow( a_pModelInstance, TVector2( a_fScale, a_fScale ), a_iBone, a_fHeightOffset, a_eFlags, a_eType );
}

// $Barnyard: FUNCTION 0060d010
void AShadowManager::RemoveShadowImp( TObject* a_pOwner )
{
	if ( !a_pOwner )
		return;

	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pOwner )
		{
			if ( it->pEffect )
				it->pEffect->GetInstance()->Kill();

			Shadow* pShadow = it;
			m_oUsedShadows.Remove( pShadow );
			m_oFreeShadows.InsertTail( pShadow );
			return;
		}
	}
}

// $Barnyard: FUNCTION 0060d390
void AShadowManager::RemoveShadow( TObject* a_pOwner )
{
	if ( a_pOwner )
		RemoveShadowImp( a_pOwner );
}

// $Barnyard: FUNCTION 0060d100
void AShadowManager::SetShadowFlagImp( TObject* a_pOwner, FLAGS a_eFlag, TBOOL a_bSet )
{
	if ( !a_pOwner )
		return;

	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pOwner )
		{
			it->eFlags &= ~a_eFlag;

			if ( a_bSet == TTRUE )
				it->eFlags |= a_eFlag;

			return;
		}
	}
}

// $Barnyard: FUNCTION 0060d240
void AShadowManager::SetShadowStatic( TObject* a_pOwner, TBOOL a_bStatic )
{
	if ( a_pOwner )
		SetShadowFlagImp( a_pOwner, FLAGS_STATIC, a_bStatic );
}

// $Barnyard: FUNCTION 0060d180
void AShadowManager::SetShadowEnabledImp( TObject* a_pOwner, TBOOL a_bEnabled )
{
	if ( !a_pOwner )
		return;

	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pOwner )
		{
			if ( !it->pEffect )
			{
				if ( a_bEnabled )
				{
					// NOTE: always "shadow", even for TYPE_CAR
					m_iEffectIndex = AParticleEffectManager::FindEffect( "shadow" );
					it->pEffect    = AParticleEffectManager::GetSingleton()->PlayEffect( m_iEffectIndex, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

					if ( it->pEffect )
						it->pEffect->SetOnFinishedCallback( OnShadowEffectFinished, it );
				}
			}
			else if ( !a_bEnabled )
			{
				it->pEffect->GetInstance()->Kill();
				it->pEffect = TNULL;
			}

			it->eFlags &= ~FLAGS_ENABLED;

			if ( a_bEnabled == TTRUE )
				it->eFlags |= FLAGS_ENABLED;

			return;
		}
	}
}

// $Barnyard: FUNCTION 0060db60
void AShadowManager::SetShadowEnabled( TObject* a_pOwner, TBOOL a_bEnabled )
{
	if ( a_pOwner )
		SetShadowEnabledImp( a_pOwner, a_bEnabled );
}

// $Barnyard: FUNCTION 0060d260
void AShadowManager::SetShadowScaleImp( TObject* a_pOwner, const TVector2& a_rcScale )
{
	if ( !a_pOwner )
		return;

	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pOwner == a_pOwner )
		{
			it->vecScale = a_rcScale;
			return;
		}
	}
}

// $Barnyard: FUNCTION 0060db80
void AShadowManager::SetShadowScale( TObject* a_pOwner, const TVector2& a_rcScale )
{
	if ( a_pOwner )
		SetShadowScaleImp( a_pOwner, a_rcScale );
}

// $Barnyard: FUNCTION 0060d2b0
void AShadowManager::OnTerrainSectionChanged( TINT a_iUnk1, TINT a_iUnk2 )
{
	T2_FOREACH( m_oUsedShadows, it )
	{
		if ( it->pEffect )
			it->eFlags |= FLAGS_FORCE_UPDATE;
	}
}

// Plays the shadow effect of a_pShadow again (inlined in the original)
static AParticleEffectHandle* RestartShadowEffect( AShadowManager::Shadow* a_pShadow, TINT& a_rEffectIndex, AParticleEffectHandle::t_OnFinished a_fnOnFinished )
{
	a_rEffectIndex     = AParticleEffectManager::FindEffect( ( a_pShadow->eType == AShadowManager::TYPE_CAR ) ? "shadow_car" : "shadow" );
	a_pShadow->pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( a_rEffectIndex, TVector4::VEC_ZERO, TVector4::VEC_NEGY );

	if ( a_pShadow->pEffect )
		a_pShadow->pEffect->SetOnFinishedCallback( a_fnOnFinished, a_pShadow );

	return a_pShadow->pEffect;
}

// Puts the shadow effect on the ground below a_rcPosition (inlined in the original)
static void ProjectShadowEffect( AShadowManager::Shadow* a_pShadow, const TVector4& a_rcPosition, const TVector4& a_rcLookDirection )
{
	TVector4 vecGroundPos;
	TINT     iAlpha = 0;
	TVector3 vecRotation;
	TVector4 vecNormal;

	if ( !ATerrainInterface::GetSingleton()->GetGroundShadowInfo( a_rcPosition, a_pShadow->fHeightOffset, vecGroundPos, iAlpha, vecRotation, vecNormal, 0.5f, 10.0f, 0x80000 ) )
		return;

	TMatrix44& rTransform = a_pShadow->pEffect->GetInstance()->GetTransform();
	rTransform.Identity();
	rTransform.GetTranslation() = vecGroundPos;
	rTransform.LookAtDirection( vecNormal, a_rcLookDirection );
	rTransform.RotateX( -TMath::HALF_PI );

	if ( a_pShadow->eFlags & AShadowManager::FLAGS_NONUNIFORM_SCALE )
	{
		TVector4 vecScaled;
		vecScaled.Multiply( rTransform.AsBasisVector4( BASISVECTOR_RIGHT ), a_pShadow->vecScale.x );
		rTransform.AsBasisVector4( BASISVECTOR_RIGHT ) = vecScaled;

		vecScaled.Multiply( rTransform.AsBasisVector4( BASISVECTOR_FORWARD ), a_pShadow->vecScale.y );
		rTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vecScaled;
	}
	else
	{
		rTransform.Scale( a_pShadow->vecScale.x, a_pShadow->vecScale.x, a_pShadow->vecScale.x );
	}
}

// TRUE if the shadow effect is still at a_rcPosition on the XZ plane (inlined in the original)
static TBOOL IsShadowEffectAt( AShadowManager::Shadow* a_pShadow, const TVector4& a_rcPosition )
{
	TVector4 vecDelta = a_pShadow->pEffect->GetInstance()->GetTransform().GetTranslation();
	vecDelta.x -= a_rcPosition.x;
	vecDelta.y -= a_rcPosition.y;
	vecDelta.z -= a_rcPosition.z;

	return vecDelta.x * vecDelta.x + vecDelta.z * vecDelta.z < 0.0001f;
}

// $Barnyard: FUNCTION 0060d3b0
void AShadowManager::Update()
{
	if ( ARootTask::GetSingleton()->IsTransitioning() == TTRUE )
		return;

	T2_FOREACH( m_oUsedShadows, it )
	{
		Shadow*                pShadow = it;
		AParticleEffectHandle* pEffect = pShadow->pEffect;

		if ( HASANYFLAG( pShadow->eFlags, FLAGS_UNIT ) )
		{
			AUnit* pUnit    = TSTATICCAST( AUnit, pShadow->pOwner );
			TBOOL  bInRange = TFALSE;

			if ( HASANYFLAG( pShadow->eFlags, FLAGS_ENABLED ) &&
			     TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->IsUpdatingSkeleton() &&
			     TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance()->IsVisible() )
			{
				const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
				bInRange                    = ( *pUnit->GetSteer()->GetPosition() - rcCameraPos ).MagnitudeSq() < SHADOW_MAX_DISTANCE_SQ;
			}

			if ( !bInRange )
			{
				if ( pEffect )
				{
					pEffect->GetInstance()->Kill();
					pShadow->pEffect = TNULL;
				}

				continue;
			}

			if ( !pEffect )
			{
				pEffect = RestartShadowEffect( pShadow, m_iEffectIndex, OnShadowEffectFinished );

				if ( !pEffect )
					continue;

				pShadow->eFlags |= FLAGS_FORCE_UPDATE;
			}

			TVector4 vecPosition;

			if ( pShadow->iBone == -1 )
			{
				AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance();
				pModelInstance->GetPosition( vecPosition );
			}
			else
			{
				TMatrix44         matBone;
				AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetModelInstance();
				pModelInstance->GetBoneTransform( pShadow->iBone, matBone );
				vecPosition = matBone.GetTranslation();
			}

			if ( HASANYFLAG( pShadow->eFlags, FLAGS_FORCE_UPDATE ) )
			{
				pShadow->eFlags &= ~FLAGS_FORCE_UPDATE;
			}
			else
			{
				if ( HASANYFLAG( pShadow->eFlags, FLAGS_STATIC | FLAGS_UNK4 ) )
					continue;

				// Only reproject once the owner moved on the XZ plane
				if ( IsShadowEffectAt( pShadow, vecPosition ) )
					continue;
			}

			TVector4 vecDirection = *pUnit->GetSteer()->GetDirection();
			vecDirection.w        = 1.0f;

			ProjectShadowEffect( pShadow, vecPosition, vecDirection );
		}
		else
		{
			AModelInstance* pModelInstance = TSTATICCAST( AModelInstance, pShadow->pOwner );

			if ( !HASANYFLAG( pShadow->eFlags, FLAGS_ENABLED ) || !pModelInstance->IsVisible() )
			{
				if ( pEffect )
				{
					pEffect->GetInstance()->Kill();
					pShadow->pEffect = TNULL;
				}

				continue;
			}

			const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
			TVector4        vecModelPos;
			pModelInstance->GetPosition( vecModelPos );

			if ( TVector4::DistanceSq( rcCameraPos, vecModelPos ) >= SHADOW_MAX_DISTANCE_SQ )
			{
				if ( pEffect )
				{
					pEffect->GetInstance()->Kill();
					pShadow->pEffect = TNULL;
				}

				continue;
			}

			if ( !pEffect )
			{
				pEffect = RestartShadowEffect( pShadow, m_iEffectIndex, OnShadowEffectFinished );

				if ( !pEffect )
					continue;

				pShadow->eFlags |= FLAGS_FORCE_UPDATE;
			}

			TVector4 vecPosition;

			if ( pShadow->iBone == -1 )
			{
				pModelInstance->GetPosition( vecPosition );
			}
			else
			{
				TMatrix44 matBone;
				pModelInstance->GetBoneTransform( pShadow->iBone, matBone );
				vecPosition = matBone.GetTranslation();
			}

			if ( HASANYFLAG( pShadow->eFlags, FLAGS_FORCE_UPDATE ) )
			{
				pShadow->eFlags &= ~FLAGS_FORCE_UPDATE;
			}
			else
			{
				if ( HASANYFLAG( pShadow->eFlags, FLAGS_STATIC | FLAGS_UNK4 ) )
					continue;

				// Only reproject once the owner moved on the XZ plane
				if ( IsShadowEffectAt( pShadow, vecPosition ) )
					continue;
			}

			ProjectShadowEffect( pShadow, vecPosition, ACamera::sm_vInitialLookDirection );
		}
	}
}
