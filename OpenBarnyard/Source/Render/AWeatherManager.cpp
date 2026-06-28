#include "pch.h"
#include "AWeatherManager.h"

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

#include <Render/TMaterial.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0060a0f0
AWeatherManager::RainDrop::RainDrop( AWeatherManager* a_pWeatherManager )
{
	m_pMesh = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( m_pMesh )
	{
		if ( m_pMesh->CreatePools( 100, 0x28, 0x3c ) )
			m_pMesh->SetMaterial( a_pWeatherManager->m_pRainMaterial );
	}

	m_pPositions = new TVector3[ 10 ];

	for ( TINT i = 0; i < 10; i++ )
		m_pPositions[ i ] = TVector3::VEC_ZERO;

	m_uiColour = 0x4cffffff;
}

// $Barnyard: FUNCTION 00608d00
AWeatherManager::RainDrop::~RainDrop()
{
	if ( m_pMesh )
		m_pMesh->DestroyResource();

	delete[] m_pPositions;
}

// $Barnyard: FUNCTION 00608d20
void AWeatherManager::RainDrop::Reset( const TVector4& a_rPosition, AWeatherManager* a_pWeatherManager )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 00609010
TBOOL AWeatherManager::RainDrop::Update( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager, TBOOL a_bForceReset )
{
	TIMPLEMENT();
	return TTRUE;
}

// $Barnyard: FUNCTION 00609380
void AWeatherManager::RainDrop::Render( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 0060a280
AWeatherManager::RainSplash::RainSplash( AWeatherManager* a_pWeatherManager )
{
	m_iNumSplashes = 5;
	m_pMesh        = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( m_pMesh )
	{
		if ( m_pMesh->CreatePools( 100, m_iNumSplashes * 8, m_iNumSplashes * 12 ) )
			m_pMesh->SetMaterial( a_pWeatherManager->m_pRainSplashMaterial );
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
AWeatherManager::RainSplash::~RainSplash()
{
	if ( m_pMesh )
		m_pMesh->DestroyResource();

	delete[] m_pPositions;
	delete[] m_pTimers;
}

// $Barnyard: FUNCTION 0060a360
void AWeatherManager::RainSplash::Reset( TFLOAT a_fRatio, AWeatherManager* a_pWeatherManager )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 006098c0
void AWeatherManager::RainSplash::Update( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 00609940
void AWeatherManager::RainSplash::Render( TFLOAT a_fDeltaTime, const TVector4& a_rCameraPosition, AWeatherManager* a_pWeatherManager )
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 0060a8f0
AWeatherManager::AWeatherManager()
{
	m_fMaxRainDrops             = 40.0f;
	m_fRainIntensity            = 0.0f;
	m_fUnkC                     = 80.0f;
	m_fRainSpawnTimer           = 0.0f;
	m_fRainSpawnInterval        = 0.0f;
	m_fRainDropWidth            = 1.0f;
	m_fRainDropLength           = 9.0f;
	m_vWindDirection.x          = 1.0f;
	m_vWindDirection.y          = 0.0f;
	m_vWindDirection.z          = 0.0f;
	m_vWindDirection.w          = 0.0f;
	m_fUnk12C                   = 0.0f;
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
AWeatherManager::~AWeatherManager()
{
	DestroyMaterials();

	m_llActiveRainSplashes.DeleteAll();
	m_llFreeRainSplashes.DeleteAll();
	m_llActiveRainDrops.DeleteAll();
	m_llFreeRainDrops.DeleteAll();
}

// $Barnyard: FUNCTION 0054c550
void AWeatherManager::SetCurrentWeather( TFLOAT a_fWeatherIntensity )
{
	m_fElapsedTime   = a_fWeatherIntensity;
	m_fRainIntensity = a_fWeatherIntensity;

	// NOTE: The original also writes 7 to DAT_00774218. The owning weather type
	// state is not identified in source yet, so it stays documented here.
}

// $Barnyard: FUNCTION 00608ce0
void AWeatherManager::SetRainDropSize( TFLOAT a_fWidth, TFLOAT a_fLength )
{
	m_fRainDropWidth  = a_fWidth;
	m_fRainDropLength = a_fLength;
}

// $Barnyard: FUNCTION 0060a3b0
void AWeatherManager::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bLockToCurrentWeatherType || !ACollisionManager::IsSingletonCreated() || !ATerrainInterface::IsSingletonCreated() )
		return;

	m_fDeltaTime  = a_fDeltaTime;
	m_uiRandomSeed = m_oRandom.RandRaw();

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	m_vRainOrigin.x  = pCamera->GetMatrix().m_f31;
	m_vRainOrigin.y  = pCamera->GetMatrix().m_f32;
	m_vRainOrigin.z  = pCamera->GetMatrix().m_f33;
	m_vRainOrigin.w  = 1.0f;
	m_vRainOrigin.y  = 0.0f;
	m_vRainOrigin.Multiply( 10.0f );
	m_vRainOrigin.x += pCamera->GetMatrix().m_f41;
	m_vRainOrigin.y += pCamera->GetMatrix().m_f42;
	m_vRainOrigin.z += pCamera->GetMatrix().m_f43;

	// TODO: Finish RainDrop/RainSplash list update once the helper methods are fully reconstructed.
}

// $Barnyard: FUNCTION 0060a5f0
void AWeatherManager::Render()
{
	if ( m_fRainIntensity <= 0.0f )
		return;

	// TODO: Finish RainDrop/RainSplash rendering once their mesh fill methods are fully reconstructed.
}

void AWeatherManager::CreateMaterials()
{
	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\MiscTextures\\Weather.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	AMaterialLibraryManager::GetSingleton()->CreateTextures( m_pMaterialLibrary );

	m_pRainMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
	m_pRainMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
	m_pRainMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "World\\Rain.tga" ) );
	m_pRainMaterial->SetTextureNum( 1 );
	m_pRainMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );

	m_pRainSplashMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
	m_pRainSplashMaterial->SetBlendMode( ASysMaterial::BLENDMODE_1 );
	m_pRainSplashMaterial->SetTexture( AMaterialLibraryManager::GetSingleton()->FindTexture( "World\\Rain_splash.tga" ) );
	m_pRainSplashMaterial->SetTextureNum( 1 );
}

void AWeatherManager::DestroyMaterials()
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

void AWeatherManager::CreateRainDropPool()
{
	for ( TINT i = 0; i < m_iMaxRainDrops; i++ )
		m_llFreeRainDrops.InsertTail( new RainDrop( this ) );
}

void AWeatherManager::CreateRainSplashPool()
{
	for ( TINT i = 0; i < m_iMaxRainSplashes; i++ )
		m_llFreeRainSplashes.InsertTail( new RainSplash( this ) );
}

// TNodeList<>::DeleteAll template:
// $Barnyard: FUNCTION 0060a830
