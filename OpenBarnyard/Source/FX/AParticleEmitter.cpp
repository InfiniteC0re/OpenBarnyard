#include "pch.h"
#include "AParticleSystem.h"

#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>
#include <Toshi/TSceneObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>
#include <Render/TModelManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Corners of the sprites, either fixed or facing the camera
static TVector4 s_vecSpriteCorner0; // 0x007cd848
static TVector4 s_vecSpriteCorner1; // 0x007cd424
static TVector4 s_vecSpriteCorner2; // 0x007cd858
static TVector4 s_vecSpriteCorner3; // 0x007cd434

// Corners of the rotating sprites, every particle uses one of them
static TVector4 s_aRotatedCorners[ 16 ][ 4 ]; // 0x007cd448
static TINT     s_iNumRotations;              // 0x00779a18, rounded up to a power of two

// The direction the ribbons face and the side of the current ribbon segment
static TVector4 s_vecRibbonUp;   // 0x007cd868
static TFLOAT   s_fRibbonMinAge; // 0x007cd418, never used
static TFLOAT   s_fRibbonMaxAge; // 0x007cd41c, never used

// The light colour (0-255) of the lit particles
static TINT s_iLightR; // 0x00779a1c
static TINT s_iLightG; // 0x00779a20
static TINT s_iLightB; // 0x00779a24

// UV settings of the emitter being rendered, never used
static TINT s_iUVUnk;  // 0x007cd410
static TINT s_iUVUnk2; // 0x007cd40c
static TINT s_iUVUnk3; // 0x007cd408
static TINT s_iUVUnk4; // 0x007cd414

// Used to pick the start angles of the rotating sprites
static TRandom s_oRotationRandom; // 0x007cde48

//-----------------------------------------------------------------------------
// AParticleEmitter
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00677a00
AParticleEmitter::AParticleEmitter()
{
	m_vecBoundingSphere = TVector4::VEC_ZERO;
	m_iMaterial         = -1;
	m_iMaxParticles     = 0;
	m_iNumParticles     = 0;
	m_fLODScale         = 1.0f / ( 300.0f * 300.0f );
	m_iMinParticles     = 0;
}

// $Barnyard: FUNCTION 00677a50
// $Barnyard: FUNCTION 00677cd0
AParticleEmitter::~AParticleEmitter()
{
}

// $Barnyard: FUNCTION 00677a80
void AParticleEmitter::Release()
{
	SetMaxParticles( 0 );
}

// $Barnyard: FUNCTION 00679c90
TINT AParticleEmitter::GetNumVertices( TINT a_iNumParticles )
{
	return 4 * a_iNumParticles;
}

void AParticleEmitter::SetNumParticles( TINT a_iNumParticles )
{
	// NOTE: the original shares this function with other classes (00566c60)
	m_iNumParticles = a_iNumParticles;
}

// $Barnyard: FUNCTION 00677a60
void AParticleEmitter::SetMaxParticles( TUINT16 a_uiMaxParticles )
{
	m_iMaxParticles = a_uiMaxParticles;

	if ( m_iNumParticles > a_uiMaxParticles )
		m_iNumParticles = a_uiMaxParticles;
}

// $Barnyard: FUNCTION 00677cf0
TBOOL AParticleEmitter::IsVisible()
{
	return AParticleSystem::IsSphereVisible( m_vecBoundingSphere, m_vecBoundingSphere.w );
}

// $Barnyard: FUNCTION 00677b00
void AParticleEmitter::RenderQuads( AParticleVertexFunc a_fnPositions, AParticleVertexFunc a_fnUVs, AParticleVertexFunc a_fnColours, AParticleIndexFunc a_fnIndices, AParticleEmitter* a_pEmitter )
{
	if ( m_iNumParticles <= 0 )
		return;

	TINT  iParticle     = 0;
	TBOOL bDone         = TFALSE;
	TINT  iVerticesLeft = m_iNumParticles * 4;

	do
	{
		AParticleMeshLock* pLock = AParticleSystem::GetRenderSystem()->GetMeshLock( iVerticesLeft, GetNumVertices( 1 ), m_iMaterial );

		if ( !pLock )
			break;

		TINT iNumParticles = TUINT( pLock->pMesh->GetMaxVertices() - pLock->uiNumVertices ) / TUINT( GetNumVertices( 1 ) );

		if ( iNumParticles == 0 )
			break;

		if ( iNumParticles + iParticle >= m_iNumParticles )
		{
			iNumParticles = m_iNumParticles - iParticle;
			bDone         = TTRUE;
		}

		iVerticesLeft -= GetNumVertices( iNumParticles );

		for ( TINT i = 0; i < iNumParticles; i++, iParticle++ )
		{
			a_fnColours( pLock, i * 4, iParticle, a_pEmitter );
			a_fnUVs( pLock, i * 4, iParticle, a_pEmitter );
			a_fnPositions( pLock, i * 4, iParticle, a_pEmitter );
		}

		TINT iNumVertices = GetNumVertices( iNumParticles );
		pLock->Advance( iNumVertices, a_fnIndices( pLock, iNumParticles, a_pEmitter ) );

	} while ( !bDone );
}

// $Barnyard: FUNCTION 00677c20
TINT AParticleEmitter::WriteQuadIndices( AParticleMeshLock* a_pLock, TINT a_iNumParticles, AParticleEmitter* a_pEmitter )
{
	TUINT16 uiIndex  = 0;
	TUINT16 uiVertex = 0;

	for ( TINT i = 0; i < a_iNumParticles; i++ )
	{
		// The quads are a strip with degenerate triangles between them
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + 1 + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + 2 + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + 3 + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + 3 + a_pLock->uiNumVertices;
		uiVertex += 4;
	}

	return 6 * a_iNumParticles;
}

//-----------------------------------------------------------------------------
// AParticleTestEmitter
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00679c70
AParticleTestEmitter::AParticleTestEmitter()
{
}

// $Barnyard: FUNCTION 00679ca0
// $Barnyard: FUNCTION 00677f00
AParticleTestEmitter::~AParticleTestEmitter()
{
}

// $Barnyard: FUNCTION 00679cb0
void AParticleTestEmitter::Start()
{
	m_fTime = 0.0f;
}

// $Barnyard: FUNCTION 00679cc0
void AParticleTestEmitter::Release()
{
	AParticleEmitter::Release();
}

TINT AParticleTestEmitter::GetNumVertices( TINT a_iNumParticles )
{
	return AParticleEmitter::GetNumVertices( a_iNumParticles );
}

// $Barnyard: FUNCTION 00679cd0
TBOOL AParticleTestEmitter::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;
	return m_fTime <= 3.0f;
}

// $Barnyard: FUNCTION 00679d00
void AParticleTestEmitter::Render()
{
	RenderQuads( SetPositions, SetUVs, SetColours, WriteQuadIndices, this );
}

// $Barnyard: FUNCTION 00679ab0
void AParticleTestEmitter::SetPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	const TVector4* pCorners = AParticleSystem::GetQuadCorners();

	TVector4 vecCorner0;
	TVector4 vecCorner2;
	TVector4 vecCorner1;
	TVector4 vecCorner3;
	vecCorner0.Multiply( pCorners[ 0 ], 3.0f );
	vecCorner2.Multiply( pCorners[ 2 ], 3.0f );
	vecCorner1.Multiply( pCorners[ 1 ], 3.0f );
	vecCorner3.Multiply( pCorners[ 3 ], 3.0f );

	const TVector4&   rcCentre  = a_pEmitter->m_vecBoundingSphere;
	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;

	pVertices[ 0 ].vPosition.Set( vecCorner0.x + rcCentre.x, vecCorner0.y + rcCentre.y, vecCorner0.z + rcCentre.z );
	pVertices[ 1 ].vPosition.Set( vecCorner2.x + rcCentre.x, vecCorner2.y + rcCentre.y, vecCorner2.z + rcCentre.z );
	pVertices[ 2 ].vPosition.Set( vecCorner1.x + rcCentre.x, vecCorner1.y + rcCentre.y, vecCorner1.z + rcCentre.z );
	pVertices[ 3 ].vPosition.Set( vecCorner3.x + rcCentre.x, vecCorner3.y + rcCentre.y, vecCorner3.z + rcCentre.z );
}

// $Barnyard: FUNCTION 00679bd0
void AParticleTestEmitter::SetUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;

	pVertices[ 0 ].vUV.Set( 0.0f, 1.0f );
	pVertices[ 1 ].vUV.Set( 0.0f, 1.0f );
	pVertices[ 2 ].vUV.Set( 0.0f, 1.0f );
	pVertices[ 3 ].vUV.Set( 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 00679c30
void AParticleTestEmitter::SetColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;

	pVertices[ 0 ].uiColor = 0xffffffff;
	pVertices[ 1 ].uiColor = 0xffffffff;
	pVertices[ 2 ].uiColor = 0xffffffff;
	pVertices[ 3 ].uiColor = 0xffffffff;
}

//-----------------------------------------------------------------------------
// AParticleGenericEmitter
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00672650
AParticleGenericEmitter::AParticleGenericEmitter()
{
	m_fnOnFinished    = TNULL;
	m_pOnFinishedData = TNULL;
	m_eFlags          = FLAGS_NONE;
	m_pSettings       = TNULL;
	m_mTransform.Identity();
}

// $Barnyard: FUNCTION 00672860
// $Barnyard: FUNCTION 00677f50
AParticleGenericEmitter::~AParticleGenericEmitter()
{
}

// $Barnyard: FUNCTION 0066f380
void AParticleGenericEmitter::Setup( AParticleEmitterSettings* a_pSettings )
{
	SetMaxParticles( 0 );

	m_pSettings     = a_pSettings;
	m_eFlags        = a_pSettings->uiAttributes;
	m_iMaterial     = a_pSettings->iMaterial;
	m_fLODScale     = 1.0f / ( a_pSettings->fLODDistance * a_pSettings->fLODDistance );
	m_iMinParticles = a_pSettings->iMinParticles;

	if ( !a_pSettings->bComputeRadius )
		m_vecBoundingSphere.w = a_pSettings->fRadius;
}

// $Barnyard: FUNCTION 0066f3d0
void AParticleGenericEmitter::SetTransform( const TMatrix44& a_rcTransform )
{
	m_mTransform = a_rcTransform;

	if ( !m_pSettings->bComputeCentre )
	{
		m_vecBoundingSphere.x = m_mTransform.m_f41;
		m_vecBoundingSphere.y = m_mTransform.m_f42;
		m_vecBoundingSphere.z = m_mTransform.m_f43;
	}
}

// $Barnyard: FUNCTION 006717e0
void AParticleGenericEmitter::Start()
{
	m_iNumParticles = 0;
	m_fTime         = 0.0f;

	TMatrix44::MatToQuat( m_quatPrevRotation, m_mTransform );
	m_vecPrevPosition = m_mTransform.GetTranslation();

	if ( HASANYFLAG( m_pSettings->uiFlags, AParticleEmitterSettings::FLAGS_ROTATE ) )
		RandomiseRotations();

	AParticleEmitterSettings* pSettings = m_pSettings;

	if ( pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_MESH )
	{
		m_fnSetPositions = TNULL;
		m_fnSetUVs       = TNULL;
		m_fnSetColours   = TNULL;
		m_fnWriteIndices = TNULL;
		m_fnGetSize      = TNULL;
	}
	else
	{
		m_fnWriteIndices = WriteQuadIndices;

		// Positions
		switch ( pSettings->eRenderType )
		{
			case AParticleEmitterSettings::RENDERTYPE_SPRITE:
				if ( HASANYFLAG( pSettings->uiFlags, AParticleEmitterSettings::FLAGS_ROTATE ) )
					m_fnSetPositions = SetRotatedSpritePositions;
				else
					m_fnSetPositions = SetSpritePositions;
				break;
			case AParticleEmitterSettings::RENDERTYPE_SPARK:
				m_fnSetPositions = SetRotatedSpritePositions;
				break;
			case AParticleEmitterSettings::RENDERTYPE_RIBBON:
				m_fnSetPositions = TNULL;
				m_fnWriteIndices = WriteRibbonIndices;
				break;
			default:
				m_fnSetPositions = SetStretchedPositions;
				break;
		}

		// Colours
		if ( pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ] )
		{
			if ( HASANYFLAG( pSettings->uiFlags, AParticleEmitterSettings::FLAGS_LIGHTING ) )
				m_fnSetColours = SetLitKeyframeColours;
			else
				m_fnSetColours = SetKeyframeColours;
		}
		else
		{
			if ( HASANYFLAG( pSettings->uiFlags, AParticleEmitterSettings::FLAGS_LIGHTING ) )
				m_fnSetColours = SetLitColours;
			else
				m_fnSetColours = SetColours;
		}

		// UVs
		if ( !pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ] && pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_RIBBON )
			m_fnSetUVs = SetRibbonUVs;
		else if ( pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ] && pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_RIBBON )
			m_fnSetUVs = SetRibbonFrameUVs;
		else if ( pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_STRETCHED )
			m_fnSetUVs = SetStretchedFrameUVs;
		else
			m_fnSetUVs = SetFrameUVs;
	}

	// Sizes
	if ( pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ] )
		m_fnGetSize = GetKeyframeSize;
	else
		m_fnGetSize = GetSize;

	m_aTimelineStates[ 0 ].Reset();
	m_aTimelineStates[ 1 ].Reset();
	m_aTimelineStates[ 2 ].Reset();
	m_aTimelineStates[ 3 ].Reset();
}

// $Barnyard: FUNCTION 00672140
void AParticleGenericEmitter::Release()
{
	SetMaxParticles( 0 );

	m_aPositions.Resize( 0 );
	m_aVelocities.Resize( 0 );
	m_aStartAxes.Resize( 0 );
	m_aAges.Resize( 0 );
	m_aAgeRates.Resize( 0 );
	m_aMasses.Resize( 0 );
	m_aSizes.Resize( 0 );
	m_aUVFrames.Resize( 0 );
	m_aColours.Resize( 0 );
	m_aColourKeys.Resize( 0 );
	m_aSizeKeys.Resize( 0 );
	m_aEventKeys.Resize( 0 );

	m_eFlags    = FLAGS_NONE;
	m_pSettings = TNULL;
	m_iMaterial = -1;

	AParticleEmitter::Release();

	if ( m_fnOnFinished )
	{
		m_fnOnFinished( this, m_pOnFinishedData );
		m_fnOnFinished    = TNULL;
		m_pOnFinishedData = TNULL;
	}
}

// $Barnyard: FUNCTION 0066f410
TINT AParticleGenericEmitter::GetNumVertices( TINT a_iNumParticles )
{
	if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_RIBBON )
		return 2 * a_iNumParticles;
	else
		return 4 * a_iNumParticles;
}

// $Barnyard: FUNCTION 006719a0
void AParticleGenericEmitter::SetNumParticles( TINT a_iNumParticles )
{
	TINT iOldNumParticles = m_iNumParticles;
	m_iNumParticles       = a_iNumParticles;

	SpawnParticles( iOldNumParticles, a_iNumParticles, 0.0f, TFALSE );
}

// $Barnyard: FUNCTION 00672210
void AParticleGenericEmitter::SetMaxParticles( TUINT16 a_uiMaxParticles )
{
	if ( m_iMaxParticles == a_uiMaxParticles )
		return;

	TINT iOldMaxParticles = m_iMaxParticles;
	AParticleEmitter::SetMaxParticles( a_uiMaxParticles );

	// Take the pages for the attributes, use less particles if there are not enough pages
	if ( HASANYFLAG( m_eFlags, FLAGS_POSITION ) && !m_aPositions.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aPositions.GetCapacity() );

	if ( HASANYFLAG( m_eFlags, FLAGS_VELOCITY ) && !m_aVelocities.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aVelocities.GetCapacity() );

	if ( HASANYFLAG( m_eFlags, FLAGS_LIFE ) )
	{
		if ( !m_aAges.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aAges.GetCapacity() );

		if ( !m_aAgeRates.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aAgeRates.GetCapacity() );
	}

	if ( HASANYFLAG( m_eFlags, FLAGS_MASS ) && !m_aMasses.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aMasses.GetCapacity() );

	if ( HASANYFLAG( m_eFlags, FLAGS_SIZE ) )
	{
		if ( !m_aSizes.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aSizes.GetCapacity() );

		if ( m_pSettings && m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ] && !m_aSizeKeys.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aSizeKeys.GetCapacity() );
	}

	if ( HASANYFLAG( m_eFlags, FLAGS_UV ) && !m_aUVFrames.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aUVFrames.GetCapacity() );

	if ( HASANYFLAG( m_eFlags, FLAGS_COLOUR ) )
	{
		if ( !m_aColours.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aColours.GetCapacity() );

		if ( m_pSettings && m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ] && !m_aColourKeys.Resize( m_iMaxParticles ) )
			m_iMaxParticles = TUINT16( m_aColourKeys.GetCapacity() );
	}

	if ( HASANYFLAG( m_eFlags, FLAGS_START_AXIS ) && !m_aStartAxes.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aStartAxes.GetCapacity() );

	if ( m_iMaxParticles > 0 && m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_EVENTS ] && !m_aEventKeys.Resize( m_iMaxParticles ) )
		m_iMaxParticles = TUINT16( m_aEventKeys.GetCapacity() );

	if ( m_iNumParticles > m_iMaxParticles )
		m_iNumParticles = m_iMaxParticles;

	SpawnParticles( iOldMaxParticles, m_iMaxParticles, g_oSystemManager.GetScheduler()->GetCurrentDeltaTime(), TTRUE );
	m_iNumParticles = m_iMaxParticles;
}

// $Barnyard: FUNCTION 0066fdf0
void AParticleGenericEmitter::SpawnParticles( TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TBOOL a_bInitLife )
{
	if ( a_iStart >= a_iEnd )
		return;

	m_pSettings->InitParticles( this, a_iStart, a_iEnd, a_bInitLife );

	if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POS | FLAGS_FOLLOW_TRANS ) )
	{
		// The particles are stored relative to the emitter
		if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POS ) )
		{
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
			{
				TMatrix44::RotateVector( m_aPositions[ i ], m_mTransform, m_aPositions[ i ] );
				TMatrix44::RotateVector( m_aVelocities[ i ], m_mTransform, m_aVelocities[ i ] );
			}
		}
	}
	else
	{
		TFLOAT fInvNumParticles = 1.0f / ( a_iEnd - a_iStart );

		if ( a_iEnd - a_iStart <= 1 )
		{
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
			{
				TMatrix44::TransformVector( m_aPositions[ i ], m_mTransform, m_aPositions[ i ] );
				TMatrix44::RotateVector( m_aVelocities[ i ], m_mTransform, m_aVelocities[ i ] );
			}
		}
		else
		{
			// Spread the new particles between the previous and the current transform
			TQuaternion quatRotation;
			TMatrix44::MatToQuat( quatRotation, m_mTransform );

			for ( TINT i = a_iStart; i < a_iEnd; i++ )
			{
				TFLOAT fProgress = ( i + 1 - a_iStart ) * fInvNumParticles;

				TQuaternion quatParticle;
				TQuaternion::Nlerp( quatParticle, m_quatPrevRotation, quatRotation, fProgress );

				TMatrix44 mTransform;
				mTransform.SetFromQuaternion( quatParticle );
				mTransform.GetTranslation().Lerp4( m_vecPrevPosition, m_mTransform.GetTranslation(), fProgress );
				mTransform.m_f44 = 1.0f;

				TMatrix44::TransformVector( m_aPositions[ i ], mTransform, m_aPositions[ i ] );
				TMatrix44::RotateVector( m_aVelocities[ i ], mTransform, m_aVelocities[ i ] );
			}
		}

		if ( a_fDeltaTime > 0.0f )
		{
			// The new particles could have been spawned during the last frame, update them
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
			{
				TFLOAT fDeltaTime = TMath::LERP( 0.0f, a_fDeltaTime, ( i + ( 1 - a_iStart ) ) * fInvNumParticles );
				m_aAges[ i ] += fDeltaTime * m_aAgeRates[ i ];

				if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ] )
					m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ]->ApplyByAge( this, i, i + 1 );

				if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ] )
					m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ]->ApplyByAge( this, i, i + 1 );

				if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_MASS ] )
					m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_MASS ]->ApplyByAge( this, i, i + 1 );

				if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ] )
					m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ]->ApplyByAge( this, i, i + 1 );

				if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_EVENTS ] )
					m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_EVENTS ]->ApplyEvents( this, i, i + 1 );

				for ( AParticleAffector* pAffector = m_pSettings->pAffectors; pAffector; pAffector = pAffector->pNext )
					pAffector->fnAffector( this, i, i + 1, fDeltaTime, pAffector->pData );
			}
		}
	}

	// Store the axis of the emitter in the particles
	AParticleEmitterSettings::AttachData* pAttachData = m_pSettings->pAttachData;

	if ( pAttachData && pAttachData->uiStartAxis != 0 )
	{
		const TVector4& rcAxis = m_mTransform.AsBasisVector4( pAttachData->uiStartAxis - 1 );

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			m_aStartAxes[ i ] = rcAxis;
	}
}

// $Barnyard: FUNCTION 006701c0
TBOOL AParticleGenericEmitter::Update( TFLOAT a_fDeltaTime )
{
	TBOOL bFinished = TTRUE;
	m_fTime += a_fDeltaTime;

	if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ] )
		m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_COLOUR ]->ApplyByAge( this, 0, m_iNumParticles );

	if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ] )
		m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_UV ]->ApplyByAge( this, 0, m_iNumParticles );

	if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_MASS ] )
		m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_MASS ]->ApplyByAge( this, 0, m_iNumParticles );

	if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ] )
		m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_SIZE ]->ApplyByAge( this, 0, m_iNumParticles );

	if ( m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_EVENTS ] )
		m_pSettings->apTimelines[ AParticleEmitterSettings::TIMELINE_EVENTS ]->ApplyEvents( this, 0, m_iNumParticles );

	for ( AParticleAffector* pAffector = m_pSettings->pAffectors; pAffector; pAffector = pAffector->pNext )
		pAffector->fnAffector( this, 0, m_iNumParticles, a_fDeltaTime, pAffector->pData );

	// Age the particles and respawn the dead ones
	for ( TINT i = 0; i < m_iNumParticles; i++ )
	{
		m_aAges[ i ] += a_fDeltaTime * m_aAgeRates[ i ];

		if ( m_aAges[ i ] <= 1.0f )
		{
			bFinished = TFALSE;
		}
		else if ( HASANYFLAG( m_eFlags, FLAGS_SPAWN_NEW ) )
		{
			SpawnParticles( i, i + 1, 0.0f, TTRUE );
			bFinished = TFALSE;
		}
	}

	if ( !HASANYFLAG( m_eFlags, FLAGS_SPAWN_NEW ) )
	{
		// The particles that are not rendered keep aging
		for ( TINT i = m_iNumParticles; i < m_iMaxParticles; i++ )
		{
			m_aAges[ i ] += a_fDeltaTime * m_aAgeRates[ i ];

			if ( m_aAges[ i ] <= 1.0f )
				bFinished = TFALSE;
		}
	}

	if ( HASANYFLAG( m_eFlags, FLAGS_KEEP_ALIVE ) )
		bFinished = TFALSE;

	TINT iNumParticles = m_iNumParticles;

	if ( iNumParticles > 0 )
	{
		if ( m_pSettings->bComputeCentre == TTRUE )
		{
			TFLOAT fX = 0.0f;
			TFLOAT fY = 0.0f;
			TFLOAT fZ = 0.0f;

			for ( TINT i = 0; i < iNumParticles; i++ )
			{
				fX += m_aPositions[ i ].x;
				fY += m_aPositions[ i ].y;
				fZ += m_aPositions[ i ].z;
			}

			TFLOAT fInvNumParticles = 1.0f / iNumParticles;
			m_vecBoundingSphere.x   = fX * fInvNumParticles;
			m_vecBoundingSphere.y   = fY * fInvNumParticles;
			m_vecBoundingSphere.z   = fZ * fInvNumParticles;
		}

		if ( m_pSettings->bComputeRadius == TTRUE )
		{
			TVector4 vecMin( 0.0f, 0.0f, 0.0f, 0.0f );
			TVector4 vecMax( 0.0f, 0.0f, 0.0f, 0.0f );

			for ( TINT i = 0; i < m_iNumParticles; i++ )
			{
				TFLOAT fX = m_aPositions[ i ].x - m_vecBoundingSphere.x;
				TFLOAT fY = m_aPositions[ i ].y - m_vecBoundingSphere.y;
				TFLOAT fZ = m_aPositions[ i ].z - m_vecBoundingSphere.z;

				if ( fX < vecMin.x )
					vecMin.x = fX;
				else if ( fX > vecMax.x )
					vecMax.x = fX;

				if ( fY < vecMin.y )
					vecMin.y = fY;
				else if ( fY > vecMax.y )
					vecMax.y = fY;

				if ( fZ < vecMin.z )
					vecMin.z = fZ;
				else if ( fZ > vecMax.z )
					vecMax.z = fZ;
			}

			vecMin.Abs4();
			vecMax.Abs4();

			TVector3 vecExtents;
			vecExtents.x = ( vecMin.x <= vecMax.x ) ? vecMax.x : vecMin.x;
			vecExtents.y = ( vecMin.y <= vecMax.y ) ? vecMax.y : vecMin.y;
			vecExtents.z = ( vecMin.z <= vecMax.z ) ? vecMax.z : vecMin.z;

			m_vecBoundingSphere.w = vecExtents.Magnitude();
		}
	}

	TMatrix44::MatToQuat( m_quatPrevRotation, m_mTransform );
	m_vecPrevPosition = m_mTransform.GetTranslation();

	return !bFinished;
}

// $Barnyard: FUNCTION 006719c0
void AParticleGenericEmitter::Render()
{
	AParticleSystem* pSystem    = AParticleSystem::GetSingleton();
	TMatrix44        mModelView = pSystem->GetModelView();

	if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_TRANS ) )
	{
		TMatrix44 mEmitterModelView;
		mEmitterModelView.Multiply( mModelView, m_mTransform );
		pSystem->SetModelView( mEmitterModelView );
	}
	else if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POS ) )
	{
		TMatrix44 mEmitterModelView = mModelView;
		TMatrix44::TransformVector( mEmitterModelView.GetTranslation(), mEmitterModelView, m_mTransform.GetTranslation() );
		pSystem->SetModelView( mEmitterModelView );
	}

	if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_SPRITE )
	{
		if ( m_pSettings->pRotationData->bFixedCorners )
		{
			s_vecSpriteCorner0.Set( -0.5f, 0.0f, -0.5f, 1.0f );
			s_vecSpriteCorner1.Set( -0.5f, 0.0f, 0.5f, 1.0f );
			s_vecSpriteCorner2.Set( 0.5f, 0.0f, -0.5f, 1.0f );
			s_vecSpriteCorner3.Set( 0.5f, 0.0f, 0.5f, 1.0f );
		}
		else
		{
			const TVector4* pCorners = AParticleSystem::GetQuadCorners();

			s_vecSpriteCorner0 = pCorners[ 0 ];
			s_vecSpriteCorner1 = pCorners[ 2 ];
			s_vecSpriteCorner2 = pCorners[ 1 ];
			s_vecSpriteCorner3 = pCorners[ 3 ];
		}
	}

	s_iUVUnk  = m_pSettings->apUVs[ 0 ]->iUnk;
	s_iUVUnk2 = m_pSettings->apUVs[ 0 ]->iUnk2;
	s_iUVUnk3 = m_pSettings->apUVs[ 1 ]->iUnk;
	s_iUVUnk4 = m_pSettings->apUVs[ 1 ]->iUnk2;

	m_afUnk[ 0 ] = 0.0f;
	m_afUnk[ 1 ] = 0.0f;
	m_afUnk[ 3 ] = 1.0f;
	m_afUnk[ 2 ] = 0.0f;
	m_afUnk[ 4 ] = 1.0f;
	m_afUnk[ 5 ] = 0.0f;
	m_afUnk[ 6 ] = 1.0f;
	m_afUnk[ 7 ] = 1.0f;

	if ( HASANYFLAG( m_pSettings->uiFlags, AParticleEmitterSettings::FLAGS_ROTATE ) )
	{
		if ( pSystem->IsUpdated() )
			UpdateRotations( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );
		else
			UpdateRotations( 0.0f );
	}

	if ( HASANYFLAG( m_pSettings->uiFlags, AParticleEmitterSettings::FLAGS_LIGHTING ) )
		UpdateLighting();

	if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_RIBBON )
		RenderRibbon( m_fnSetPositions, m_fnSetUVs, m_fnSetColours, m_fnWriteIndices, this );
	else if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_MESH )
		RenderMeshes();
	else
		RenderQuads( m_fnSetPositions, m_fnSetUVs, m_fnSetColours, m_fnWriteIndices, this );

	if ( HASANYFLAG( m_eFlags, FLAGS_FOLLOW_POS | FLAGS_FOLLOW_TRANS ) )
		pSystem->SetModelView( mModelView );
}

// $Barnyard: FUNCTION 0066f430
void AParticleGenericEmitter::RandomiseRotations()
{
	TINT iNumRotations;

	if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_SPARK )
		iNumRotations = m_pSettings->pSparkData->iNumRotations;
	else
		iNumRotations = m_pSettings->pRotationData->iNumRotations;

	for ( TINT i = 0; i < iNumRotations; i++ )
		m_afRotations[ i ] = s_oRotationRandom.GetFloat2() * TMath::PI;
}

// $Barnyard: FUNCTION 0066f4c0
void AParticleGenericEmitter::UpdateRotations( TFLOAT a_fDeltaTime )
{
	const TMatrix44& rcViewModel = TRenderInterface::GetSingleton()->GetCurrentContext()->GetViewModelMatrix();

	auto fnSetCorners = []( TINT a_iRotation, const TVector4& a_rcSide, const TVector4& a_rcUp ) {
		TVector4* pCorners = s_aRotatedCorners[ a_iRotation ];

		pCorners[ 0 ].Set( a_rcUp.x - a_rcSide.x, a_rcUp.y - a_rcSide.y, a_rcUp.z - a_rcSide.z, a_rcUp.w );
		pCorners[ 1 ].Set( a_rcUp.x + a_rcSide.x, a_rcSide.y + a_rcUp.y, a_rcSide.z + a_rcUp.z, a_rcUp.w );
		pCorners[ 3 ].Set( a_rcSide.x - a_rcUp.x, a_rcSide.y - a_rcUp.y, a_rcSide.z - a_rcUp.z, a_rcSide.w );
		pCorners[ 2 ].Set( -a_rcUp.x - a_rcSide.x, -a_rcUp.y - a_rcSide.y, -a_rcUp.z - a_rcSide.z, -a_rcUp.w );
	};

	if ( m_pSettings->eRenderType == AParticleEmitterSettings::RENDERTYPE_SPARK )
	{
		AParticleEmitterSettings::SparkData* pSpark = m_pSettings->pSparkData;

		s_iNumRotations = pSpark->iNumRotations;

		if ( s_iNumRotations > m_iNumParticles )
			s_iNumRotations = m_iNumParticles;

		TVector4 vecForward( pSpark->vecForward.x, pSpark->vecForward.y, pSpark->vecForward.z, 1.0f );
		TVector4 vecUp( pSpark->vecUp.x, pSpark->vecUp.y, pSpark->vecUp.z, 1.0f );
		TBOOL    bLocalAxes = pSpark->uiFlags & AParticleEmitterSettings::SparkData::FLAGS_LOCAL_AXES;

		TMatrix44 mAxes = TMatrix44::IDENTITY;
		mAxes.AsBasisVector4( 0 ).Set( vecForward.x, vecForward.y, vecForward.z, 1.0f );
		mAxes.AsBasisVector4( 1 ).CrossProduct( vecForward, vecUp );
		mAxes.AsBasisVector4( 2 ) = vecUp;

		if ( bLocalAxes )
		{
			vecForward.Set( 1.0f, 0.0f, 0.0f, 1.0f );
			vecUp.Set( 0.0f, 0.0f, 1.0f, 1.0f );
		}

		for ( TINT i = 0; i < s_iNumRotations; i++ )
		{
			TFLOAT fAngle      = a_fDeltaTime + m_afRotations[ i ];
			m_afRotations[ i ] = fAngle;

			TFLOAT fAngleX = fAngle * pSpark->afSpeedsX[ i ];
			TFLOAT fAngleY = fAngle * pSpark->afSpeedsY[ i ];
			TFLOAT fAngleZ = fAngle * pSpark->afSpeedsZ[ i ];

			TVector4 vecSide = vecForward;
			vecSide.AsVector3().RotateX( fAngleX );
			vecSide.AsVector3().RotateY( fAngleY );
			vecSide.AsVector3().RotateZ( fAngleZ );

			TVector4 vecSparkUp = vecUp;
			vecSparkUp.AsVector3().RotateX( fAngleX );
			vecSparkUp.AsVector3().RotateY( fAngleY );
			vecSparkUp.AsVector3().RotateZ( fAngleZ );

			if ( bLocalAxes )
			{
				TMatrix44::RotateVector( vecSparkUp, mAxes, vecSparkUp );
				TMatrix44::RotateVector( vecSide, mAxes, vecSide );
			}

			fnSetCorners( i, vecSide, vecSparkUp );
		}
	}
	else
	{
		AParticleEmitterSettings::RotationData* pRotation = m_pSettings->pRotationData;

		s_iNumRotations = pRotation->iNumRotations;

		if ( s_iNumRotations > m_iNumParticles )
			s_iNumRotations = m_iNumParticles;

		if ( pRotation->bFixedCorners )
		{
			for ( TINT i = 0; i < s_iNumRotations; i++ )
			{
				m_afRotations[ i ] += a_fDeltaTime * pRotation->afSpeeds[ i ];

				TFLOAT fSin;
				TFLOAT fCos;
				TMath::SinCos( m_afRotations[ i ], fSin, fCos );

				fCos *= 0.5f;
				fSin *= 0.5f;

				TVector4* pCorners = s_aRotatedCorners[ i ];
				pCorners[ 0 ].Set( fSin - fCos, 0.0f, fCos + fSin, 1.0f );
				pCorners[ 1 ].Set( fSin + fCos, 0.0f, fCos - fSin, 1.0f );
				pCorners[ 3 ].Set( fCos - fSin, 0.0f, -fSin - fCos, 1.0f );
				pCorners[ 2 ].Set( -fSin - fCos, 0.0f, fSin - fCos, -1.0f );
			}
		}
		else
		{
			for ( TINT i = 0; i < s_iNumRotations; i++ )
			{
				m_afRotations[ i ] = a_fDeltaTime * pRotation->afSpeeds[ i ] + m_afRotations[ i ];

				TFLOAT fSin;
				TFLOAT fCos;
				TMath::SinCos( m_afRotations[ i ], fSin, fCos );

				fCos *= 0.5f;
				fSin *= 0.5f;

				TVector4 vecSide( fCos, fSin, 0.0f, 1.0f );
				TVector4 vecUp( -fSin, fCos, 0.0f, 1.0f );

				TMatrix44::RotateVector( vecSide, rcViewModel, vecSide );
				TMatrix44::RotateVector( vecUp, rcViewModel, vecUp );

				fnSetCorners( i, vecSide, vecUp );
			}
		}
	}

	// Round up to a power of two (the particles pick their rotation with a mask)
	TFLOAT fValue   = TFLOAT( 2 * s_iNumRotations - 1 );
	TINT   iBits    = *TREINTERPRETCAST( TINT*, &fValue );
	s_iNumRotations = 1 << ( ( ( iBits >> 23 ) - 127 ) & 31 );
}

// $Barnyard: FUNCTION 0066fb80
void AParticleGenericEmitter::UpdateLighting()
{
	TRenderInterface* pRenderInterface = TRenderInterface::GetSingleton();
	const TVector4&   rcAmbient        = pRenderInterface->GetCurrentContext()->GetAmbientColour();

	TVector4 vecLight;
	vecLight.Multiply( pRenderInterface->GetLightColour().AsBasisVector4( 0 ), m_pSettings->fLightScale );
	vecLight.x += rcAmbient.x;
	vecLight.y += rcAmbient.y;
	vecLight.z += rcAmbient.z;
	vecLight.Multiply( vecLight, 255.0f );

	TMath::Clip( vecLight.x, 0.0f, 255.0f );
	TMath::Clip( vecLight.y, 0.0f, 255.0f );
	TMath::Clip( vecLight.z, 0.0f, 255.0f );

	s_iLightR = TINT( vecLight.x );
	s_iLightG = TINT( vecLight.y );
	s_iLightB = TINT( vecLight.z );
}

// $Barnyard: FUNCTION 00671060
void AParticleGenericEmitter::RenderRibbon( AParticleVertexFunc a_fnPositions, AParticleVertexFunc a_fnUVs, AParticleVertexFunc a_fnColours, AParticleIndexFunc a_fnIndices, AParticleEmitter* a_pEmitter )
{
	if ( m_iNumParticles <= 1 )
		return;

	AParticleEmitterSettings::AttachData* pAttach = m_pSettings->pAttachData;

	TINT  iVerticesLeft = m_iNumParticles * 4;
	TBOOL bDone         = TFALSE;
	TINT  iParticle     = 0;

	if ( pAttach->uiUpAxis != 0 )
	{
		s_vecRibbonUp = m_mTransform.AsBasisVector4( pAttach->uiUpAxis - 1 );
	}
	else
	{
		s_vecRibbonUp.x = pAttach->vecUp.x;
		s_vecRibbonUp.y = pAttach->vecUp.y;
		s_vecRibbonUp.z = pAttach->vecUp.z;
	}

	// Sort the particles by age
	TUINT8 aOrder[ 256 ];
	TINT   iNumParticles = m_iNumParticles;

	for ( TINT i = 0; i < iNumParticles; i++ )
		aOrder[ i ] = TUINT8( i );

	for ( TINT i = 0; i < iNumParticles - 1; i++ )
	{
		TFLOAT fMinAge = m_aAges[ aOrder[ i ] ];

		for ( TINT j = i + 1; j < iNumParticles; j++ )
		{
			TFLOAT fAge = m_aAges[ aOrder[ j ] ];

			if ( fAge < fMinAge )
			{
				fMinAge = fAge;
				TSwapValues( aOrder[ i ], aOrder[ j ] );
			}
		}
	}

	s_fRibbonMinAge           = m_aAges[ aOrder[ 0 ] ];
	s_fRibbonMaxAge           = m_aAges[ aOrder[ m_iNumParticles - 1 ] ];
	aOrder[ m_iNumParticles ] = aOrder[ m_iNumParticles - 1 ];

	if ( pAttach->bAttachFirst )
		m_aPositions[ aOrder[ 0 ] ] = m_mTransform.GetTranslation();

	TVector4 vecSide;

	do
	{
		AParticleMeshLock* pLock = AParticleSystem::GetRenderSystem()->GetMeshLock( iVerticesLeft, GetNumVertices( 1 ), m_iMaterial );

		if ( !pLock )
			break;

		TINT iNumSegments = TUINT( pLock->pMesh->GetMaxVertices() - pLock->uiNumVertices ) / TUINT( GetNumVertices( 1 ) );

		if ( iNumSegments == 0 )
			break;

		if ( iNumSegments + iParticle >= m_iNumParticles )
		{
			iNumSegments = m_iNumParticles - iParticle;
			bDone        = TTRUE;
		}

		iVerticesLeft -= GetNumVertices( iNumSegments );

		TINT iNext = iParticle + 1;

		for ( TINT i = 0; i < iNumSegments; i++, iNext++ )
		{
			TUINT8 uiParticle = aOrder[ iNext - 1 ];

			if ( pAttach->uiStartAxis == 0 )
			{
				if ( iNext != m_iNumParticles )
				{
					// The side is perpendicular to the direction to the next particle
					const TVector4& rcPosition = m_aPositions[ uiParticle ];
					const TVector4& rcNext     = m_aPositions[ aOrder[ iNext ] ];

					TVector4 vecDirection( rcNext.x - rcPosition.x, rcNext.y - rcPosition.y, rcNext.z - rcPosition.z, rcNext.w );
					vecDirection.Normalise();
					vecSide.CrossProduct( vecDirection, s_vecRibbonUp );
				}
			}
			else
			{
				vecSide = m_aStartAxes[ uiParticle ];
			}

			a_fnColours( pLock, 2 * i, uiParticle, a_pEmitter );
			a_fnUVs( pLock, 2 * i, uiParticle, a_pEmitter );
			SetRibbonPositions( pLock, 2 * i, uiParticle, a_pEmitter, vecSide );
		}

		TINT iNumVertices = GetNumVertices( iNumSegments );
		pLock->Advance( iNumVertices, a_fnIndices( pLock, iNumSegments, a_pEmitter ) );

		if ( bDone )
			break;

		iParticle = iNext - 1;

	} while ( TTRUE );
}

// $Barnyard: FUNCTION 006715b0
void AParticleGenericEmitter::RenderMeshes()
{
	if ( m_iNumParticles <= 0 || m_pSettings->iModel < 0 )
		return;

	AParticleModel* pModel         = &AParticleSystem::GetSingleton()->GetModelLibrary()->pModels[ m_pSettings->iModel ];
	TModel*         pTModel        = pModel->pManagedModel->GetModel();
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	TINT       iNumMeshes = pTModel->m_LODs[ 0 ].iNumMeshes;
	TModelLOD& rLOD       = pTModel->m_LODs[ 0 ];

	TMatrix44 mOldModelView = pRenderContext->GetModelViewMatrix();
	TMatrix44 mParticle     = TMatrix44::IDENTITY;

	pRenderContext->SetSkeletonInstance( pModel->pSceneObject->GetSkeletonInstance() );

	for ( TINT i = 0; i < m_iNumParticles; i++ )
	{
		mParticle.GetTranslation() = m_aPositions[ i ];

		if ( m_aVelocities[ i ].MagnitudeSq() >= 0.01f )
		{
			// Face the direction of the particle
			const TVector4& rcVelocity = m_aVelocities[ i ];

			mParticle.AsBasisVector4( 2 ).Set( -rcVelocity.x, -rcVelocity.y, -rcVelocity.z, rcVelocity.w );
			mParticle.AsBasisVector4( 2 ).Normalise();
			mParticle.AsBasisVector4( 0 ).CrossProduct( TVector4::VEC_POSZ, mParticle.AsBasisVector4( 2 ) );
			mParticle.AsBasisVector4( 1 ).CrossProduct( mParticle.AsBasisVector4( 0 ), mParticle.AsBasisVector4( 2 ) );
		}
		else
		{
			// NOTE: this resets the position of the particle too
			mParticle = TMatrix44::IDENTITY;
		}

		mParticle.m_f14 = 0.0f;
		mParticle.m_f24 = 0.0f;
		mParticle.m_f34 = 0.0f;
		mParticle.m_f44 = 1.0f;

		TMatrix44 mModelView;
		mModelView.Multiply( mOldModelView, mParticle );
		pRenderContext->SetModelViewMatrix( mModelView );

		for ( TINT j = 0; j < iNumMeshes; j++ )
			rLOD.ppMeshes[ j ]->Render();
	}

	pRenderContext->SetSkeletonInstance( TNULL );
	pRenderContext->SetModelViewMatrix( mOldModelView );
}

// $Barnyard: FUNCTION 00670a90
void AParticleGenericEmitter::SetRibbonPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter, const TVector4& a_rcSide )
{
	AParticleGenericEmitter* pEmitter   = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	const TVector4&          rcPosition = pEmitter->m_aPositions[ a_iParticle ];

	TVector4 vecOffset;
	vecOffset.Multiply( a_rcSide, pEmitter->m_aSizes[ a_iParticle ] );

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vPosition.Set( vecOffset.x + rcPosition.x, vecOffset.y + rcPosition.y, vecOffset.z + rcPosition.z );
	pVertices[ 1 ].vPosition.Set( -vecOffset.x + rcPosition.x, -vecOffset.y + rcPosition.y, -vecOffset.z + rcPosition.z );
}

// $Barnyard: FUNCTION 0066fc60
TINT AParticleGenericEmitter::WriteRibbonIndices( AParticleMeshLock* a_pLock, TINT a_iNumParticles, AParticleEmitter* a_pEmitter )
{
	a_pLock->pIndices[ 0 ] = a_pLock->uiNumVertices;

	TUINT16 uiIndex  = 1;
	TUINT16 uiVertex = 0;

	for ( TINT i = 0; i < a_iNumParticles; i++ )
	{
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + a_pLock->uiNumVertices;
		a_pLock->pIndices[ uiIndex++ ] = uiVertex + 1 + a_pLock->uiNumVertices;
		uiVertex += 2;
	}

	a_pLock->pIndices[ uiIndex ] = uiVertex + a_pLock->uiNumVertices - 1;
	return TUINT16( uiIndex + 1 );
}

// $Barnyard: FUNCTION 00670660
void AParticleGenericEmitter::SetSpritePositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TFLOAT                   fSize    = pEmitter->m_fnGetSize( pEmitter, a_iParticle );

	TVector4 vecCorner0;
	TVector4 vecCorner1;
	TVector4 vecCorner2;
	TVector4 vecCorner3;
	vecCorner0.Multiply( s_vecSpriteCorner0, fSize );
	vecCorner1.Multiply( s_vecSpriteCorner1, fSize );
	vecCorner2.Multiply( s_vecSpriteCorner2, fSize );
	vecCorner3.Multiply( s_vecSpriteCorner3, fSize );

	const TVector4&   rcPosition = pEmitter->m_aPositions[ a_iParticle ];
	ASysMesh::Vertex* pVertices  = a_pLock->pVertices + a_iVertex;

	pVertices[ 0 ].vPosition.Set( vecCorner0.x + rcPosition.x, vecCorner0.y + rcPosition.y, vecCorner0.z + rcPosition.z );
	pVertices[ 1 ].vPosition.Set( vecCorner1.x + rcPosition.x, vecCorner1.y + rcPosition.y, vecCorner1.z + rcPosition.z );
	pVertices[ 2 ].vPosition.Set( vecCorner2.x + rcPosition.x, vecCorner2.y + rcPosition.y, vecCorner2.z + rcPosition.z );
	pVertices[ 3 ].vPosition.Set( vecCorner3.x + rcPosition.x, vecCorner3.y + rcPosition.y, vecCorner3.z + rcPosition.z );
}

// $Barnyard: FUNCTION 00670780
void AParticleGenericEmitter::SetRotatedSpritePositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter  = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TINT                     iRotation = ( ( 4 * a_iParticle ) & ( 4 * s_iNumRotations - 4 ) ) / 4;
	TFLOAT                   fSize     = pEmitter->m_fnGetSize( pEmitter, a_iParticle );

	TVector4 vecCorner2;
	TVector4 vecCorner0;
	TVector4 vecCorner3;
	TVector4 vecCorner1;
	vecCorner2.Multiply( s_aRotatedCorners[ iRotation ][ 2 ], fSize );
	vecCorner0.Multiply( s_aRotatedCorners[ iRotation ][ 0 ], fSize );
	vecCorner3.Multiply( s_aRotatedCorners[ iRotation ][ 3 ], fSize );
	vecCorner1.Multiply( s_aRotatedCorners[ iRotation ][ 1 ], fSize );

	const TVector4&   rcPosition = pEmitter->m_aPositions[ a_iParticle ];
	ASysMesh::Vertex* pVertices  = a_pLock->pVertices + a_iVertex;

	pVertices[ 0 ].vPosition.Set( vecCorner2.x + rcPosition.x, vecCorner2.y + rcPosition.y, vecCorner2.z + rcPosition.z );
	pVertices[ 1 ].vPosition.Set( vecCorner0.x + rcPosition.x, vecCorner0.y + rcPosition.y, vecCorner0.z + rcPosition.z );
	pVertices[ 2 ].vPosition.Set( vecCorner3.x + rcPosition.x, vecCorner3.y + rcPosition.y, vecCorner3.z + rcPosition.z );
	pVertices[ 3 ].vPosition.Set( vecCorner1.x + rcPosition.x, vecCorner1.y + rcPosition.y, vecCorner1.z + rcPosition.z );
}

// $Barnyard: FUNCTION 006708d0
void AParticleGenericEmitter::SetStretchedPositions( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter*               pEmitter   = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TFLOAT                                 fSize      = pEmitter->m_fnGetSize( pEmitter, a_iParticle );
	const TVector4&                        rcPosition = pEmitter->m_aPositions[ a_iParticle ];
	const TVector4&                        rcVelocity = pEmitter->m_aVelocities[ a_iParticle ];
	AParticleEmitterSettings::StretchData* pStretch   = pEmitter->m_pSettings->pStretchData;

	TFLOAT fWidth = fSize * pStretch->fWidthScale;
	TFLOAT fFront = fSize * pStretch->fFrontScale;
	TFLOAT fBack  = -( fSize * pStretch->fBackScale );

	TVector4 vecDirection = rcVelocity;
	vecDirection.Normalise();

	// The side faces up
	TVector4 vecSide;
	vecSide.Multiply( vecDirection, fWidth );
	vecSide.CrossProduct( vecSide, TVector4::VEC_NEGY );

	TVector4 vecFront;
	TVector4 vecBack;
	vecFront.Multiply( vecDirection, fFront );
	vecBack.Multiply( vecDirection, fBack );

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vPosition.Set( vecSide.x + rcPosition.x, vecSide.y + rcPosition.y, vecSide.z + rcPosition.z );
	pVertices[ 1 ].vPosition.Set( vecFront.x + rcPosition.x, vecFront.y + rcPosition.y, vecFront.z + rcPosition.z );
	pVertices[ 2 ].vPosition.Set( vecBack.x + rcPosition.x, vecBack.y + rcPosition.y, vecBack.z + rcPosition.z );
	pVertices[ 3 ].vPosition.Set( -vecSide.x + rcPosition.x, -vecSide.y + rcPosition.y, -vecSide.z + rcPosition.z );
}

// $Barnyard: FUNCTION 00670b70
void AParticleGenericEmitter::SetRibbonUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TFLOAT                   fAge     = pEmitter->m_aAges[ a_iParticle ];

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vUV.Set( 0.0f, fAge );
	pVertices[ 1 ].vUV.Set( 1.0f, fAge );
}

// $Barnyard: FUNCTION 00670bd0
void AParticleGenericEmitter::SetFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	const TVector4&          rcUV     = AParticleSystem::GetSingleton()->GetMaterialLibrary()->pMaterials[ pEmitter->m_iMaterial ].pUVs[ pEmitter->m_aUVFrames[ a_iParticle ] ];

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vUV.Set( rcUV.x, rcUV.w );
	pVertices[ 1 ].vUV.Set( rcUV.x, rcUV.y );
	pVertices[ 2 ].vUV.Set( rcUV.z, rcUV.w );
	pVertices[ 3 ].vUV.Set( rcUV.z, rcUV.y );
}

// $Barnyard: FUNCTION 00670c80
void AParticleGenericEmitter::SetRibbonFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	const TVector4&          rcUV     = AParticleSystem::GetSingleton()->GetMaterialLibrary()->pMaterials[ pEmitter->m_iMaterial ].pUVs[ pEmitter->m_aUVFrames[ a_iParticle ] ];

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vUV.Set( rcUV.x, rcUV.w );
	pVertices[ 1 ].vUV.Set( rcUV.z, rcUV.w );
}

// $Barnyard: FUNCTION 00670d00
void AParticleGenericEmitter::SetStretchedFrameUVs( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	const TVector4&          rcUV     = AParticleSystem::GetSingleton()->GetMaterialLibrary()->pMaterials[ pEmitter->m_iMaterial ].pUVs[ pEmitter->m_aUVFrames[ a_iParticle ] ];

	// NOTE: the original adds the start to the middle of the frame
	TFLOAT fMiddleU = ( rcUV.z + rcUV.x ) * 0.5f + rcUV.x;
	TFLOAT fMiddleV = ( rcUV.w + rcUV.y ) * 0.5f + rcUV.y;

	ASysMesh::Vertex* pVertices = a_pLock->pVertices + a_iVertex;
	pVertices[ 0 ].vUV.Set( rcUV.x, fMiddleV );
	pVertices[ 1 ].vUV.Set( fMiddleU, rcUV.y );
	pVertices[ 2 ].vUV.Set( fMiddleU, rcUV.w );
	pVertices[ 3 ].vUV.Set( rcUV.z, fMiddleV );
}

// Multiplies RGB of a_uiColour by the light colour
static TUINT32 LightColour( TUINT32 a_uiColour )
{
	TUINT32 uiR = TUINT16( s_iLightR * ( ( a_uiColour >> 16 ) & 0xff ) ) >> 8;
	TUINT32 uiG = TUINT16( s_iLightG * ( ( a_uiColour >> 8 ) & 0xff ) ) >> 8;
	TUINT32 uiB = TUINT16( s_iLightB * ( a_uiColour & 0xff ) ) >> 8;

	return ( a_uiColour & 0xff000000 ) | ( ( uiR & 0xff ) << 16 ) | ( ( uiG & 0xff ) << 8 ) | ( uiB & 0xff );
}

// $Barnyard: FUNCTION 00670dc0
void AParticleGenericEmitter::SetColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TUINT32                  uiColour = pEmitter->m_aColours[ a_iParticle ];

	for ( TUINT i = 0; i < TUINT( a_pEmitter->GetNumVertices( 1 ) ); i++ )
		a_pLock->pVertices[ a_iVertex + i ].uiColor = uiColour;
}

// $Barnyard: FUNCTION 00670e30
void AParticleGenericEmitter::SetKeyframeColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	AParticleKeyframe*       pKey     = TREINTERPRETCAST( AParticleKeyframe*, pEmitter->m_aColourKeys[ a_iParticle ] );
	TUINT32                  uiColour;

	if ( pKey )
		uiColour = pKey->BlendColour( pEmitter, a_iParticle );
	else
		uiColour = pEmitter->m_aColours[ a_iParticle ];

	for ( TUINT i = 0; i < TUINT( a_pEmitter->GetNumVertices( 1 ) ); i++ )
		a_pLock->pVertices[ a_iVertex + i ].uiColor = uiColour;
}

// $Barnyard: FUNCTION 00670ec0
void AParticleGenericEmitter::SetLitColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	TUINT32                  uiColour = LightColour( pEmitter->m_aColours[ a_iParticle ] );

	for ( TUINT i = 0; i < TUINT( a_pEmitter->GetNumVertices( 1 ) ); i++ )
		a_pLock->pVertices[ a_iVertex + i ].uiColor = uiColour;
}

// $Barnyard: FUNCTION 00670f80
void AParticleGenericEmitter::SetLitKeyframeColours( AParticleMeshLock* a_pLock, TINT a_iVertex, TINT a_iParticle, AParticleEmitter* a_pEmitter )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	AParticleKeyframe*       pKey     = TREINTERPRETCAST( AParticleKeyframe*, pEmitter->m_aColourKeys[ a_iParticle ] );
	TUINT32                  uiColour;

	if ( pKey )
		uiColour = pKey->BlendColour( pEmitter, a_iParticle );
	else
		uiColour = pEmitter->m_aColours[ a_iParticle ];

	uiColour = LightColour( uiColour );

	for ( TUINT i = 0; i < TUINT( a_pEmitter->GetNumVertices( 1 ) ); i++ )
		a_pLock->pVertices[ a_iVertex + i ].uiColor = uiColour;
}

// $Barnyard: FUNCTION 0066fd60
TFLOAT AParticleGenericEmitter::GetSize( AParticleEmitter* a_pEmitter, TINT a_iParticle )
{
	return TSTATICCAST( AParticleGenericEmitter, a_pEmitter )->m_aSizes[ a_iParticle ];
}

// $Barnyard: FUNCTION 0066fd90
TFLOAT AParticleGenericEmitter::GetKeyframeSize( AParticleEmitter* a_pEmitter, TINT a_iParticle )
{
	AParticleGenericEmitter* pEmitter = TSTATICCAST( AParticleGenericEmitter, a_pEmitter );
	AParticleKeyframe*       pKey     = TREINTERPRETCAST( AParticleKeyframe*, pEmitter->m_aSizeKeys[ a_iParticle ] );

	if ( !pKey )
		return pEmitter->m_aSizes[ a_iParticle ];

	return pKey->BlendSize( pEmitter, a_iParticle );
}
