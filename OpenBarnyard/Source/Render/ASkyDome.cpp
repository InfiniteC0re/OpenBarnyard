#include "pch.h"
#include "ASkyDome.h"

#include "Assets/AMaterialLibrary.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Animation/AModelRepos.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysShader.h"

#include <Math/TMathInline.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>

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
	m_ppMesh2           = TNULL;
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
	pUntexturedSky->SetBlendMode( ASysMaterial::BLENDMODE_8 );
	pUntexturedSky->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
	pUntexturedSky->SetTextureNum( 0 );
	pUntexturedSky->SetName( "UntexturedSky" );

	m_iNumSomeMeshes = 1;
	m_iNumMeshes2    = 1;
	m_ppMesh2        = new ASysMesh*[ m_iNumSomeMeshes ];

	for ( TINT i = 0; i < TEXTURE_NUMOF; i++ )
	{
		ASysMaterial* pMat            = ASysShader::GetSingleton()->CreateMaterial( TNULL );
		m_apTexturedSkyMaterials[ i ] = pMat;
		pMat->SetBlendMode( ASysMaterial::BLENDMODE_9 );
		pMat->SetFlags( TMaterial::FLAGS_BLENDING, TTRUE );
		pMat->SetName( "TexturedSky" );
		pMat->SetTextureNum( 0 );
	}

	for ( TINT i = 0; i < m_iNumSomeMeshes; i++ )
		m_ppMesh2[ i ] = TNULL;

	CreateMesh1();
	CreateMesh2();

	m_iPrevSkyIndex     = 1;
	m_iTexturedSkyIndex = 0;

	m_pSkyLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\Sky\\sky.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pSkyLibrary );

	m_pNightTexture = AMaterialLibraryManager::GetSingleton()->FindTexture( "sky_night_16.tga" );
	m_pSunnyTexture = AMaterialLibraryManager::GetSingleton()->FindTexture( "sky_sunny_16.tga" );

	m_apTexturedSkyMaterials[ TEXTURE_NIGHT ]->SetTexture( m_pNightTexture );
	m_apTexturedSkyMaterials[ TEXTURE_NIGHT ]->SetTextureNum( 1 );
	m_apTexturedSkyMaterials[ TEXTURE_DAY ]->SetTexture( m_pSunnyTexture );
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
	m_apTexturedSkyMaterials[ 0 ] = TNULL;
	m_apTexturedSkyMaterials[ 1 ] = TNULL;
	m_pSunnyTexture               = TNULL;
	m_pNightTexture               = TNULL;
}

// $Barnyard: FUNCTION 0042c8d0
void ASkyDome::DestroyMeshes()
{
	for ( TINT i = 0; i < m_iNumSomeMeshes; i++ )
	{
		if ( m_ppMesh2[ i ] )
		{
			m_ppMesh2[ i ]->DestroyResource();
			m_ppMesh2[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 0042ce30
void ASkyDome::DestroyResources()
{
	if ( m_pMesh1 )
		m_pMesh1->DestroyResource();

	delete[] m_ppMesh2;
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
	m_pMesh1        = pMesh;

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
	*m_ppMesh2      = pMesh;

	if ( pMesh )
	{
		pMesh->CreatePools( 0x52, ( uSides + 1 ) * (TUINT)uRings + 1, ( uRings - 1 ) * ( uSides + 1 ) * 2 );
	}
}

// $Barnyard: FUNCTION 0042da70
void ASkyDome::UpdateMeshData( TINT a_iSkyIndex )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 0042d0a0
void ASkyDome::Render1_Impl()
{
	TIMPLEMENT();

	if ( m_bNeedsUpdate )
	{
		UpdateMeshData( m_iTexturedSkyIndex );
		m_bNeedsUpdate = TFALSE;
	}
}

// $Barnyard: FUNCTION 0042cc10
void ASkyDome::Render2_Impl()
{
	TIMPLEMENT();

	if ( m_bIsDay && m_bNeedsUpdate )
	{
		UpdateMeshData( m_iTexturedSkyIndex );
		m_bNeedsUpdate = TFALSE;
	}
}
