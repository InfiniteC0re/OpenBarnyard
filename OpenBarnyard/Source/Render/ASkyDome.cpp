#include "pch.h"
#include "ASkyDome.h"

#include "Assets/AMaterialLibrary.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysShader.h"

#include <Math/TMathInline.h>
#include <Render/TRenderInterface.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>
#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0042c960
ASkyDome::ASkyDome()
{
	m_iNumSomeMeshes    = 0;
	m_iNumMeshes2       = 0;
	m_ppDomeMeshes      = TNULL;
	m_pSkyLibrary       = TNULL;
	m_pVertexData       = TNULL;
	m_pStarsInstance    = TNULL;
	m_eFlags            = 6;
	m_iRings            = 0;
	m_iSides            = 0;
	m_fRotation         = 1.0f;
	m_uiUnk174          = 0;
	m_fUnk178           = 0.05f;
	m_iTexturedSkyIndex = 0;
	m_iPrevSkyIndex     = 0;
	m_fHeight           = 0;
	m_fRadius           = 0.0f;
	m_fUnk15c           = 1.0f;
	m_fUnk160           = 1.0f;
	m_fUnk164           = 1.0f;
	m_fUnk168           = 1.0f;
	m_fStarsOpacity     = 1.0f;
	m_bIsDay            = TFALSE;
	m_bNeedsUpdate      = TTRUE;

	m_oColorMatrix.m_f41 = 1.0f;
	m_oColorMatrix.m_f42 = 1.0f;
	m_oColorMatrix.m_f43 = 1.0f;
	m_oColorMatrix.m_f44 = 1.0f;
	m_oColorMatrix.m_f21 = 1.0f;
	m_oColorMatrix.m_f22 = 1.0f;
	m_oColorMatrix.m_f23 = 1.0f;
	m_oColorMatrix.m_f24 = 1.0f;
	m_oColorMatrix.m_f31 = 1.0f;
	m_oColorMatrix.m_f32 = 1.0f;
	m_oColorMatrix.m_f33 = 1.0f;
	m_oColorMatrix.m_f34 = 1.0f;
	m_oColorMatrix.m_f11 = 1.0f;
	m_oColorMatrix.m_f12 = 1.0f;
	m_oColorMatrix.m_f13 = 1.0f;
	m_oColorMatrix.m_f14 = 1.0f;

	m_fTargetBlend = 0.0f;
	m_fBlend       = 0.0f;

	m_oLightMatrix       = m_oColorMatrix;
	m_oTargetLightMatrix = m_oColorMatrix;

	m_oColorMatrix.m_f41 = 1.0f;
	m_oColorMatrix.m_f42 = 0.0f;
	m_oColorMatrix.m_f43 = 0.0f;
	m_oColorMatrix.m_f44 = 1.0f;
	m_oColorMatrix.m_f21 = 0.0f;
	m_oColorMatrix.m_f22 = 1.0f;
	m_oColorMatrix.m_f23 = 0.0f;
	m_oColorMatrix.m_f24 = 1.0f;
	m_oColorMatrix.m_f31 = 1.0f;
	m_oColorMatrix.m_f32 = 0.0f;
	m_oColorMatrix.m_f33 = 0.0f;
	m_oColorMatrix.m_f34 = 1.0f;
	m_oColorMatrix.m_f11 = 0.0f;
	m_oColorMatrix.m_f12 = 1.0f;
	m_oColorMatrix.m_f13 = 0.0f;
	m_oColorMatrix.m_f14 = 1.0f;

	m_oTargetColorMatrix = m_oColorMatrix;

	m_vSunDirection.x = 0.16156033f;
	m_vSunDirection.y = -0.14003125f;
	m_vSunDirection.z = 0.82599306f;
	m_vSunDirection.w = 0.0f;
	m_vSunDirection.Normalise();

	m_fSunMin = 4.0f;
	m_fSunMax = 20.0f;

	ResetMaterials();
}

// $Barnyard: FUNCTION 0042cad0
ASkyDome::~ASkyDome()
{
	DestroyMeshes();
	DestroyResources();
}

// $Barnyard: FUNCTION 0042ced0
void ASkyDome::Create( TFLOAT a_fRadius, TFLOAT a_fHeight, TINT a_iSides, TINT a_iRings )
{
	m_iSides  = a_iSides;
	m_fHeight = a_fHeight;
	m_iRings  = a_iRings;
	m_fRadius = a_fRadius;

	m_pVertexData = new DomeVertex[ a_iSides * a_iRings ];
	GenerateDomeVertices( a_fRadius, a_fHeight );

	ASysMaterial* pUntexturedSky = ASysShader::GetSingleton()->CreateMaterial( TNULL );
	m_pUntexturedSky             = pUntexturedSky;
	pUntexturedSky->SetBlendMode( ASysMaterial::BLENDMODE_UNTEXTURED_SKY );
	pUntexturedSky->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
	pUntexturedSky->SetTextureNum( 0 );
	pUntexturedSky->SetName( "UntexturedSky" );

	m_iNumSomeMeshes = 1;
	m_iNumMeshes2    = 1;
	m_ppDomeMeshes   = new ASysMesh*[ m_iNumMeshes2 ];

	for ( TINT i = 0; i < TEXTURE_NUMOF; i++ )
	{
		ASysMaterial* pMat            = ASysShader::GetSingleton()->CreateMaterial( TNULL );
		m_apTexturedSkyMaterials[ i ] = pMat;
		pMat->SetBlendMode( ASysMaterial::BLENDMODE_TEXTURED_SKY );
		pMat->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
		pMat->SetName( "TexturedSky" );
		pMat->SetTextureNum( 0 );
	}

	for ( TINT i = 0; i < m_iNumMeshes2; i++ )
		m_ppDomeMeshes[ i ] = TNULL;

	CreateMesh1();
	CreateMesh2();

	m_iPrevSkyIndex     = 1;
	m_iTexturedSkyIndex = 0;

	m_pSkyLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\Sky\\sky.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pSkyLibrary );

	m_apSkyTextures[ TEXTURE_NIGHT ] = AMaterialLibraryManager::GetSingleton()->FindTexture( "sky_night_16.tga" );
	m_apSkyTextures[ TEXTURE_DAY ]   = AMaterialLibraryManager::GetSingleton()->FindTexture( "sky_sunny_16.tga" );

	m_apTexturedSkyMaterials[ TEXTURE_NIGHT ]->SetTexture( m_apSkyTextures[ TEXTURE_NIGHT ] );
	m_apTexturedSkyMaterials[ TEXTURE_NIGHT ]->SetTextureNum( 1 );
	m_apTexturedSkyMaterials[ TEXTURE_DAY ]->SetTexture( m_apSkyTextures[ TEXTURE_DAY ] );
	m_apTexturedSkyMaterials[ TEXTURE_DAY ]->SetTextureNum( 1 );
}

// $Barnyard: FUNCTION 0042c670
void ASkyDome::Render1()
{
	if ( ( m_eFlags & 2 ) != 0 )
		Render1_Impl();
}

// $Barnyard: FUNCTION 0042c680
void ASkyDome::Render2()
{
	if ( ( m_eFlags & 2 ) != 0 )
		Render2_Impl();
}

// $Barnyard: FUNCTION 0042c6a0
void ASkyDome::SetSkyColors( TFLOAT a_fDuration, const TMatrix44* a_pColorMatrix, const TMatrix44* a_pLightMatrix, TFLOAT a_fTargetBlend, TINT a_iNewSkyIndex, TFLOAT a_fElapsed )
{
	if ( m_bIsDay )
	{
		TFLOAT fRatio = m_fTransitionElapsed / m_fTransitionDuration;
		m_oColorMatrix.Lerp( m_oColorMatrix, m_oTargetColorMatrix, fRatio );
		m_oLightMatrix.Lerp( m_oLightMatrix, m_oTargetLightMatrix, fRatio );
		m_fBlend = TMath::LERP( m_fBlend, m_fTargetBlend, fRatio );
	}

	m_fTransitionDuration = a_fDuration;
	m_fTransitionElapsed  = a_fElapsed;
	m_oTargetColorMatrix  = *a_pColorMatrix;
	m_oTargetLightMatrix  = *a_pLightMatrix;
	m_bIsDay              = TTRUE;
	m_bNeedsUpdate        = TTRUE;
	m_iPrevSkyIndex       = m_iTexturedSkyIndex;
	m_fTargetBlend        = a_fTargetBlend;
	m_iTexturedSkyIndex   = a_iNewSkyIndex;

	if ( a_fDuration <= a_fElapsed )
	{
		m_oColorMatrix = m_oTargetColorMatrix;
		m_bIsDay       = TFALSE;
		m_oLightMatrix = m_oTargetLightMatrix;
		m_fBlend       = a_fTargetBlend;
	}
}

// $Barnyard: FUNCTION 0042ce20
void ASkyDome::SetDirty()
{
	m_bNeedsUpdate = TTRUE;
}

// $Barnyard: FUNCTION 0042c580
void ASkyDome::GenerateDomeVertices( TFLOAT a_fRadius, TFLOAT a_fHeight )
{
	m_fHeight = a_fHeight;
	m_fRadius = a_fRadius;

	TFLOAT fSideStep = TMath::TWO_PI / m_iSides;
	TINT   iVertex   = 0;

	for ( TINT iRing = 0; iRing < m_iRings; iRing++ )
	{
		TFLOAT fRingRatio = iRing / (TFLOAT)( m_iRings - 1 );
		TFLOAT fCosRing   = TMath::Cos( fRingRatio * TMath::HALF_PI );
		TFLOAT fAngle     = 0.0f;

		for ( TINT iSide = 0; iSide < m_iSides; iSide++ )
		{
			m_pVertexData[ iVertex ].vPosition.x = TMath::Sin( fAngle ) * ( fCosRing * a_fRadius );
			m_pVertexData[ iVertex ].vPosition.y = TMath::Cos( fAngle ) * ( fCosRing * a_fRadius );
			m_pVertexData[ iVertex ].vPosition.z = fRingRatio * m_fHeight;

			fAngle += fSideStep;
			iVertex++;
		}
	}
}

// $Barnyard: FUNCTION 0042cbf0
void ASkyDome::ResetMaterials()
{
	m_apTexturedSkyMaterials[ TEXTURE_DAY ]   = TNULL;
	m_apTexturedSkyMaterials[ TEXTURE_NIGHT ] = TNULL;
	m_apSkyTextures[ TEXTURE_DAY ]            = TNULL;
	m_apSkyTextures[ TEXTURE_NIGHT ]          = TNULL;
}

// $Barnyard: FUNCTION 0042c8d0
void ASkyDome::DestroyMeshes()
{
	for ( TINT i = 0; i < m_iNumSomeMeshes; i++ )
	{
		if ( m_ppDomeMeshes[ i ] )
		{
			m_ppDomeMeshes[ i ]->DestroyResource();
			m_ppDomeMeshes[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0042ce30
void ASkyDome::DestroyResources()
{
	if ( m_pFancyDome )
		m_pFancyDome->DestroyResource();

	delete[] m_ppDomeMeshes;
	delete[] m_pVertexData;

	if ( m_pSkyLibrary )
	{
		AMaterialLibraryManager::GetSingleton()->UnloadTexturesOfLibrary( m_pSkyLibrary );
		AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pSkyLibrary );
	}

	if ( m_pUntexturedSky )
	{
		m_pUntexturedSky->Destroy();
	}

	for ( TINT i = 0; i < TEXTURE_NUMOF; i++ )
	{
		if ( m_apTexturedSkyMaterials[ i ] )
		{
			m_apTexturedSkyMaterials[ i ]->Destroy();
		}
	}
}

// $Barnyard: FUNCTION 0042def0
void ASkyDome::CreateMesh1()
{
	ASysMesh* pMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );
	m_pFancyDome    = pMesh;

	if ( pMesh )
	{
		TUINT16 uSides = (TUINT16)m_iSides;
		TUINT16 uRings = (TUINT16)m_iRings;

		if ( pMesh->CreatePools( 100, ( uSides + 1 ) * (TUINT)uRings + 1, ( uRings - 1 ) * ( uSides + 1 ) * 2 ) )
		{
			pMesh->SetMaterial( m_apTexturedSkyMaterials[ TEXTURE_DAY ] );
		}
	}
}

// $Barnyard: FUNCTION 0042da10
void ASkyDome::CreateMesh2()
{
	TUINT16 uSides = (TUINT16)m_iSides;
	TUINT16 uRings = (TUINT16)m_iRings;

	ASysMesh* pMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );
	*m_ppDomeMeshes = pMesh;

	if ( pMesh )
	{
		pMesh->CreatePools( 82, ( uSides + 1 ) * (TUINT)uRings + 1, ( uRings - 1 ) * ( uSides + 1 ) * 2 );
	}
}

// $Barnyard: FUNCTION 0042d5d0
void ASkyDome::DrawFancyDome( const TVector4& a_rColour1, const TVector4& a_rColour2, const TVector4& a_rColour3, const TVector4& a_rColour4, ASysMaterial* a_pMaterial, TFLOAT a_fOpacity, TINT a_iMode )
{
	const TFLOAT fSideUVStep  = 1.0f / m_iSides;
	const TFLOAT fHeightScale = 1.0f / m_fHeight;
	const TUINT  uiRingStride = TUINT16( m_iSides ) + 1;
	const TUINT  uiNumRings   = TUINT16( m_iRings );
	const TUINT  uiNumSides   = TUINT16( m_iSides );

	const TUINT uiNumVertices = uiNumRings * uiRingStride + 1;
	const TUINT uiNumIndices  = ( uiNumRings - 1 ) * uiRingStride * 2;

	m_pFancyDome->SetMaterial( a_pMaterial );

	ASysMesh::LockBuffer oLockBuffer;
	m_pFancyDome->Lock( oLockBuffer );

	auto pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	auto pIndices  = oLockBuffer.IndexLock.pBuffer;

	TUINT uiVertexIndex = 0;
	TUINT uiIndexIndex  = 0;

	for ( TUINT uiRing = 0; uiRing < uiNumRings; uiRing++ )
	{
		ASysMesh::Vertex* pRingStart = &pVertices[ uiVertexIndex ];

		for ( TUINT uiSide = 0; uiSide < uiNumSides; uiSide++ )
		{
			const TUINT       uiSourceVertexIndex = uiRing * uiNumSides + uiSide;
			const DomeVertex& rDomeVertex         = m_pVertexData[ uiSourceVertexIndex ];

			ASysMesh::Vertex& rVertex = pVertices[ uiVertexIndex ];
			rVertex.vPosition         = rDomeVertex.vPosition;

			TFLOAT fHorizontalBlend = uiSide / (TFLOAT)m_iSides;

			if ( fHorizontalBlend > 0.5f )
			{
				fHorizontalBlend = 1.0f - fHorizontalBlend;
			}

			TFLOAT fHeight = rDomeVertex.vPosition.z;

			if ( fHeight <= 0.0f )
			{
				fHeight = 0.0f;
			}

			TVector4 oHorizontalColour1;
			TVector4 oHorizontalColour2;
			TVector4 oColour;

			oHorizontalColour1.Lerp4( a_rColour1, a_rColour3, fHorizontalBlend + fHorizontalBlend );
			oHorizontalColour2.Lerp4( a_rColour2, a_rColour4, fHorizontalBlend + fHorizontalBlend );
			oColour.Lerp4( oHorizontalColour1, oHorizontalColour2, fHeight / m_fHeight );

			TUINT uiR = (TUINT)( oColour.x * 255.0f );
			TUINT uiG = (TUINT)( oColour.y * 255.0f );
			TUINT uiB = (TUINT)( oColour.z * 255.0f );

			rVertex.uiColor = TCOLOR4( uiR, uiG, uiB, 255 );
			rVertex.vUV.x   = ( uiSide % ( m_iSides + 1 ) ) * fSideUVStep - 0.00390625f;
			rVertex.vUV.y   = 1.0f - 1.5f * rDomeVertex.vPosition.z * fHeightScale + 0.2f;

			TMath::Clip( rVertex.vUV.y, 0.005859375f, 0.994140625f );

			uiVertexIndex++;
		}

		ASysMesh::Vertex& rSeamVertex = pVertices[ uiVertexIndex ];
		rSeamVertex                   = *pRingStart;
		rSeamVertex.vUV.x             = 0.99609375f;

		if ( uiRing < uiNumRings - 1 )
		{
			for ( TUINT uiSide = 0; uiSide < uiNumSides; uiSide++ )
			{
				pIndices[ uiIndexIndex++ ] = TIndexType( uiRing * uiRingStride + uiSide );
				pIndices[ uiIndexIndex++ ] = TIndexType( ( uiRing + 1 ) * uiRingStride + uiSide );
			}

			pIndices[ uiIndexIndex++ ] = TIndexType( uiVertexIndex );
			pIndices[ uiIndexIndex++ ] = TIndexType( uiRingStride + uiVertexIndex );
		}

		uiVertexIndex++;
	}

	m_pFancyDome->Unlock( uiNumVertices, uiNumIndices );
	m_pFancyDome->Validate();
	m_pFancyDome->Render();
}

// $Barnyard: FUNCTION 0042da70
void ASkyDome::UpdateCachedMeshData( TINT a_iSkyIndex )
{
	const TFLOAT fSideUVStep  = 1.0f / m_iSides;
	const TFLOAT fHeightScale = 1.0f / m_fHeight;
	const TUINT  uiRingStride = TUINT16( m_iSides ) + 1;
	const TUINT  uiNumRings   = TUINT16( m_iRings );
	const TUINT  uiNumSides   = TUINT16( m_iSides );

	const TUINT uiNumVertices = uiNumRings * uiRingStride + 1;
	const TUINT uiNumIndices  = ( uiNumRings - 1 ) * uiRingStride * 2;

	ASysMaterial* pMaterial = m_apTexturedSkyMaterials[ a_iSkyIndex ];
	pMaterial->SetTexture( m_apSkyTextures[ a_iSkyIndex ] );
	pMaterial->SetTextureNum( 1 );

	ASysMesh* pMesh = *m_ppDomeMeshes;
	pMesh->SetMaterial( pMaterial );

	ASysMesh::LockBuffer oLockBuffer;
	pMesh->Lock( oLockBuffer );

	auto pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	auto pIndices  = oLockBuffer.IndexLock.pBuffer;

	TUINT uiVertexIndex = 0;
	TUINT uiIndexIndex  = 0;

	for ( TUINT uiRing = 0; uiRing < uiNumRings; uiRing++ )
	{
		ASysMesh::Vertex* pRingStart = &pVertices[ uiVertexIndex ];

		for ( TUINT uiSide = 0; uiSide < uiNumSides; uiSide++ )
		{
			const TUINT       uiSourceVertexIndex = uiRing * uiNumSides + uiSide;
			const DomeVertex& rDomeVertex         = m_pVertexData[ uiSourceVertexIndex ];

			ASysMesh::Vertex& rVertex = pVertices[ uiVertexIndex ];
			rVertex.vPosition         = rDomeVertex.vPosition;

			TFLOAT fHorizontalBlend = uiSide / TFLOAT( m_iSides );

			if ( fHorizontalBlend > 0.5f )
			{
				fHorizontalBlend = 1.0f - fHorizontalBlend;
			}

			TFLOAT fHeight = rDomeVertex.vPosition.z;

			if ( fHeight <= 0.0f )
			{
				fHeight = 0.0f;
			}

			TVector4 oHorizontalColour1;
			TVector4 oHorizontalColour2;
			TVector4 oColour;

			oHorizontalColour1.Lerp4(
			    m_oTargetColorMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
			    m_oTargetColorMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
			    fHorizontalBlend + fHorizontalBlend
			);

			oHorizontalColour2.Lerp4(
			    m_oTargetColorMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
			    m_oTargetColorMatrix.AsBasisVector4( BASISVECTOR_UP ),
			    fHorizontalBlend + fHorizontalBlend
			);

			oColour.Lerp4( oHorizontalColour1, oHorizontalColour2, fHeight / m_fHeight );

			TUINT uiR = (TUINT)( oColour.x * 255.0f );
			TUINT uiG = (TUINT)( oColour.y * 255.0f );
			TUINT uiB = (TUINT)( oColour.z * 255.0f );

			rVertex.uiColor = TCOLOR4( uiR, uiG, uiB, 255 );
			rVertex.vUV.x   = ( uiSide % ( m_iSides + 1 ) ) * fSideUVStep - 0.00390625f;
			rVertex.vUV.y   = 1.0f - 1.5f * rDomeVertex.vPosition.z * fHeightScale + 0.2f;

			TMath::Clip( rVertex.vUV.y, 0.005859375f, 0.994140625f );

			uiVertexIndex++;
		}

		ASysMesh::Vertex& rSeamVertex = pVertices[ uiVertexIndex ];
		rSeamVertex                   = *pRingStart;
		rSeamVertex.vUV.x             = 0.99609375f;

		if ( uiRing < uiNumRings - 1 )
		{
			for ( TUINT uiSide = 0; uiSide < uiNumSides; uiSide++ )
			{
				pIndices[ uiIndexIndex++ ] = TIndexType( uiRing * uiRingStride + uiSide );
				pIndices[ uiIndexIndex++ ] = TIndexType( ( uiRing + 1 ) * uiRingStride + uiSide );
			}

			pIndices[ uiIndexIndex++ ] = TIndexType( uiVertexIndex );
			pIndices[ uiIndexIndex++ ] = TIndexType( uiRingStride + uiVertexIndex );
		}

		uiVertexIndex++;
	}

	pMesh->Unlock( uiNumVertices, uiNumIndices );
	pMesh->Validate();
}

// $Barnyard: FUNCTION 0042d0a0
void ASkyDome::Render1_Impl()
{
	if ( m_bNeedsUpdate )
	{
		UpdateCachedMeshData( m_iTexturedSkyIndex );
		m_bNeedsUpdate = TFALSE;
	}

	TRenderContext* pRenderContext  = TRenderInterface::GetSingleton()->GetCurrentContext();
	TMatrix44       oOldModelView   = pRenderContext->GetModelViewMatrix();
	TMatrix44       oSkyModelView   = oOldModelView;
	TVector4&       rSkyTranslation = oSkyModelView.GetTranslation();

	rSkyTranslation = TVector4::VEC_ZERO;

	TMatrix44 oBaseSkyModelView = oSkyModelView;
	TMatrix44 oSkyTransform;
	oSkyTransform.m_f11 = 1.0f;
	oSkyTransform.m_f12 = 0.0f;
	oSkyTransform.m_f13 = 0.0f;
	oSkyTransform.m_f14 = 0.0f;
	oSkyTransform.m_f21 = 0.0f;
	oSkyTransform.m_f22 = 0.0f;
	oSkyTransform.m_f23 = 1.0f;
	oSkyTransform.m_f24 = 0.0f;
	oSkyTransform.m_f31 = 0.0f;
	oSkyTransform.m_f32 = -1.0f;
	oSkyTransform.m_f33 = 0.0f;
	oSkyTransform.m_f34 = 0.0f;
	oSkyTransform.m_f41 = 0.0f;
	oSkyTransform.m_f42 = 0.0f;
	oSkyTransform.m_f43 = 0.0f;
	oSkyTransform.m_f44 = 1.0f;

	oSkyModelView.Multiply( oBaseSkyModelView, oSkyTransform );

	TMatrix44 oSkyModelViewScaled = oSkyModelView;
	oSkyModelViewScaled.Scale( 1.0f, 1.0f, -1.0f );

	TMatrix44 oSkyModelViewRotated = oSkyModelView;
	oSkyModelViewRotated.RotateZ( m_fRotation );

	TMatrix44 oSkyModelViewRotatedScaled = oSkyModelViewRotated;
	oSkyModelViewRotatedScaled.Scale( 1.0f, 1.0f, -1.0f );

	pRenderContext->SetModelViewMatrix( oSkyModelView );

	if ( m_bIsDay )
	{
		//-----------------------------------------------------------------------------
		// Day
		//-----------------------------------------------------------------------------
		// 1. Render cloud layers at up and down
		//-----------------------------------------------------------------------------

		TFLOAT fRatio = m_fTransitionElapsed / m_fTransitionDuration;
		TMath::Clip( fRatio, 0.0f, 1.0f );

		TMatrix44 oColorMatrix;
		TMatrix44 oLightMatrix;
		oColorMatrix.Lerp( m_oColorMatrix, m_oTargetColorMatrix, fRatio );
		oLightMatrix.Lerp( m_oLightMatrix, m_oTargetLightMatrix, fRatio );

		TFLOAT fBlend = TMath::LERP( m_fBlend, m_fTargetBlend, fRatio );

		pRenderContext->SetModelViewMatrix( oSkyModelViewRotated );

		if ( m_iTexturedSkyIndex < m_iPrevSkyIndex )
		{
			// Blend current->prev

			DrawFancyDome(
			    oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
			    m_apTexturedSkyMaterials[ m_iTexturedSkyIndex ],
			    fBlend,
			    2
			);

			DrawFancyDome(
			    oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
			    m_apTexturedSkyMaterials[ m_iPrevSkyIndex ],
			    1.0f - fBlend,
			    2
			);
		}
		else
		{
			// Blend prev->current

			DrawFancyDome(
			    oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
			    m_apTexturedSkyMaterials[ m_iPrevSkyIndex ],
			    1.0f - fBlend,
			    2
			);

			DrawFancyDome(
			    oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
			    oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
			    m_apTexturedSkyMaterials[ m_iTexturedSkyIndex ],
			    fBlend,
			    2
			);
		}

		pRenderContext->SetModelViewMatrix( oSkyModelViewRotatedScaled );
		DrawFancyDome(
		    oColorMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_UP ),
		    m_pUntexturedSky,
		    1.0f,
		    0
		);

		//-----------------------------------------------------------------------------
		// 2. Render dome at up and down
		//-----------------------------------------------------------------------------
		pRenderContext->SetModelViewMatrix( oSkyModelView );
		DrawFancyDome(
		    oColorMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
		    oColorMatrix.AsBasisVector4( BASISVECTOR_UP ),
		    m_pUntexturedSky,
		    1.0f,
		    0
		);
	}
	else
	{
		//-----------------------------------------------------------------------------
		// Night
		//-----------------------------------------------------------------------------
		// 1. Render cloud layers at up and down
		//-----------------------------------------------------------------------------
		pRenderContext->SetModelViewMatrix( oSkyModelViewRotatedScaled );
		DrawFancyDome(
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
		    m_apTexturedSkyMaterials[ m_iTexturedSkyIndex ],
		    1.0f,
		    1
		);

		pRenderContext->SetModelViewMatrix( oSkyModelViewRotated );
		DrawFancyDome(
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_FORWARD ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_RIGHT ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ),
		    m_oLightMatrix.AsBasisVector4( BASISVECTOR_UP ),
		    m_apTexturedSkyMaterials[ m_iTexturedSkyIndex ],
		    1.0f,
		    1
		);

		//-----------------------------------------------------------------------------
		// 2. Render dome at up and down
		//-----------------------------------------------------------------------------
		pRenderContext->SetModelViewMatrix( oSkyModelViewScaled );

		for ( TINT i = 0; i < m_iNumMeshes2; i++ )
		{
			if ( m_ppDomeMeshes[ i ] )
			{
				m_ppDomeMeshes[ i ]->SetMaterial( m_pUntexturedSky );
				m_ppDomeMeshes[ i ]->Render();
			}
		}

		pRenderContext->SetModelViewMatrix( oSkyModelView );

		for ( TINT i = 0; i < m_iNumMeshes2; i++ )
		{
			if ( m_ppDomeMeshes[ i ] )
			{
				m_ppDomeMeshes[ i ]->SetMaterial( m_pUntexturedSky );
				m_ppDomeMeshes[ i ]->Render();
			}
		}
	}

	pRenderContext->SetModelViewMatrix( oOldModelView );

	if ( m_pStarsInstance && 0.0f < m_fStarsOpacity )
	{
		m_pStarsInstance->SetSkeletonUpdating( TTRUE );
		m_pStarsInstance->GetSceneObject()->EnableUnknown1();
		m_pStarsInstance->GetSceneObject()->GetSomeVector2().y = m_fStarsOpacity;

		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
		m_pStarsInstance->GetTransform().SetTranslate( pCamera->GetMatrix().GetTranslation3() );
		m_pStarsInstance->GetSceneObject()->RenderIfVisible();
		m_pStarsInstance->SetSkeletonUpdating( TFALSE );
	}
}

// $Barnyard: FUNCTION 0042cc10
void ASkyDome::Render2_Impl()
{
	TRenderContext* pRenderContext  = TRenderInterface::GetSingleton()->GetCurrentContext();
	TMatrix44       oOldModelView   = pRenderContext->GetModelViewMatrix();
	TMatrix44       oSkyModelView   = oOldModelView;
	TVector4&       rSkyTranslation = oSkyModelView.GetTranslation();

	rSkyTranslation = TVector4::VEC_ZERO;

	TMatrix44 oBaseSkyModelView = oSkyModelView;
	TMatrix44 oSkyTransform;
	oSkyTransform.m_f11 = 1.0f;
	oSkyTransform.m_f12 = 0.0f;
	oSkyTransform.m_f13 = 0.0f;
	oSkyTransform.m_f14 = 0.0f;
	oSkyTransform.m_f21 = 0.0f;
	oSkyTransform.m_f22 = 0.0f;
	oSkyTransform.m_f23 = 1.0f;
	oSkyTransform.m_f24 = 0.0f;
	oSkyTransform.m_f31 = 0.0f;
	oSkyTransform.m_f32 = -1.0f;
	oSkyTransform.m_f33 = 0.0f;
	oSkyTransform.m_f34 = 0.0f;
	oSkyTransform.m_f41 = 0.0f;
	oSkyTransform.m_f42 = 0.0f;
	oSkyTransform.m_f43 = 0.0f;
	oSkyTransform.m_f44 = 1.0f;

	oSkyModelView.Multiply( oBaseSkyModelView, oSkyTransform );

	TMatrix44 oSkyModelViewScaled = oSkyModelView;
	oSkyModelViewScaled.Scale( 1.0f, 1.0f, -1.0f );

	TMatrix44 oSkyModelViewRotated = oSkyModelView;
	oSkyModelViewRotated.RotateZ( m_fRotation );

	pRenderContext->SetModelViewMatrix( oSkyModelView );

	if ( !m_bIsDay )
	{
		if ( m_bNeedsUpdate )
		{
			UpdateCachedMeshData( m_iTexturedSkyIndex );
			m_bNeedsUpdate = TFALSE;
		}
	}
	else
	{
		TFLOAT fRatio = m_fTransitionElapsed / m_fTransitionDuration;

		TMatrix44 oColorMatrix;
		TMatrix44 oLightMatrix;
		oColorMatrix.Lerp( m_oColorMatrix, m_oTargetColorMatrix, fRatio );
		oLightMatrix.Lerp( m_oLightMatrix, m_oTargetLightMatrix, fRatio );

		TMath::LERP( m_fBlend, m_fTargetBlend, fRatio );
	}

	// ??????????????????????????????????
	// Is this something console only????

	pRenderContext->SetModelViewMatrix( oOldModelView );
}
