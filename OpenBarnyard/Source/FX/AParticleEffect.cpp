#include "pch.h"
#include "AParticleSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

//-----------------------------------------------------------------------------
// AParticleEffectPool
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00674fb0
AParticleEffectPool::AParticleEffectPool( TINT a_iMaxInstances )
{
	m_iMaxInstances = a_iMaxInstances;
	m_pLibrary      = TNULL;

	AParticleEffectInstance* pInstances = TSTATICCAST( AParticleEffectInstance, TMalloc( a_iMaxInstances * sizeof( AParticleEffectInstance ) ) );

	if ( pInstances )
	{
		for ( TINT i = 0; i < a_iMaxInstances; i++ )
			new ( &pInstances[ i ] ) AParticleEffectInstance();
	}
	else
	{
		pInstances = TNULL;
	}

	m_pFreeInstances = TNULL;
	m_pUsedHead      = TNULL;
	m_pInstances     = pInstances;
	m_bHasStealable  = TFALSE;
}

// $Barnyard: FUNCTION 00674ce0
AParticleEffectPool::~AParticleEffectPool()
{
	TFree( m_pInstances );
}

// $Barnyard: FUNCTION 00674cf0
void AParticleEffectPool::Load( AParticleSystem* a_pSystem )
{
	m_pLibrary = a_pSystem->CastSymbol<AParticleEffectLibrary>( "EffectManager" );

	for ( TINT i = 0; i < m_pLibrary->iNumSettings; i++ )
		m_pLibrary->ppSettings[ i ]->Resolve();

	for ( TINT i = 0; i < m_pLibrary->iNumEffects; i++ )
		m_pLibrary->ppEffects[ i ]->Resolve();

	// Link the free instances
	for ( TINT i = 0; i < m_iMaxInstances - 1; i++ )
		m_pInstances[ i ].m_pNext = &m_pInstances[ i + 1 ];

	for ( TINT i = 1; i < m_iMaxInstances; i++ )
		m_pInstances[ i ].m_pPrev = &m_pInstances[ i - 1 ];

	m_pInstances[ 0 ].m_pPrev                   = TNULL;
	m_pInstances[ m_iMaxInstances - 1 ].m_pNext = TNULL;

	m_pUsedHead      = TNULL;
	m_pUsedTail      = TNULL;
	m_pFreeInstances = m_pInstances;
}

// $Barnyard: FUNCTION 00674de0
void AParticleEffectPool::Unload()
{
	m_pLibrary       = TNULL;
	m_pFreeInstances = TNULL;
	m_pUsedHead      = TNULL;
}

// $Barnyard: FUNCTION 00674df0
AParticleEffectInstance* AParticleEffectPool::CreateInstance( AParticleEffect* a_pEffect )
{
	AParticleEffectInstance* pInstance = m_pFreeInstances;

	if ( !pInstance )
		return TNULL;

	m_pFreeInstances = pInstance->m_pNext;
	pInstance->Start( a_pEffect );

	if ( m_pUsedTail )
	{
		pInstance->m_pPrev   = m_pUsedTail;
		m_pUsedTail->m_pNext = pInstance;
		pInstance->m_pNext   = TNULL;
	}
	else
	{
		pInstance->m_pPrev = TNULL;
		pInstance->m_pNext = TNULL;
		m_pUsedHead        = pInstance;
	}

	m_pUsedTail     = pInstance;
	m_bHasStealable = m_bHasStealable || a_pEffect->iStealable != 0;

	return pInstance;
}

// $Barnyard: FUNCTION 00674e80
void AParticleEffectPool::ReleaseInstance( AParticleEffectInstance* a_pInstance )
{
	if ( a_pInstance->m_pPrev )
		a_pInstance->m_pPrev->m_pNext = a_pInstance->m_pNext;

	if ( a_pInstance->m_pNext )
		a_pInstance->m_pNext->m_pPrev = a_pInstance->m_pPrev;

	if ( m_pUsedHead == a_pInstance )
		m_pUsedHead = a_pInstance->m_pNext;

	if ( m_pUsedTail == a_pInstance )
		m_pUsedTail = a_pInstance->m_pPrev;

	a_pInstance->m_pNext = m_pFreeInstances;

	if ( m_pFreeInstances )
		m_pFreeInstances->m_pPrev = a_pInstance;

	m_pFreeInstances     = a_pInstance;
	a_pInstance->m_pPrev = TNULL;
}

// $Barnyard: FUNCTION 00674f00
void AParticleEffectPool::Update( TFLOAT a_fDeltaTime )
{
	AParticleEffectInstance* pInstance = m_pUsedHead;

	while ( pInstance )
	{
		AParticleEffectInstance* pNext = pInstance->m_pNext;

		if ( !pInstance->Update( a_fDeltaTime ) )
			ReleaseInstance( pInstance );

		pInstance = pNext;
	}
}

// $Barnyard: FUNCTION 00675040
void AParticleEffectPool::KillAll()
{
	for ( AParticleEffectInstance* pInstance = m_pUsedHead; pInstance; )
	{
		AParticleEffectInstance* pNext = pInstance->m_pNext;
		pInstance->Kill();
		pInstance = pNext;
	}

	for ( AParticleEffectInstance* pInstance = m_pUsedHead; pInstance; )
	{
		AParticleEffectInstance* pNext = pInstance->m_pNext;

		if ( !pInstance->Update( 0.0f ) )
			ReleaseInstance( pInstance );

		pInstance = pNext;
	}
}

// $Barnyard: FUNCTION 006750a0
void AParticleEffectPool::KillEffect( AParticleEffect* a_pEffect )
{
	for ( AParticleEffectInstance* pInstance = m_pUsedHead; pInstance; )
	{
		AParticleEffectInstance* pNext = pInstance->m_pNext;

		if ( pInstance->m_pEffect == a_pEffect )
			pInstance->Kill();

		pInstance = pNext;
	}

	for ( AParticleEffectInstance* pInstance = m_pUsedHead; pInstance; )
	{
		AParticleEffectInstance* pNext = pInstance->m_pNext;

		if ( !pInstance->Update( 0.0f ) )
			ReleaseInstance( pInstance );

		pInstance = pNext;
	}
}

// $Barnyard: FUNCTION 00675100
void AParticleEffectPool::StealInstance()
{
	if ( !m_bHasStealable )
		return;

	for ( AParticleEffectInstance* pInstance = m_pUsedHead; pInstance; pInstance = pInstance->m_pNext )
	{
		if ( pInstance->m_pEffect->iStealable != 0 )
		{
			pInstance->Kill();
			ReleaseInstance( pInstance );
			return;
		}
	}

	m_bHasStealable = TFALSE;
}

// $Barnyard: FUNCTION 00674f40
TINT AParticleEffectPool::FindEffect( const TCHAR* a_szName ) const
{
	for ( TINT i = 0; i < m_pLibrary->iNumEffects; i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_pLibrary->ppEffects[ i ]->szName, a_szName ) == 0 )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00674f90
AParticleEffect* AParticleEffectPool::GetEffect( TINT a_iEffect ) const
{
	if ( a_iEffect < 0 || a_iEffect >= m_pLibrary->iNumEffects )
		return TNULL;

	return m_pLibrary->ppEffects[ a_iEffect ];
}

//-----------------------------------------------------------------------------
// AParticleEffectInstance
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 006749f0
AParticleEffectInstance::AParticleEffectInstance()
{
	m_pEffect = TNULL;
	m_mTransform.Identity();
}

// $Barnyard: FUNCTION 00674a10
void AParticleEffectInstance::Start( AParticleEffect* a_pEffect )
{
	m_fnOnFinished = TNULL;
	m_mTransform.Identity();
	m_pEffect = a_pEffect;

	for ( TINT i = 0; i < a_pEffect->iNumEmitters; i++ )
	{
		m_aEmitters[ i ].pEmitter = TNULL;
		m_aEmitters[ i ].eFlags   = EMITTERFLAGS_NONE;
		m_aEmitters[ i ].oTimeline.Reset();
	}
}

// $Barnyard: FUNCTION 00674a70
TBOOL AParticleEffectInstance::Update( TFLOAT a_fDeltaTime )
{
	ms_pCurrentInstance = this;

	TBOOL bAlive = TFALSE;

	for ( TINT i = 0; i < m_pEffect->iNumEmitters; i++ )
	{
		Emitter& rEmitter = m_aEmitters[ i ];

		if ( HASANYFLAG( rEmitter.eFlags, EMITTERFLAGS_FINISHED ) )
			continue;

		if ( HASANYFLAG( rEmitter.eFlags, EMITTERFLAGS_STOPPING ) )
		{
			// Waits for the particles of the emitter to die
			bAlive = TTRUE;
			continue;
		}

		ms_pCurrentEffectEmitter = &m_pEffect->pEmitters[ i ];
		ms_mEmitterTransform.Multiply( m_mTransform, *ms_pCurrentEffectEmitter->pOffset );

		if ( ms_pCurrentEffectEmitter->pTimeline->Update( a_fDeltaTime, &rEmitter.oTimeline ) == TTRUE )
		{
			bAlive = TTRUE;
		}
		else
		{
			// The timeline is over, let the emitter finish
			if ( rEmitter.pEmitter )
			{
				rEmitter.pEmitter->SetFlags( rEmitter.pEmitter->GetFlags() & ~( AParticleGenericEmitter::FLAGS_KEEP_ALIVE | AParticleGenericEmitter::FLAGS_SPAWN_NEW ) );

				AParticleGenericEmitter* pEmitter = rEmitter.pEmitter;
				rEmitter.pEmitter                 = TNULL;

				if ( pEmitter->GetNumParticles() <= 0 )
					AParticleSystem::GetSingleton()->ReleaseEmitter( pEmitter );
			}

			rEmitter.eFlags |= EMITTERFLAGS_FINISHED;
		}
	}

	ms_pCurrentInstance = TNULL;

	if ( !bAlive && m_fnOnFinished )
		m_fnOnFinished( this, m_pOnFinishedData );

	return bAlive;
}

// $Barnyard: FUNCTION 00674b80
void AParticleEffectInstance::StopLooping()
{
	for ( TINT i = 0; i < m_pEffect->iNumEmitters; i++ )
		m_aEmitters[ i ].oTimeline.iLoop = 0;
}

// $Barnyard: FUNCTION 00674bb0
void AParticleEffectInstance::Stop()
{
	for ( TINT i = 0; i < m_pEffect->iNumEmitters; i++ )
	{
		Emitter& rEmitter = m_aEmitters[ i ];

		if ( rEmitter.pEmitter )
		{
			rEmitter.pEmitter->SetFlags( rEmitter.pEmitter->GetFlags() & ~( AParticleGenericEmitter::FLAGS_KEEP_ALIVE | AParticleGenericEmitter::FLAGS_SPAWN_NEW ) );

			if ( rEmitter.pEmitter->GetNumParticles() == 0 )
			{
				AParticleGenericEmitter* pEmitter = rEmitter.pEmitter;
				rEmitter.pEmitter                 = TNULL;

				AParticleSystem::GetSingleton()->ReleaseEmitter( pEmitter );
				rEmitter.eFlags |= EMITTERFLAGS_FINISHED;
			}

			rEmitter.eFlags |= EMITTERFLAGS_STOPPING;
		}
		else
		{
			rEmitter.eFlags |= EMITTERFLAGS_FINISHED;
		}
	}
}

// $Barnyard: FUNCTION 00674c20
void AParticleEffectInstance::Kill()
{
	for ( TINT i = 0; i < m_pEffect->iNumEmitters; i++ )
	{
		Emitter& rEmitter = m_aEmitters[ i ];

		if ( rEmitter.pEmitter )
		{
			AParticleGenericEmitter* pEmitter = rEmitter.pEmitter;
			rEmitter.pEmitter                 = TNULL;

			AParticleSystem::GetSingleton()->ReleaseEmitter( pEmitter );
		}

		rEmitter.eFlags |= EMITTERFLAGS_FINISHED;
	}

	if ( m_fnOnFinished )
		m_fnOnFinished( this, m_pOnFinishedData );

	AParticleSystem::GetSingleton()->GetEffectPool()->ReleaseInstance( this );
}

// $Barnyard: FUNCTION 00674c90
void AParticleEffectInstance::OnEmitterFinished( AParticleGenericEmitter* a_pEmitter, void* a_pUserData )
{
	AParticleEffectInstance* pInstance = TSTATICCAST( AParticleEffectInstance, a_pUserData );

	for ( TINT i = 0; i < pInstance->m_pEffect->iNumEmitters; i++ )
	{
		Emitter& rEmitter = pInstance->m_aEmitters[ i ];

		if ( a_pEmitter == rEmitter.pEmitter )
		{
			rEmitter.pEmitter = TNULL;
			rEmitter.eFlags   = ( rEmitter.eFlags & ~EMITTERFLAGS_STOPPING ) | EMITTERFLAGS_FINISHED;
		}
	}
}

//-----------------------------------------------------------------------------
// AParticleEffect
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00674920
AParticleEffect* AParticleEffect::Clone() const
{
	AParticleEffect* pEffect = TSTATICCAST( AParticleEffect, TMalloc( sizeof( AParticleEffect ) ) );

	pEffect->szName       = szName;
	pEffect->iNumEmitters = iNumEmitters;
	pEffect->iStealable   = iStealable;
	pEffect->uiUnk        = uiUnk;

	// NOTE: the original allocates the array with the count in front like new[] would
	TINT* pArray = TSTATICCAST( TINT, TMalloc( iNumEmitters * sizeof( AParticleEffectEmitter ) + sizeof( TINT ) ) );

	if ( pArray )
	{
		*pArray            = iNumEmitters;
		pEffect->pEmitters = TREINTERPRETCAST( AParticleEffectEmitter*, pArray + 1 );
	}
	else
	{
		pEffect->pEmitters = TNULL;
	}

	for ( TINT i = 0; i < iNumEmitters; i++ )
		new ( &pEffect->pEmitters[ i ] ) AParticleEffectEmitter( pEmitters[ i ] );

	pEffect->Resolve();
	return pEffect;
}

// $Barnyard: FUNCTION 00674870
void AParticleEffect::Resolve()
{
	for ( TINT16 i = 0; i < iNumEmitters; i++ )
	{
		AParticleTimeline* pTimeline = pEmitters[ i ].pTimeline;
		pTimeline->Resolve();

		// The emitter keys know the previous key to blend the number of particles
		for ( TINT j = 1; j < pTimeline->iNumKeyframes; j++ )
			TREINTERPRETCAST( AParticleEmitterKeyframeData*, pTimeline->ppKeyframes[ j ]->pData )->pPrevious = pTimeline->ppKeyframes[ j - 1 ];
	}
}

// $Barnyard: FUNCTION 006749b0
void AParticleEffect::DeleteClone()
{
	AParticleSystem::GetSingleton()->GetEffectPool()->KillEffect( this );

	if ( pEmitters )
	{
		TINT* pArray = TREINTERPRETCAST( TINT*, pEmitters ) - 1;

		for ( TINT i = 0; i < *pArray; i++ )
			pEmitters[ i ].~AParticleEffectEmitter();

		TFree( pArray );
	}
}

//-----------------------------------------------------------------------------
// AParticleEffectEmitter
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00674780
AParticleEffectEmitter::AParticleEffectEmitter( const AParticleEffectEmitter& a_rcEmitter )
{
	pOffset = TSTATICCAST( TMatrix44, TMalloc( sizeof( TMatrix44 ) ) );
	TUtil::MemCopy( pOffset, a_rcEmitter.pOffset, sizeof( TMatrix44 ) );

	pSettings = new AParticleEmitterSettings( *a_rcEmitter.pSettings );
	pTimeline = new AParticleTimeline( *a_rcEmitter.pTimeline );
	uiFlags   = a_rcEmitter.uiFlags;

	for ( TINT i = 0; i < pTimeline->iNumKeyframes; i++ )
		TREINTERPRETCAST( AParticleEmitterKeyframeData*, pTimeline->ppKeyframes[ i ]->pData )->pSettings = pSettings;
}

// $Barnyard: FUNCTION 006748d0
AParticleEffectEmitter::~AParticleEffectEmitter()
{
	AParticleSystem::GetSingleton()->ReleaseEmittersOfSettings( pSettings );
	TFree( pOffset );

	if ( pSettings )
		delete pSettings;

	if ( pTimeline )
		delete pTimeline;
}

//-----------------------------------------------------------------------------
// AParticleEmitterSettings
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00672e20
AParticleEmitterSettings::AParticleEmitterSettings( const AParticleEmitterSettings& a_rcSettings )
{
	uiUnk          = a_rcSettings.uiUnk;
	szMaterialName = a_rcSettings.szMaterialName;
	szModelName    = a_rcSettings.szModelName;
	iMaxParticles  = a_rcSettings.iMaxParticles;
	uiUnk3         = a_rcSettings.uiUnk3;
	fLODDistance   = a_rcSettings.fLODDistance;
	iMinParticles  = a_rcSettings.iMinParticles;
	bComputeCentre = a_rcSettings.bComputeCentre;
	bComputeRadius = a_rcSettings.bComputeRadius;
	fRadius        = a_rcSettings.fRadius;
	iMaterial      = a_rcSettings.iMaterial;
	iModel         = a_rcSettings.iModel;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original doesn't copy these
	fLightScale = a_rcSettings.fLightScale;
	uiUnk4      = a_rcSettings.uiUnk4;
	uiUnk5      = a_rcSettings.uiUnk5;
#endif

	uiFlags      = a_rcSettings.uiFlags;
	uiAttributes = a_rcSettings.uiAttributes;
	eRenderType  = a_rcSettings.eRenderType;

	// Copy the affectors
	pAffectors                   = TNULL;
	AParticleAffector* pPrevious = TNULL;

	for ( AParticleAffector* pAffector = a_rcSettings.pAffectors; pAffector; pAffector = pAffector->pNext )
	{
		AParticleAffector* pCopy = new AParticleAffector( *pAffector );
		pCopy->pNext             = TNULL;

		if ( pPrevious )
			pPrevious->pNext = pCopy;
		else
			pAffectors = pCopy;

		pPrevious = pCopy;
	}

	// Copy the initialisers
	for ( TINT i = 0; i < INIT_NUMOF; i++ )
	{
		apInits[ i ] = TNULL;

		if ( a_rcSettings.apInits[ i ] )
			apInits[ i ] = new AParticleInit( *a_rcSettings.apInits[ i ] );
	}

	apUVs[ 0 ] = new UVData( *a_rcSettings.apUVs[ 0 ] );
	apUVs[ 1 ] = new UVData( *a_rcSettings.apUVs[ 1 ] );

	pRotationData = TNULL;
	pSparkData    = TNULL;
	pStretchData  = TNULL;
	pAttachData   = TNULL;

	if ( a_rcSettings.pRotationData )
	{
		pRotationData = TSTATICCAST( RotationData, TMalloc( sizeof( RotationData ) ) );
		TUtil::MemCopy( pRotationData, a_rcSettings.pRotationData, sizeof( RotationData ) );
	}

	if ( a_rcSettings.pSparkData )
	{
		pSparkData = TSTATICCAST( SparkData, TMalloc( sizeof( SparkData ) ) );
		TUtil::MemCopy( pSparkData, a_rcSettings.pSparkData, sizeof( SparkData ) );
	}

	if ( a_rcSettings.pStretchData )
	{
		pStretchData  = TSTATICCAST( StretchData, TMalloc( sizeof( StretchData ) ) );
		*pStretchData = *a_rcSettings.pStretchData;
	}

	if ( a_rcSettings.pAttachData )
	{
		pAttachData  = TSTATICCAST( AttachData, TMalloc( sizeof( AttachData ) ) );
		*pAttachData = *a_rcSettings.pAttachData;
	}

	// Copy the timelines
	for ( TINT i = 0; i < TIMELINE_NUMOF; i++ )
	{
		apTimelines[ i ] = TNULL;

		if ( a_rcSettings.apTimelines[ i ] )
			apTimelines[ i ] = new AParticleTimeline( *a_rcSettings.apTimelines[ i ] );
	}

	Resolve();
}

// $Barnyard: FUNCTION 00672b30
AParticleEmitterSettings::~AParticleEmitterSettings()
{
	while ( pAffectors )
	{
		AParticleAffector* pAffector = pAffectors;
		pAffectors                   = pAffector->pNext;

		if ( pAffector )
			delete pAffector;
	}

	for ( TINT i = 0; i < INIT_NUMOF; i++ )
	{
		if ( apInits[ i ] )
			delete apInits[ i ];
	}

	TFree( apUVs[ 0 ] );
	TFree( apUVs[ 1 ] );
	TFree( pRotationData );
	TFree( pSparkData );
	TFree( pStretchData );
	TFree( pAttachData );

	for ( TINT i = 0; i < TIMELINE_NUMOF; i++ )
	{
		if ( apTimelines[ i ] )
			delete apTimelines[ i ];
	}
}

// $Barnyard: FUNCTION 00672cd0
void AParticleEmitterSettings::Resolve()
{
	for ( TINT i = 0; i < INIT_NUMOF; i++ )
	{
		if ( apInits[ i ] )
			apInits[ i ]->Resolve();
	}

	for ( AParticleAffector* pAffector = pAffectors; pAffector; pAffector = pAffector->pNext )
		pAffector->Resolve();

	for ( TINT i = 0; i < TIMELINE_NUMOF; i++ )
	{
		if ( apTimelines[ i ] )
			apTimelines[ i ]->Resolve();
	}

	uiAttributes = AParticleGenericEmitter::FLAGS_POSITION | AParticleGenericEmitter::FLAGS_VELOCITY | AParticleGenericEmitter::FLAGS_LIFE | AParticleGenericEmitter::FLAGS_UNK5 | AParticleGenericEmitter::FLAGS_MASS | AParticleGenericEmitter::FLAGS_SIZE | AParticleGenericEmitter::FLAGS_UV | AParticleGenericEmitter::FLAGS_COLOUR;

	if ( pAttachData && pAttachData->uiStartAxis != 0 )
		uiAttributes |= AParticleGenericEmitter::FLAGS_START_AXIS;

	if ( HASANYFLAG( uiFlags, FLAGS_ROTATE ) )
	{
		if ( pRotationData )
			pRotationData->RandomiseSpeeds();

		if ( pSparkData )
			pSparkData->RandomiseSpeeds();
	}

	AParticleSystem* pSystem = AParticleSystem::GetSingleton();
	iMaterial                = pSystem->GetMaterialLibrary()->FindMaterial( szMaterialName );

	if ( szModelName )
	{
		iModel    = pSystem->GetModelLibrary()->FindModel( szModelName );
		iMaterial = -1;
	}
}

// $Barnyard: FUNCTION 00672970
void AParticleEmitterSettings::RotationData::RandomiseSpeeds()
{
	TRandom oRandom;

	for ( TINT i = 0; i < iNumRotations; i++ )
		afSpeeds[ i ] = TMath::LERP( fMinSpeed, fMaxSpeed, oRandom.GetFloat() );
}

// $Barnyard: FUNCTION 00672a00
void AParticleEmitterSettings::SparkData::RandomiseSpeeds()
{
	TRandom oRandom;

	for ( TINT i = 0; i < iNumRotations; i++ )
	{
		afSpeedsX[ i ] = TMath::LERP( fMinSpeedX, fMaxSpeedX, oRandom.GetFloat() );
		afSpeedsY[ i ] = TMath::LERP( fMinSpeedY, fMaxSpeedY, oRandom.GetFloat() );
		afSpeedsZ[ i ] = TMath::LERP( fMinSpeedZ, fMaxSpeedZ, oRandom.GetFloat() );
	}
}

// $Barnyard: FUNCTION 006779b0
AParticleEmitterSettings::UVData::UVData( const UVData& a_rcData )
{
	uiUnk = a_rcData.uiUnk;
	iUnk  = a_rcData.iUnk;
	TUtil::MemCopy( auiUnk, a_rcData.auiUnk, sizeof( auiUnk ) );
	iUnk2 = a_rcData.iUnk2;
	TUtil::MemCopy( auiUnk2, a_rcData.auiUnk2, sizeof( auiUnk2 ) );
}

//-----------------------------------------------------------------------------
// Particle functions data
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00675140
AParticleInit::AParticleInit( const AParticleInit& a_rcInit )
{
	fnInit = a_rcInit.fnInit;
	pData  = TSTATICCAST( AParticleFunctionData, TMalloc( a_rcInit.pData->uiSize ) );
	TUtil::MemCopy( pData, a_rcInit.pData, a_rcInit.pData->uiSize );
}

// $Barnyard: FUNCTION 00673800
AParticleAffector::AParticleAffector( const AParticleAffector& a_rcAffector )
{
	pData = TSTATICCAST( AParticleFunctionData, TMalloc( a_rcAffector.pData->uiSize ) );
	TUtil::MemCopy( pData, a_rcAffector.pData, a_rcAffector.pData->uiSize );
}

// $Barnyard: FUNCTION 006767d0
AParticleKeyframe::AParticleKeyframe( const AParticleKeyframe& a_rcKeyframe )
{
	pData = TSTATICCAST( AParticleKeyframeData, TMalloc( a_rcKeyframe.pData->uiSize ) );
	TUtil::MemCopy( pData, a_rcKeyframe.pData, a_rcKeyframe.pData->uiSize );
}

// $Barnyard: FUNCTION 00676820
TFLOAT AParticleKeyframe::GetProgress( TFLOAT a_fProgress )
{
	switch ( pData->iCurve )
	{
		case AParticleKeyframeData::CURVE_STEP:
			return TFLOAT( TINT( a_fProgress ) );
		case AParticleKeyframeData::CURVE_LINEAR:
			return a_fProgress;
		case AParticleKeyframeData::CURVE_EASE_IN:
			return a_fProgress * a_fProgress;
		case AParticleKeyframeData::CURVE_EASE_OUT:
			return 1.0f - ( 1.0f - a_fProgress ) * ( 1.0f - a_fProgress );
		case AParticleKeyframeData::CURVE_EASE_IN_CUBIC:
			return a_fProgress * a_fProgress * a_fProgress;
		case AParticleKeyframeData::CURVE_EASE_OUT_CUBIC:
			return 1.0f - ( 1.0f - a_fProgress ) * ( ( 1.0f - a_fProgress ) * ( 1.0f - a_fProgress ) );
		case AParticleKeyframeData::CURVE_EASE_IN_OUT:
			return TMath::EaseInOut( a_fProgress );
		case AParticleKeyframeData::CURVE_GAIN:
			return TMath::Gain( 0.25f, a_fProgress );
		default:
			return 0.0f;
	}
}

// $Barnyard: FUNCTION 00676fc0
AParticleTimeline::AParticleTimeline( const AParticleTimeline& a_rcTimeline )
{
	iNumKeyframes = a_rcTimeline.iNumKeyframes;

	if ( iNumKeyframes <= 0 )
		ppKeyframes = TNULL;
	else
		ppKeyframes = TSTATICCAST( AParticleKeyframe*, TMalloc( iNumKeyframes * sizeof( AParticleKeyframe* ) ) );

	for ( TINT i = 0; i < iNumKeyframes; i++ )
		ppKeyframes[ i ] = new AParticleKeyframe( *a_rcTimeline.ppKeyframes[ i ] );
}

// $Barnyard: FUNCTION 00677170
AParticleTimeline::~AParticleTimeline()
{
	for ( TINT i = 0; i < iNumKeyframes; i++ )
	{
		if ( ppKeyframes[ i ] )
			delete ppKeyframes[ i ];
	}

	TFree( ppKeyframes );
}

// $Barnyard: FUNCTION 00677130
void AParticleTimeline::Resolve()
{
	for ( TINT i = 0; i < iNumKeyframes; i++ )
	{
		ppKeyframes[ i ]->Resolve();

		if ( i > 0 )
			ppKeyframes[ i ]->pPrevious = ppKeyframes[ i - 1 ];
	}
}

// $Barnyard: FUNCTION 00677070
void AParticleTimelineState::Reset()
{
	iKeyframe = -1;
	iLoop     = 1;
	pKeyframe = TNULL;
	pPrevious = TNULL;
	fTime     = 0.0f;
}

// $Barnyard: FUNCTION 00677090
AParticleKeyframe* AParticleTimeline::GetNextKeyframe( AParticleTimelineState* a_pState )
{
	if ( iNumKeyframes == 0 )
	{
		a_pState->iKeyframe = -1;
		return TNULL;
	}

	if ( a_pState->iLoop > 0 && a_pState->iKeyframe >= 0 )
	{
		AParticleKeyframeData* pData = ppKeyframes[ a_pState->iKeyframe ]->pData;

		if ( pData->iLoopType == AParticleKeyframe::LOOPTYPE_END )
		{
			// Go back to the start of the loop
			TINT16 iLoopStart = 0;

			while ( ppKeyframes[ iLoopStart ]->pData->iLoopType != AParticleKeyframe::LOOPTYPE_START )
				iLoopStart++;

			a_pState->fTime     = a_pState->fTime - pData->fTime + ppKeyframes[ iLoopStart ]->pData->fTime;
			a_pState->iKeyframe = iLoopStart;
			return ppKeyframes[ iLoopStart ];
		}
	}

	TINT iNext = ++a_pState->iKeyframe;

	if ( iNext < iNumKeyframes )
		return ppKeyframes[ iNext ];

	a_pState->iKeyframe = -1;
	return TNULL;
}

// $Barnyard: FUNCTION 006771b0
TBOOL AParticleTimeline::Update( TFLOAT a_fDeltaTime, AParticleTimelineState* a_pState )
{
	if ( !a_pState->pKeyframe )
	{
		a_pState->pKeyframe = GetNextKeyframe( a_pState );

		if ( !a_pState->pKeyframe )
			return TFALSE;

		a_pState->pKeyframe->fnOnStart( a_pState->pKeyframe, TNULL );
	}

	a_pState->fTime += a_fDeltaTime;

	AParticleKeyframe* pKeyframe = a_pState->pKeyframe;
	TFLOAT             fTime     = a_pState->fTime;
	TFLOAT             fLength   = pKeyframe->pData->fTime;

	if ( a_pState->pPrevious )
	{
		fTime -= a_pState->pPrevious->pData->fTime;
		fLength -= a_pState->pPrevious->pData->fTime;
	}

	TFLOAT fProgress = pKeyframe->GetProgress( fTime / fLength );
	pKeyframe->fnUpdate( pKeyframe, a_fDeltaTime, fProgress, a_pState );

	if ( a_pState->fTime > pKeyframe->pData->fTime )
	{
		AParticleKeyframe* pNext = GetNextKeyframe( a_pState );

		if ( !pNext )
			return TFALSE;

		AParticleKeyframe* pPrevious = a_pState->pKeyframe;
		a_pState->pPrevious          = pPrevious;
		a_pState->pKeyframe          = pNext;
		pNext->fnOnStart( pNext, pPrevious );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00677330
void AParticleTimeline::ApplyByAge( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fAge = a_pEmitter->m_aAges[ i ];

		// Find the first key the particle hasn't reached yet
		TINT iKeyframe = 0;

		if ( iNumKeyframes <= 0 )
			continue;

		while ( fAge >= ppKeyframes[ iKeyframe ]->pData->fTime )
		{
			if ( ++iKeyframe >= iNumKeyframes )
				break;
		}

		if ( iKeyframe >= iNumKeyframes )
			continue;

		AParticleKeyframe* pKeyframe = ppKeyframes[ iKeyframe ];
		TFLOAT             fLength   = pKeyframe->pData->fTime;
		TFLOAT             fTime;

		if ( iKeyframe <= 0 )
		{
			fTime = fAge;
		}
		else
		{
			AParticleKeyframeData* pPrevious = ppKeyframes[ iKeyframe - 1 ]->pData;
			fTime                            = fAge - pPrevious->fTime;
			fLength                          = fLength - pPrevious->fTime;
		}

		TFLOAT fProgress = pKeyframe->GetProgress( fTime / fLength );
		ppKeyframes[ iKeyframe ]->fnApply( ppKeyframes[ iKeyframe ], a_pEmitter, fProgress, i );
	}
}

// $Barnyard: FUNCTION 00677260
void AParticleTimeline::ApplyEvents( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TUINT8 uiKeyframe = a_pEmitter->m_aEventKeys[ i ];

		if ( uiKeyframe < iNumKeyframes )
		{
			AParticleKeyframe* pKeyframe = ppKeyframes[ uiKeyframe ];

			if ( a_pEmitter->m_aAges[ i ] > pKeyframe->pData->fTime )
			{
				pKeyframe->fnApply( pKeyframe, a_pEmitter, 0.0f, i );
				a_pEmitter->m_aEventKeys[ i ] = uiKeyframe + 1;
			}
		}
	}
}
