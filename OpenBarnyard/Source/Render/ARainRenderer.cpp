#include "pch.h"
#include "ARainRenderer.h"

#include "AppBoot.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASysShader/ASysShader.h"
#include "Physics/ACollisionManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"

#include <Render/TRenderInterface.h>

#include <Render/TMaterial.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0060a0f0
ARainRenderer::RainDrop::RainDrop( ARainRenderer* a_pRainRenderer )
{
	m_pMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( m_pMesh )
	{
		if ( m_pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 0x28, 0x3c ) )
			m_pMesh->SetMaterial( a_pRainRenderer->m_pRainMaterial );
	}

	m_pPositions = new TVector3[ 10 ];

	for ( TINT i = 0; i < 10; i++ )
		m_pPositions[ i ] = TVector3::VEC_ZERO;

	m_uiColour = 0x4cffffff;
}

// $Barnyard: FUNCTION 00608d00
ARainRenderer::RainDrop::~RainDrop()
{
	if ( m_pMesh )
		m_pMesh->DestroyResource();

	delete[] m_pPositions;
}

// Randoms of the drops (seeded with ARainRenderer::m_uiRandomSeed every frame) and of the splashes
static TRandom s_oRainDropRandom;   // 0x0079b220
static TRandom s_oRainSplashRandom; // 0x0079b188

// Returns 0 when the camera is above -20 (y is down) and 1 when it's above -60,
// the rain is spread wider and spawned higher the higher the camera is
// NOTE: inlined in ARainRenderer::RainDrop::Reset and ARainRenderer::RainDrop::Update
static TFLOAT GetCameraHeightFactor()
{
	TFLOAT fCameraHeight = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().m_f42;

	if ( fCameraHeight > -20.0f )
		return 0.0f;

	if ( fCameraHeight >= -60.0f )
		return ( fCameraHeight - -20.0f ) / ( -60.0f - -20.0f );

	return 1.0f;
}

// $Barnyard: FUNCTION 00608d20
void ARainRenderer::RainDrop::Reset( const TVector4& a_rPosition, ARainRenderer* a_pRainRenderer )
{
	TFLOAT fHeightFactor  = GetCameraHeightFactor();
	TFLOAT fWindStrength  = a_pRainRenderer->m_fWindStrength;
	TFLOAT fSpread        = TMath::LERP( 30.0f, 40.0f, fHeightFactor );
	TFLOAT fSpawnHeight   = TMath::LERP( -5.0f, -40.0f, fHeightFactor );
	TFLOAT fWindInfluence = ( 1.0f - fHeightFactor ) * 0.5f;

	for ( TINT i = 0; i < NUM_POSITIONS; i++ )
	{
		TVector3& rPos = m_pPositions[ i ];

		rPos.x = a_rPosition.x;
		rPos.y = fSpawnHeight;
		rPos.z = a_rPosition.z;

		rPos.x += s_oRainDropRandom.GetFloat2() * fSpread;
		rPos.y -= s_oRainDropRandom.GetFloat() * 32.0f;
		rPos.z += s_oRainDropRandom.GetFloat2() * fSpread;

		// Start against the wind so the drops get blown over the camera
		rPos.x -= fWindInfluence * a_pRainRenderer->m_vWindDirection.x * fSpread * fWindStrength;
		rPos.z -= fWindInfluence * a_pRainRenderer->m_vWindDirection.z * fSpread * fWindStrength;
	}

	// Random grey with a random alpha
	TFLOAT fBrightness = TFLOAT( s_oRainDropRandom.RandRaw() >> 1 ) * ( 1.0f / 4294967296.0f ) + 0.5f;
	TFLOAT fAlpha      = s_oRainDropRandom.GetFloat() * 0.2f + 0.5f;
	TUINT  uiGrey      = TUINT( fBrightness * 255.0f );

	m_uiColour = ( TUINT( fAlpha * 255.0f ) << 24 ) | ( uiGrey << 16 ) | ( uiGrey << 8 ) | uiGrey;
}

// $Barnyard: FUNCTION 00609010
TBOOL ARainRenderer::RainDrop::Update( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer, TBOOL a_bRecycle )
{
	TBOOL bLanded = TTRUE;

	TFLOAT fHeightFactor  = GetCameraHeightFactor();
	TFLOAT fWind          = 0.5f * fHeightFactor * a_pRainRenderer->m_fWindStrength + 1.0f - 0.5f;
	TFLOAT fFallDistance  = ( 0.5f * fHeightFactor + 1.0f - 0.5f ) * a_pRainRenderer->m_fRainFallSpeed * a_fDeltaTime;
	TFLOAT fSpread        = TMath::LERP( 30.0f, 40.0f, fHeightFactor );
	TFLOAT fSpawnHeight   = TMath::LERP( -5.0f, -40.0f, fHeightFactor );
	TFLOAT fWindInfluence = ( 1.0f - fHeightFactor ) * 0.5f;

	// Respawn around the point in front of the camera unless the camera looks down
	ACamera*        pCamera  = ACameraManager::GetSingleton()->GetCurrentCamera();
	const TVector4& rOrigin  = ( pCamera->GetMatrix().m_f32 < 0.505f ) ? a_pRainRenderer->m_vRainOrigin : pCamera->GetMatrix().GetTranslation();
	TFLOAT          fOriginX = rOrigin.x;
	TFLOAT          fOriginZ = rOrigin.z;

	for ( TINT i = 0; i < NUM_POSITIONS; i++ )
	{
		TVector3& rPos = m_pPositions[ i ];
		rPos.y += fFallDistance;

		if ( a_bRecycle )
		{
			if ( rPos.y > -3.0f )
			{
				rPos.x = fOriginX;
				rPos.y = 0.0f;
				rPos.z = fOriginZ;
				rPos.y = fSpawnHeight;

				rPos.x += s_oRainDropRandom.GetFloat2() * fSpread;
				rPos.y -= s_oRainDropRandom.GetFloat() * 25.0f;
				rPos.z += s_oRainDropRandom.GetFloat2() * fSpread;

				rPos.x -= fWindInfluence * a_pRainRenderer->m_vWindDirection.x * fSpread * fWind;
				rPos.z -= fWindInfluence * a_pRainRenderer->m_vWindDirection.z * fSpread * fWind;
			}
		}
		else if ( rPos.y < 0.0f )
		{
			bLanded = TFALSE;
		}

		TFLOAT fWindMove = fWind * a_pRainRenderer->m_fWindStrength * 2.0f;
		rPos.x += fWindMove * a_pRainRenderer->m_vWindDirection.x;
		rPos.z += fWindMove * a_pRainRenderer->m_vWindDirection.z;
	}

	// The drop goes back to the free list once all of its positions landed
	return !bLanded || a_bRecycle;
}

// $Barnyard: FUNCTION 00609380
void ARainRenderer::RainDrop::Render( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer )
{
	ASysMesh::LockBuffer oLockBuffer;

	if ( !m_pMesh->Lock( oLockBuffer ) )
		return;

	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	TUINT16*          pIndices  = oLockBuffer.IndexLock.pBuffer;

	ACameraManager::GetSingleton()->GetCurrentCamera();

	for ( TINT i = 0; i < NUM_POSITIONS; i++ )
	{
		const TVector3& rPos = m_pPositions[ i ];

		// The drops get wider with the distance (up to 4 times at 35 units)
		TFLOAT fWidth = a_pRainRenderer->m_fRainDropWidth;
		TFLOAT fDX    = rPos.x - a_rCameraPosition.x;
		TFLOAT fDZ    = -( rPos.z - a_rCameraPosition.z );
		TFLOAT fScale = ( fDX * fDX + fDZ * fDZ - 10.0f ) * ( 1.0f / 1225.0f );
		TMath::Clip( fScale, 0.0f, 1.0f );
		fWidth = ( fWidth * 4.0f - fWidth ) * fScale + fWidth;

		// Side direction (perpendicular to the camera direction)
		TFLOAT fInvMax = ( TMath::Abs( fDZ ) <= TMath::Abs( fDX ) ) ? 1.0f / TMath::Abs( fDX ) : 1.0f / TMath::Abs( fDZ );
		TFLOAT fSideX  = fWidth * ( fDZ * fInvMax );
		TFLOAT fSideZ  = ( fDX * fInvMax ) * fWidth;

		TFLOAT fLength = a_pRainRenderer->m_fRainDropLength;
		TFLOAT fWindX  = a_pRainRenderer->m_fWindStrength * 30.0f * a_pRainRenderer->m_vWindDirection.x;
		TFLOAT fWindZ  = a_pRainRenderer->m_fWindStrength * 30.0f * a_pRainRenderer->m_vWindDirection.z;

		ASysMesh::Vertex* pQuad = &pVertices[ i * 4 ];

		pQuad[ 0 ].vPosition = rPos;
		pQuad[ 0 ].uiColor   = m_uiColour;
		pQuad[ 0 ].vUV       = TVector2( 0.0f, 0.0f );

		pQuad[ 1 ].vPosition = TVector3( rPos.x + fSideX, rPos.y, rPos.z + fSideZ );
		pQuad[ 1 ].uiColor   = m_uiColour;
		pQuad[ 1 ].vUV       = TVector2( 1.0f, 0.0f );

		// The tail is blown by the wind
		pQuad[ 2 ].vPosition = TVector3( rPos.x + fWindX, rPos.y + fLength, rPos.z + fWindZ );
		pQuad[ 2 ].uiColor   = m_uiColour;
		pQuad[ 2 ].vUV       = TVector2( 0.0f, 1.0f );

		pQuad[ 3 ].vPosition = TVector3( rPos.x + fWindX + fSideX, rPos.y + fLength, rPos.z + fWindZ + fSideZ );
		pQuad[ 3 ].uiColor   = m_uiColour;
		pQuad[ 3 ].vUV       = TVector2( 1.0f, 1.0f );
	}

	// Strip of quads separated by degenerate triangles
	TINT iNumIndices = 0;

	for ( TINT i = 0; i < NUM_POSITIONS; i++ )
	{
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 1 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 2 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 3 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 3 );
	}

	m_pMesh->Unlock( NUM_POSITIONS * 4, iNumIndices );
	m_pMesh->Validate();
	m_pMesh->Render();
}

// $Barnyard: FUNCTION 0060a280
ARainRenderer::RainSplash::RainSplash( ARainRenderer* a_pRainRenderer )
{
	m_iNumSplashes = 5;
	m_pMesh        = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( m_pMesh )
	{
		if ( m_pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, m_iNumSplashes * 8, m_iNumSplashes * 12 ) )
			m_pMesh->SetMaterial( a_pRainRenderer->m_pRainSplashMaterial );
	}

	m_pPositions = new TVector3[ m_iNumSplashes ];
	m_pTimers    = new TFLOAT[ m_iNumSplashes ];

	for ( TINT i = 0; i < m_iNumSplashes; i++ )
	{
		m_pPositions[ i ] = TVector3::VEC_ZERO;
		m_pTimers[ i ]    = 0.0f;
	}

	m_uiColour = 0x4cffffff;
}

// $Barnyard: FUNCTION 00609690
ARainRenderer::RainSplash::~RainSplash()
{
	if ( m_pMesh )
		m_pMesh->DestroyResource();

	delete[] m_pPositions;
	delete[] m_pTimers;
}

// $Barnyard: FUNCTION 0060a360
void ARainRenderer::RainSplash::Reset( TFLOAT a_fRatio, ARainRenderer* a_pRainRenderer )
{
	TINT iOldNumSplashes = m_iNumSplashes;
	m_iNumSplashes       = TINT( a_fRatio * 5.0f );

	// NOTE: only resets the splashes that got added (none with the ratio of 1 used by ARainRenderer::Update),
	// the others start once their timer runs out in Update
	for ( TINT i = iOldNumSplashes; i < m_iNumSplashes; i++ )
		ResetSplash( i, a_pRainRenderer );
}

// $Barnyard: FUNCTION 006096c0
void ARainRenderer::RainSplash::ResetSplash( TINT a_iIndex, ARainRenderer* a_pRainRenderer )
{
	TFLOAT fRandomZ = s_oRainSplashRandom.GetFloat2();
	TFLOAT fRandomX = s_oRainSplashRandom.GetFloat2();

	// Find the ground below a random point around the rain origin
	TVector4 vOrigin(
	    fRandomX * 10.0f + a_pRainRenderer->m_vRainOrigin.x,
	    a_pRainRenderer->m_vRainOrigin.y - 100.0f,
	    fRandomZ * 10.0f + a_pRainRenderer->m_vRainOrigin.z,
	    1.0f
	);

	ARayData oRay( vOrigin, TVector4::VEC_POSY, 200.0f );
	TFLOAT   fDistance = ATerrainInterface::GetSingleton()->GetRayHitDistance( oRay.vOrigin, oRay.vDirection, oRay.fLength, 0x80000, TTRUE );

	if ( fDistance != oRay.fLength )
	{
		m_pPositions[ a_iIndex ] = TVector3( vOrigin.x, fDistance - 0.05f + vOrigin.y, vOrigin.z );
		m_pTimers[ a_iIndex ]    = TMath::LERP( 0.3f, 0.5f, s_oRainSplashRandom.GetFloat() );
	}
}

// $Barnyard: FUNCTION 006098c0
void ARainRenderer::RainSplash::Update( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer )
{
	for ( TINT i = 0; i < m_iNumSplashes; i++ )
	{
		m_pTimers[ i ] -= a_fDeltaTime;

		if ( m_pTimers[ i ] < 0.0f && a_pRainRenderer->m_fRainIntensity > 0.0f )
			ResetSplash( i, a_pRainRenderer );

		// Bounces up and falls back down
		TFLOAT fSpeed = m_pTimers[ i ] - 0.3f;
		m_pPositions[ i ].y -= fSpeed * a_fDeltaTime + fSpeed * a_fDeltaTime;
	}
}

// $Barnyard: FUNCTION 00609940
void ARainRenderer::RainSplash::Render( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, ARainRenderer* a_pRainRenderer )
{
	ASysMesh::LockBuffer oLockBuffer;

	if ( !m_pMesh->Lock( oLockBuffer ) )
		return;

	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	TUINT16*          pIndices  = oLockBuffer.IndexLock.pBuffer;

	// Corners of the ring on the ground
	const TVector4 vGround0( -0.5f, 0.0f, 0.5f, 1.0f );
	const TVector4 vGround1( 0.5f, 0.0f, 0.5f, 1.0f );
	const TVector4 vGround2( -0.5f, 0.0f, -0.5f, 1.0f );
	const TVector4 vGround3( 0.5f, 0.0f, -0.5f, 1.0f );

	// UVs of the ring (left half of the texture), mirrored differently for every splash
	TVector2 aGroundUVs[ 4 ][ 4 ];

	for ( TINT i = 0; i < 4; i++ )
	{
		aGroundUVs[ i ][ 0 ] = TVector2( 0.0f, 0.0f );
		aGroundUVs[ i ][ 1 ] = TVector2( 0.5f, 0.0f );
		aGroundUVs[ i ][ 2 ] = TVector2( 0.0f, 1.0f );
		aGroundUVs[ i ][ 3 ] = TVector2( 0.5f, 1.0f );

		for ( TINT k = 0; k < 4; k++ )
		{
			if ( i == 1 || i == 3 ) aGroundUVs[ i ][ k ].x = 0.5f - aGroundUVs[ i ][ k ].x;
			if ( i == 2 || i == 3 ) aGroundUVs[ i ][ k ].y = -aGroundUVs[ i ][ k ].y;
		}
	}

	// The drop part (right half of the texture) faces the camera
	TMatrix44 mInvModelView;
	mInvModelView.InvertOrthogonal( TRenderInterface::GetSingleton()->GetCurrentContext()->GetModelViewMatrix() );

	TVector4 vRight( 1.0f, 0.0f, 0.0f, 0.0f );
	TMatrix44::RotateVector( vRight, mInvModelView, vRight );
	vRight.Multiply( 0.5f );

	const TVector4 vUpperRight( vRight.x, vRight.y + -0.4f, vRight.z, 1.0f );
	const TVector4 vLowerRight( vRight.x, vRight.y, vRight.z, 1.0f );
	const TVector4 vUpperLeft( -vRight.x, -0.4f - vRight.y, -vRight.z, 1.0f );
	const TVector4 vLowerLeft( -vRight.x, -vRight.y, -vRight.z, 1.0f );

	for ( TINT i = 0; i < m_iNumSplashes; i++ )
	{
		TFLOAT fTimer = m_pTimers[ i ];
		TFLOAT fScale = 0.4f / 0.5f * ( 0.5f - fTimer ) + 0.1f;

		// Fades out during the last 0.3 seconds
		TFLOAT fAlpha = 0.3f;

		if ( fTimer < 0.3f )
			fAlpha = ( fTimer <= 0.0f ) ? 0.0f : fTimer;

		m_uiColour = ( TUINT32( fAlpha * 255.0f ) << 24 ) | 0xFFFFFF;

		const TVector3&   rPos  = m_pPositions[ i ];
		const TVector2*   pUVs  = aGroundUVs[ i & 3 ];
		ASysMesh::Vertex* pQuad = &pVertices[ i * 8 ];

		const TVector4* aCorners[ 8 ] = { &vGround0, &vGround1, &vGround2, &vGround3, &vUpperLeft, &vUpperRight, &vLowerLeft, &vLowerRight };
		const TVector2  aUVs[ 8 ]     = { pUVs[ 0 ], pUVs[ 1 ], pUVs[ 2 ], pUVs[ 3 ], TVector2( 0.5f, 0.0f ), TVector2( 1.0f, 0.0f ), TVector2( 0.5f, 1.0f ), TVector2( 1.0f, 1.0f ) };

		for ( TINT k = 0; k < 8; k++ )
		{
			TVector4 vCorner;
			vCorner.Multiply( *aCorners[ k ], fScale );

			pQuad[ k ].vPosition = TVector3( vCorner.x + rPos.x, vCorner.y + rPos.y, vCorner.z + rPos.z );
			pQuad[ k ].uiColor   = m_uiColour;
			pQuad[ k ].vUV       = aUVs[ k ];
		}
	}

	// Strip of quads separated by degenerate triangles
	TINT iNumIndices = 0;

	for ( TINT i = 0; i < m_iNumSplashes * 2; i++ )
	{
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 1 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 2 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 3 );
		pIndices[ iNumIndices++ ] = TUINT16( 4 * i + 3 );
	}

	m_pMesh->Unlock( TUINT16( m_iNumSplashes * 8 ), iNumIndices );
	m_pMesh->Validate();
	m_pMesh->Render();
}

// $Barnyard: FUNCTION 0060a8f0
ARainRenderer::ARainRenderer()
{
	m_fMaxRainDrops             = 40.0f;
	m_fRainIntensity            = 0.0f;
	m_fRainFallSpeed            = 80.0f;
	m_fRainSpawnTimer           = 0.0f;
	m_fRainSpawnInterval        = 0.0f;
	m_fRainDropWidth            = 1.0f;
	m_fRainDropLength           = 9.0f;
	m_vWindDirection.x          = 1.0f;
	m_vWindDirection.y          = 0.0f;
	m_vWindDirection.z          = 0.0f;
	m_vWindDirection.w          = 0.0f;
	m_fWindStrength             = 0.0f;
	m_bLockToCurrentWeatherType = TFALSE;
	m_pRainMaterial             = TNULL;
	m_pRainSplashMaterial       = TNULL;
	m_pMaterialLibrary          = TNULL;

	CreateMaterials();

	m_iMaxRainDrops = TINT( m_fMaxRainDrops );
	CreateRainDropPool();

	m_iMaxRainSplashes = 4;
	CreateRainSplashPool();

	m_fDeltaTime = 0.0f;
}

// $Barnyard: FUNCTION 0060ab50
ARainRenderer::~ARainRenderer()
{
	DestroyMaterials();

	m_llActiveRainSplashes.DeleteAll();
	m_llFreeRainSplashes.DeleteAll();
	m_llActiveRainDrops.DeleteAll();
	m_llFreeRainDrops.DeleteAll();
}

// $Barnyard: FUNCTION 00608ce0
void ARainRenderer::SetRainDropSize( TFLOAT a_fWidth, TFLOAT a_fLength )
{
	m_fRainDropWidth  = a_fWidth;
	m_fRainDropLength = a_fLength;
}

// $Barnyard: FUNCTION 0060a3b0
void ARainRenderer::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bLockToCurrentWeatherType == TTRUE || !ACollisionManager::IsSingletonCreated() || !ATerrainInterface::IsSingletonCreated() )
		return;

	// Start all of the splashes
	while ( !m_llFreeRainSplashes.IsEmpty() )
	{
		RainSplash* pSplash = m_llFreeRainSplashes.RemoveTail();
		pSplash->Reset( 1.0f, this );
		m_llActiveRainSplashes.InsertHead( pSplash );
	}

	m_fDeltaTime   = a_fDeltaTime;
	m_uiRandomSeed = m_oRandom.RandRaw();

	// The rain is centered 10 units in front of the camera
	ACamera*        pCamera         = ACameraManager::GetSingleton()->GetCurrentCamera();
	const TVector4& rCameraPosition = pCamera->GetMatrix().GetTranslation();

	m_vRainOrigin.x = pCamera->GetMatrix().m_f31;
	m_vRainOrigin.y = pCamera->GetMatrix().m_f32;
	m_vRainOrigin.z = pCamera->GetMatrix().m_f33;
	m_vRainOrigin.w = 1.0f;
	m_vRainOrigin.y = 0.0f;
	m_vRainOrigin.Multiply( 10.0f );
	m_vRainOrigin.x += rCameraPosition.x;
	m_vRainOrigin.y += rCameraPosition.y;
	m_vRainOrigin.z += rCameraPosition.z;

	// NOTE: the drops past the current maximum number of drops are the ones that keep being recycled,
	// the others go back to the free list once they landed and Render spawns them again
	TINT iMaxActiveDrops = TINT( m_fMaxRainDrops * m_fRainIntensity );
	TINT iIndex          = 0;

	for ( auto it = m_llActiveRainDrops.Begin(); it != m_llActiveRainDrops.End(); )
	{
		RainDrop* pDrop = it;
		it++;

		if ( !pDrop->Update( a_fDeltaTime, rCameraPosition, this, iIndex >= iMaxActiveDrops ) )
		{
			m_llActiveRainDrops.Remove( pDrop );
			m_llFreeRainDrops.InsertTail( pDrop );
		}

		iIndex++;
	}

	if ( ACollisionManager::IsSingletonCreated() && ATerrainInterface::IsSingletonCreated() )
	{
		for ( auto it = m_llActiveRainSplashes.Begin(); it != m_llActiveRainSplashes.End(); )
		{
			RainSplash* pSplash = it;
			it++;

			pSplash->Update( m_fDeltaTime, rCameraPosition, this );
		}
	}
}

// $Barnyard: FUNCTION 0060a5f0
void ARainRenderer::Render()
{
	if ( m_fRainIntensity < 0.05f )
		return;

	s_oRainDropRandom.SetSeed( m_uiRandomSeed );

	m_fElapsedTime += m_fDeltaTime;
	m_fRainSpawnTimer -= m_fDeltaTime;

	ACamera*        pCamera         = ACameraManager::GetSingleton()->GetCurrentCamera();
	const TVector4& rCameraPosition = pCamera->GetMatrix().GetTranslation();

	if ( m_fRainIntensity > 0.05f )
	{
		TINT iMaxActiveDrops = TINT( m_fMaxRainDrops * m_fRainIntensity );

		// New drops spawn around the point 25 units in front of the camera
		TVector4 vForward( pCamera->GetMatrix().m_f31, pCamera->GetMatrix().m_f32, pCamera->GetMatrix().m_f33, 1.0f );
		vForward.Multiply( 25.0f );

		TVector4 vSpawnPosition( vForward.x + rCameraPosition.x, 0.0f, vForward.z + rCameraPosition.z, rCameraPosition.w );

		while ( m_fRainSpawnTimer < 0.0f )
		{
			if ( TINT( m_llActiveRainDrops.Count() ) >= iMaxActiveDrops || TINT( m_llFreeRainDrops.Count() ) <= 0 )
				break;

			m_fRainSpawnTimer += m_fRainSpawnInterval;

			RainDrop* pDrop = m_llFreeRainDrops.RemoveHead();
			pDrop->Reset( vSpawnPosition, this );
			m_llActiveRainDrops.InsertHead( pDrop );
		}
	}

	for ( auto it = m_llActiveRainDrops.Begin(); it != m_llActiveRainDrops.End(); it++ )
		it->Render( m_fDeltaTime, rCameraPosition, this );

	if ( ACollisionManager::IsSingletonCreated() && ATerrainInterface::IsSingletonCreated() )
	{
		for ( auto it = m_llActiveRainSplashes.Begin(); it != m_llActiveRainSplashes.End(); it++ )
			it->Render( m_fDeltaTime, rCameraPosition, this );
	}
}

void ARainRenderer::CreateMaterials()
{
	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\MiscTextures\\Weather.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pMaterialLibrary );

	m_pRainMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
	m_pRainMaterial->Create( ASysMaterial::BLENDMODE_1 );
	m_pRainMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
	m_pRainMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "World\\Rain.tga" ) );
	m_pRainMaterial->SetTextureNum( 1 );
	m_pRainMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );

	m_pRainSplashMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
	m_pRainSplashMaterial->Create( ASysMaterial::BLENDMODE_1 );
	m_pRainSplashMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
	m_pRainSplashMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "World\\Rain_splash.tga" ) );
	m_pRainSplashMaterial->SetTextureNum( 1 );
}

void ARainRenderer::DestroyMaterials()
{
	if ( m_pRainMaterial )
		m_pRainMaterial->Destroy();

	if ( m_pRainSplashMaterial )
		m_pRainSplashMaterial->Destroy();

	if ( m_pMaterialLibrary )
	{
		AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( m_pMaterialLibrary );
		AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pMaterialLibrary );
	}
}

void ARainRenderer::CreateRainDropPool()
{
	for ( TINT i = 0; i < m_iMaxRainDrops; i++ )
		m_llFreeRainDrops.InsertTail( new RainDrop( this ) );
}

void ARainRenderer::CreateRainSplashPool()
{
	for ( TINT i = 0; i < m_iMaxRainSplashes; i++ )
		m_llFreeRainSplashes.InsertTail( new RainSplash( this ) );
}

// NOTE: the TNodeList<>::DeleteAll instances (0060a830, 0060a880) are marked in TNodeList.h
