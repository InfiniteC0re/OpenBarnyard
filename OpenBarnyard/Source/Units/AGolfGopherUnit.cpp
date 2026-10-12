#include "pch.h"
#include "AGolfGopherUnit.h"
#include "ARandom.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "FX/AParticleEffectManager.h"
#include "UnitMovement/Steering/AGopherMoundSteer.h"

#include <Math/TMath.h>
#include <Render/TAnimation.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00417d40, CreateObject 00418c50, CreateObjectInPlace 00418cb0 (the constructor is inlined in both)
TDEFINE_CLASS( AGolfGopherUnit );

TPSTRING8_DECLARE( sod_1 );
TPSTRING8_DECLARE( sod_2 );
TPSTRING8_DECLARE( sod_3 );
TPSTRING8_DECLARE( submerge );
TPSTRING8_DECLARE( surface );
TPSTRING8_DECLARE( l_tunnel );
TPSTRING8_DECLARE( r_tunnel );
TPSTRING8_DECLARE( inground_surface );
TPSTRING8_DECLARE( aboveground_surface );
TPSTRING8_DECLARE( aboveground_dive );
TPSTRING8_DECLARE( victory_surface );
TPSTRING8_DECLARE( lose_surface );
TPSTRING8_DECLARE( inground_dive );
TPSTRING8_DECLARE( strike );
TPSTRING8_DECLARE( victory );
TPSTRING8_DECLARE( lose );
TPSTRING8_DECLARE( victory_end );
TPSTRING8_DECLARE( lose_end );

// $Barnyard: FUNCTION 00417e30
AGolfGopherUnit::AnimListener::AnimListener( TGenericEmitter& a_rEmitter, void* a_pCaller, TGenericListener::t_Callback a_fnCallback, TINT a_iPriority )
{
	pEmitter = &a_rEmitter;
	oListener.Connect( a_rEmitter, a_pCaller, a_fnCallback, a_iPriority );
}

// $Barnyard: FUNCTION 00417ed0
AGolfGopherUnit::AnimListener* AGolfGopherUnit::AnimListeners::Add( TGenericEmitter& a_rEmitter, TINT a_iPriority )
{
	AnimListener* pListener = new AnimListener( a_rEmitter, pCaller, fnCallback, a_iPriority );
	oList.InsertTail( pListener );

	return pListener;
}

// $Barnyard: FUNCTION 00417f50
void AGolfGopherUnit::AnimListeners::FreeAll()
{
	while ( !oList.IsEmpty() )
	{
		AnimListener* pListener = oList.Head();
		pListener->Remove();
		delete pListener;
	}
}

// $Barnyard: FUNCTION 00418b80
AGolfGopherUnit::AGolfGopherUnit()
{
	m_oSurfaceListeners.pCaller     = this;
	m_oSurfaceListeners.fnCallback  = TREINTERPRETCAST( TGenericListener::t_Callback, OnSurfaceEventCallback );
	m_oSubmergeListeners.pCaller    = this;
	m_oSubmergeListeners.fnCallback = TREINTERPRETCAST( TGenericListener::t_Callback, OnSubmergeEventCallback );

	m_iMoveEffect       = AParticleEffectManager::FindEffect( "gopher_move" );
	m_iMoveChunksEffect = AParticleEffectManager::FindEffect( "gopher_move_chunks" );
}

// $Barnyard: FUNCTION 00418aa0
// $Barnyard: FUNCTION 00418c30
AGolfGopherUnit::~AGolfGopherUnit()
{
}

// $Barnyard: FUNCTION 00417d50
void AGolfGopherUnit::ResetTrail( const TVector4& a_rcPosition )
{
	m_vTrailPosition = a_rcPosition;
	m_bSurfaced      = TFALSE;
}

// $Barnyard: FUNCTION 00417dd0
void AGolfGopherUnit::Submerge()
{
	for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
	{
		m_apMound[ i ]->KillAllAnimations();

		AToshiAnimationInterface& rAnimInterface = m_apMound[ i ]->GetToshiAnimInterface();
		AToshiAnimationRef*       pAnimRef       = rAnimInterface.PlayAnim( TPS8( submerge ), -1.0f, TFALSE );

		if ( pAnimRef )
			rAnimInterface.SetAnimSpeed( pAnimRef, 2.0f );
	}

	m_bSurfaced = TFALSE;
}

// $Barnyard: FUNCTION 00418b50
TBOOL __stdcall AGolfGopherUnit::OnSurfaceEventCallback( AGolfGopherUnit* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData )
{
	// The breakpoints are shared by every gopher
	if ( a_pThis->GetAnimatableObject() == a_pData->pObject )
		a_pThis->Surface();

	return TTRUE;
}

// $Barnyard: FUNCTION 00417ea0
TBOOL __stdcall AGolfGopherUnit::OnSubmergeEventCallback( AGolfGopherUnit* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData )
{
	if ( a_pThis->GetAnimatableObject() == a_pData->pObject )
		a_pThis->Submerge();

	return TTRUE;
}

// $Barnyard: FUNCTION 00417f90
TBOOL AGolfGopherUnit::Create( AUnitTransform* a_pTransform )
{
	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	AAnimatableObjectType* apSodTypes[ 3 ];
	apSodTypes[ 0 ] = pObjectManager->FindType( TPS8( sod_1 ) );
	apSodTypes[ 1 ] = pObjectManager->FindType( TPS8( sod_2 ) );
	apSodTypes[ 2 ] = pObjectManager->FindType( TPS8( sod_3 ) );

	// Creates a random sod, never the same kind twice in a row
	TINT iLastSod  = -1;
	auto CreateSod = [ & ]() {
		TINT iSod = ARandom::GetSingleton()->m_oRandom.GetInt( 3 );
		if ( iSod == iLastSod ) iSod = ( iSod + 1 ) % 3;

		iLastSod = iSod;

		AAnimatableObject* pSod = pObjectManager->CreateAnimatableObject( apSodTypes[ iSod ], TNULL, 0 );
		pSod->GetModelInstance()->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX );
		pSod->SetSkeletonUpdating( TFALSE, TTRUE );
		pSod->SetVisible( TTRUE );

		return pSod;
	};

	for ( TINT i = 0; i < NUM_TRAIL_OBJECTS; i++ )
		m_apTrail[ i ] = CreateSod();

	m_iNextTrail = 0;
	m_bTrailLeft = TFALSE;

	for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
		m_apMound[ i ] = CreateSod();

	m_bSurfaced = TFALSE;
	m_pSteer->CopyPosition( m_vTrailPosition );

	// NOTE: retail doesn't check if the model has the breakpoints
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( inground_surface ) )->GetEmitter(), 0 );
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( aboveground_surface ) )->GetEmitter(), 0 );
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( surface ) )->GetEmitter(), 0 );
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( aboveground_dive ) )->GetEmitter(), 0 );
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( victory_surface ) )->GetEmitter(), 0 );
	m_oSurfaceListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( lose_surface ) )->GetEmitter(), 0 );
	m_oSubmergeListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( inground_dive ) )->GetEmitter(), 0 );
	m_oSubmergeListeners.Add( GetAnimatableObject()->GetAnimationEmitter( TPS8( strike ) )->GetEmitter(), 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 00418400
void AGolfGopherUnit::OnDestroy()
{
	AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < NUM_TRAIL_OBJECTS; i++ )
		pObjectManager->DestroyAnimatableObject( m_apTrail[ i ] );

	for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
		pObjectManager->DestroyAnimatableObject( m_apMound[ i ] );

	// NOTE: retail only frees the surface listeners here, the submerge
	// listeners stay connected until the destructor
	m_oSurfaceListeners.FreeAll();

	AAnimatedUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00418490
TBOOL AGolfGopherUnit::Think( TFLOAT a_fDeltaTime )
{
	TBOOL bResult = AAnimatedUnit::Think( a_fDeltaTime );

	TVector4 vPosition;
	TVector4 vDirection;
	m_pSteer->CopyPosition( vPosition );
	m_pSteer->CopyDirection( vDirection );

	// Leave a trail object every 0.4 units
	if ( TVector4::DistanceSq( m_vTrailPosition, vPosition ) > 0.16f && TSTATICCAST( AGopherMoundSteer, m_pSteer )->IsUnderground() )
	{
		m_bTrailLeft = !m_bTrailLeft;
		vDirection.AsVector3().RotateY( TFLOAT( TINT( m_bTrailLeft ) ) * TMath::PI );

		AAnimatableObject* pTrail = m_apTrail[ m_iNextTrail ];
		pTrail->GetModelInstance()->SetTransform( vPosition, vDirection );
		pTrail->GetModelInstance()->SetSkeletonUpdating( TTRUE );
		pTrail->PlayAnimation( m_bTrailLeft ? TPS8( l_tunnel ) : TPS8( r_tunnel ) );

		m_vTrailPosition = vPosition;

		if ( ++m_iNextTrail >= NUM_TRAIL_OBJECTS )
			m_iNextTrail = 0;

		AParticleEffectManager::GetSingleton()->PlayEffect( m_iMoveEffect, vPosition, TVector4::VEC_NEGY );
		AParticleEffectManager::GetSingleton()->PlayEffect( m_iMoveChunksEffect, vPosition, TVector4::VEC_NEGY );
	}

	// Fade out the trail
	for ( TINT i = 0; i < NUM_TRAIL_OBJECTS; i++ )
	{
		AAnimatableObject* pTrail = m_apTrail[ i ];

		if ( pTrail->IsAnimPlaying( TPS8( l_tunnel ), 0.0f ) )
		{
			pTrail->SetOpacity( 1.0f - pTrail->GetAnimationProgress( TPS8( l_tunnel ) ), TTRUE );
		}
		else if ( pTrail->IsAnimPlaying( TPS8( r_tunnel ), 0.0f ) )
		{
			pTrail->SetOpacity( 1.0f - pTrail->GetAnimationProgress( TPS8( r_tunnel ) ), TTRUE );
		}
		else
		{
			pTrail->SetOpacity( 1.0f, TTRUE );
			pTrail->GetModelInstance()->SetSkeletonUpdating( TFALSE );
		}
	}

	// Fade out the submerging mound
	if ( m_apMound[ 0 ]->IsAnimPlaying( TPS8( submerge ), 0.0f ) )
	{
		TFLOAT fOpacity = 1.0f - m_apMound[ 0 ]->GetAnimationProgress( TPS8( submerge ) );

		for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
			m_apMound[ i ]->SetOpacity( fOpacity, TTRUE );
	}
	else
	{
		for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
		{
			m_apMound[ i ]->SetOpacity( 1.0f, TTRUE );
			m_apMound[ i ]->SetSkeletonUpdating( m_bSurfaced, TTRUE );
		}
	}

	// Submerge the mound once it has surfaced and the gopher dives or the game ends
	if ( m_bSurfaced && !m_apMound[ 0 ]->IsAnimPlaying( TPS8( surface ), 0.0f ) )
	{
		if ( !TSTATICCAST( AGopherMoundSteer, m_pSteer )->IsUnderground() )
			Submerge();

		if ( GetAnimatableObject()->IsAnimPlaying( TPS8( aboveground_dive ), 0.0f ) )
			Submerge();

		if ( GetAnimatableObject()->IsAnimPlaying( TPS8( victory ), 0.0f ) ||
		     GetAnimatableObject()->IsAnimPlaying( TPS8( lose ), 0.0f ) ||
		     GetAnimatableObject()->IsAnimPlaying( TPS8( victory_end ), 0.0f ) ||
		     GetAnimatableObject()->IsAnimPlaying( TPS8( lose_end ), 0.0f ) )
		{
			Submerge();
		}
	}

	return bResult;
}

// $Barnyard: FUNCTION 004188b0
void AGolfGopherUnit::Surface()
{
	TVector4 vPosition;
	TVector4 vDirection;
	m_pSteer->CopyPosition( vPosition );
	m_pSteer->CopyDirection( vDirection );

	// Place the mound objects in a ring around the gopher
	for ( TINT i = 0; i < NUM_MOUND_OBJECTS; i++ )
	{
		m_apMound[ i ]->GetModelInstance()->SetTransform( vPosition, vDirection );
		m_apMound[ i ]->GetModelInstance()->SetSkeletonUpdating( TTRUE );

		AToshiAnimationInterface& rAnimInterface = m_apMound[ i ]->GetToshiAnimInterface();
		rAnimInterface.KillAllAnimations();

		AToshiAnimationRef* pAnimRef = rAnimInterface.PlayAnim( TPS8( surface ), -1.0f, TFALSE );

		if ( pAnimRef )
			rAnimInterface.GetAnimation( *pAnimRef )->SetUpdateStateOnRemove( TTRUE );

		vDirection.AsVector3().RotateY( TMath::PI * 0.4f );
	}

	AParticleEffectManager::GetSingleton()->PlayEffect( m_iMoveEffect, vPosition, TVector4::VEC_NEGY );
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iMoveChunksEffect, vPosition, TVector4::VEC_NEGY );

	m_bSurfaced = TTRUE;
}
