#include "pch.h"
#include "ASunFlareEffect.h"
#include "Render/ASysShader/ASysShader.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"
#include "Cameras/ACameraManager.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AGameLoader.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionManager.h"

#include <Render/TRenderInterface.h>
#include <Render/TTextureFactory.h>
#include <Math/TQuaternion.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// DAT_0079b32c: offset of the inner points of the flare towards the sun, never written so it's always 0
static TFLOAT s_fFlareOffset = 0.0f;

// $Barnyard: FUNCTION 0060edd0
ASunFlareEffect::ASunFlareEffect()
{
	m_pSunMesh       = TNULL;
	m_pSunMaterial   = TNULL;
	m_pFlareMesh     = TNULL;
	m_pFlareMaterial = TNULL;
	m_pMoonMesh      = TNULL;
	m_pMoonMaterial  = TNULL;
	m_bEnabled       = TTRUE;
	m_bSunVisible    = TFALSE;
	m_bSunUnoccluded = TFALSE;
	m_bUseTime       = TFALSE;
	m_fSunSize       = 0.1f;
	m_fMoonSize      = 0.2f;
	m_fFadeTime      = 0.0f;
	m_fSunIntensity  = 1.0f;
	m_fMoonIntensity = 1.0f;
	m_fFadeOutTime   = 0.15f;
	m_fFadeSpeed     = 1.0f / 0.15f;
	m_fMaxGlowTime   = 0.8f;
	m_fUnk74         = 1.25f;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail leaves these uninitialised
	m_iUnk58       = 0;
	m_fGlowTime    = 0.0f;
	m_bMoonVisible = TFALSE;
	m_iHour        = 0;
	m_iMinute      = 0;
	m_iSecond      = 0;
#endif

	// Default direction of the sun
	TQuaternion quatSun( -0.046243802f, -0.477332f, -0.873416f, -0.084616303f );
	quatSun.AsVector4().Divide4( TMath::Sqrt( 0.9999997629005224f ) );

	TVector3 vecRotated;
	TQuaternion::RotateVector( vecRotated, quatSun, TVector3( 0.0f, 0.0f, 1.0f ) );

	m_vecSunDir.x = vecRotated.x;
	m_vecSunDir.y = vecRotated.y;
	m_vecSunDir.z = vecRotated.z;
	m_vecSunDir.w = 0.0f;

	m_vecMoonDir    = m_vecSunDir;
	m_vecSunriseDir = m_vecSunDir;
	m_vecNoonDir    = m_vecSunDir;

	m_uiIgnoreGroups = 0x80000;

	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
	    "Data\\MiscTextures\\SunflaretexWin.ttl",
	    AAssetLoader::GetAssetTRB( AAssetType_Startup )
	);

	TTextureFactory* pTextureFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );

	TINT iSunTexture    = m_pMaterialLibrary->FindTextureIndex( "sun.tga" );
	TINT iCoronaTexture = m_pMaterialLibrary->FindTextureIndex( "corona.tga" );
	TINT iMoonTexture   = m_pMaterialLibrary->FindTextureIndex( "moon.tga" );

	TTexture* pSunTexture    = pTextureFactory->CreateFromT2Texture( m_pMaterialLibrary->GetTexture( iSunTexture )->pTexture );
	TTexture* pCoronaTexture = pTextureFactory->CreateFromT2Texture( m_pMaterialLibrary->GetTexture( iCoronaTexture )->pTexture );
	TTexture* pMoonTexture   = pTextureFactory->CreateFromT2Texture( m_pMaterialLibrary->GetTexture( iMoonTexture )->pTexture );

	if ( !m_pSunMaterial )
	{
		m_pSunMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
		m_pSunMaterial->Create( ASysMaterial::BLENDMODE_6 );
		m_pSunMaterial->SetTextureNum( 1 );
		m_pSunMaterial->SetTexture( pSunTexture );
		m_pSunMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
	}

	if ( !m_pFlareMaterial )
	{
		m_pFlareMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
		m_pFlareMaterial->Create( ASysMaterial::BLENDMODE_6 );
		m_pFlareMaterial->SetTextureNum( 1 );
		m_pFlareMaterial->SetTexture( pCoronaTexture );
		m_pFlareMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
	}

	if ( !m_pMoonMaterial )
	{
		m_pMoonMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );
		m_pMoonMaterial->Create( ASysMaterial::BLENDMODE_6 );
		m_pMoonMaterial->SetTextureNum( 1 );
		m_pMoonMaterial->SetTexture( pMoonTexture );
		m_pMoonMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TTRUE );
	}

	m_pFlareMaterial->SetBlendMode( ASysMaterial::BLENDMODE_7 );

	ASysMaterial* pSunMaterial = m_pSunMaterial;
	m_pSunMesh                 = ASysShader::GetSingleton()->CreateMesh( TNULL );
	m_pSunMesh->CreatePools( 82, 40, 60 );
	m_pSunMesh->SetMaterial( pSunMaterial );

	ASysMaterial* pFlareMaterial = m_pFlareMaterial;
	m_pFlareMesh                 = ASysShader::GetSingleton()->CreateMesh( TNULL );
	m_pFlareMesh->CreatePools( 82, 40, 60 );
	m_pFlareMesh->SetMaterial( pFlareMaterial );

	ASysMaterial* pMoonMaterial = m_pMoonMaterial;
	m_pMoonMesh                 = ASysShader::GetSingleton()->CreateMesh( TNULL );
	m_pMoonMesh->CreatePools( 82, 40, 60 );
	m_pMoonMesh->SetMaterial( pMoonMaterial );
}

// $Barnyard: FUNCTION 0060e7a0
ASunFlareEffect::~ASunFlareEffect()
{
	AMaterialLibraryManager::List::GetSingleton()->DestroyLibrary( m_pMaterialLibrary );

	m_pSunMaterial->Destroy();
	m_pFlareMaterial->Destroy();
	m_pMoonMaterial->Destroy();

	m_pSunMesh->DestroyResource();
	m_pMoonMesh->DestroyResource();
	m_pFlareMesh->DestroyResource();
}

// $Barnyard: FUNCTION 0060f530
void ASunFlareEffect::Render()
{
	if ( !m_bEnabled || !ATerrainInterface::IsSingletonCreated() )
		return;

	if ( !( m_fSunIntensity > 0.01f || m_fMoonIntensity > 0.01f ) )
		return;

	TRenderInterface* pRender  = TRenderInterface::GetSingleton();
	TRenderContext*   pContext = pRender->GetCurrentContext();

	TMatrix44                        oOldModelView  = pContext->GetModelViewMatrix();
	TRenderContext::PROJECTIONPARAMS oOldProjParams = pContext->GetProjectionParams();
	TRenderContext::PROJECTIONPARAMS oScreenProjParams;
	oScreenProjParams.m_Centre.x  = 0.0f;
	oScreenProjParams.m_Centre.y  = 0.0f;
	oScreenProjParams.m_Proj.x    = 1.0f;
	oScreenProjParams.m_Proj.y    = 1.0f;
	oScreenProjParams.m_fNearClip = 0.05f;
	oScreenProjParams.m_fFarClip  = 200.0f;

	Update();

	TMatrix44 oIdentity = TMatrix44::IDENTITY;

	// The lens flare is in view space
	if ( m_bSunVisible == TTRUE )
	{
		pContext->SetModelViewMatrix( TMatrix44::IDENTITY );
		m_pFlareMesh->Render();
	}

	pRender->FlushOrderTables();

	// The sun and the moon are in screen space
	pContext->SetCameraMode( TRenderContext::CameraMode_Orthographic );
	pContext->SetProjectionParams( oScreenProjParams );
	pContext->Update();
	pContext->SetModelViewMatrix( oIdentity );

	if ( m_bSunVisible == TTRUE )
		m_pSunMesh->Render();

	if ( m_bMoonVisible == TTRUE )
		m_pMoonMesh->Render();

	pRender->FlushOrderTables();

	pContext->SetCameraMode( TRenderContext::CameraMode_Perspective );
	pContext->SetProjectionParams( oOldProjParams );
	pContext->Update();
	pContext->SetModelViewMatrix( oOldModelView );
}

// $Barnyard: FUNCTION 0060f110
void ASunFlareEffect::Update()
{
	if ( AGameLoader::IsMainGameLoaded() )
	{
		AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
		TINT              iHour;
		TINT              iMinute;
		TINT              iSecond;

		if ( m_bUseTime )
		{
			iHour   = m_iHour;
			iMinute = m_iMinute;
			iSecond = m_iSecond;
		}
		else
		{
			iHour   = pGameTimeManager->GetCurrentHour();
			iMinute = pGameTimeManager->GetCurrentMinute();
			iSecond = pGameTimeManager->GetCurrentSecond();
		}

		TFLOAT fHour = iMinute * ( 1.0f / 60.0f ) + iHour + iSecond * ( 1.0f / 3600.0f );

		if ( fHour <= 6.0f || fHour >= 20.0f )
		{
			m_vecSunDir = m_vecSunriseDir;
		}
		else if ( fHour < 11.0f || fHour > 15.0f )
		{
			if ( fHour < 11.0f )
			{
				// Sunrise to noon
				m_vecSunDir.Lerp4( m_vecSunriseDir, m_vecNoonDir, TMath::EaseInOut( ( fHour - 6.0f ) * 0.2f ) );
				m_vecSunDir.Normalise();
			}
			else
			{
				// Noon to sunset
				m_vecSunDir.Lerp4( m_vecNoonDir, m_vecSunriseDir, TMath::EaseInOut( ( fHour - 15.0f ) * 0.2f ) );
				m_vecSunDir.Normalise();
			}
		}
		else
		{
			m_vecSunDir = m_vecNoonDir;
		}
	}

	TVector4 vecMoonPos;
	TVector4 vecSunPos;
	TBOOL    bMoonUnoccluded;

	m_bMoonVisible = GetScreenPosition( vecMoonPos, m_vecMoonDir, &bMoonUnoccluded, TFALSE, 260.0f, 260.0f );
	m_bSunVisible  = GetScreenPosition( vecSunPos, m_vecSunDir, &m_bSunUnoccluded, TTRUE, 100.0f, 100.0f );

	TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();

	if ( m_bSunUnoccluded == TTRUE )
	{
		m_fGlowTime += fDeltaTime;

		if ( m_fGlowTime >= m_fMaxGlowTime )
			m_fGlowTime = m_fMaxGlowTime;
	}
	else
	{
		m_fGlowTime -= fDeltaTime;

		if ( m_fGlowTime < 0.0f )
			m_fGlowTime = 0.0f;
	}

	TFLOAT fSunAlpha  = 1.0f;
	TFLOAT fMoonAlpha = m_fMoonIntensity;

	if ( m_fSunIntensity > 0.01f )
	{
		if ( !m_bSunVisible )
		{
			// Fade out
			m_fFadeTime += fDeltaTime;
			fSunAlpha = 1.0f - m_fFadeTime * m_fFadeSpeed;

			if ( fSunAlpha < 0.0f )
			{
				m_fFadeTime = m_fFadeOutTime;
				fSunAlpha   = 0.0f;
			}
		}
		else if ( m_fFadeTime > 0.0f )
		{
			// Fade in
			m_fFadeTime -= g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
			fSunAlpha = 1.0f - m_fFadeTime * m_fFadeSpeed;

			if ( fSunAlpha >= 1.0f )
			{
				fSunAlpha   = 1.0f;
				m_fFadeTime = 0.0f;
			}
		}
	}
	else
	{
		fSunAlpha = 0.0f;
	}

	if ( m_fMoonIntensity > 0.01f )
	{
		if ( !m_bSunVisible )
		{
			// The moon goes once it's far enough off the screen
			TRenderContext* pContext = TRenderInterface::GetSingleton()->GetCurrentContext();

			if ( vecMoonPos.x <= -100.0f || pContext->GetWidth() + 100.0f <= vecMoonPos.x ||
			     vecMoonPos.y <= -100.0f || pContext->GetHeight() + 100.0f <= vecMoonPos.y )
				fMoonAlpha = 0.0f;
		}
	}
	else
	{
		fMoonAlpha = 0.0f;
	}

	fSunAlpha *= m_fSunIntensity;
	fMoonAlpha *= m_fMoonIntensity;

	if ( m_bSunVisible )
		m_bSunVisible = fSunAlpha > 0.0f;

	if ( m_bMoonVisible )
		m_bMoonVisible = fMoonAlpha > 0.0f;

	if ( m_bSunVisible )
	{
		UpdateQuad( vecSunPos, fSunAlpha, TFALSE );
		UpdateFlare( vecSunPos, fSunAlpha );
	}

	if ( m_bMoonVisible )
		UpdateQuad( vecMoonPos, fMoonAlpha, TTRUE );
}

// $Barnyard: FUNCTION 0060ebe0
TBOOL ASunFlareEffect::GetScreenPosition( TVector4& a_rOutPos, const TVector4& a_rcDirection, TBOOL* a_pOutVisible, TBOOL a_bCollide, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TRenderContext* pContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	TVector4        vecFarPos  = a_rcDirection;
	const TVector4& vecCamPos  = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	TFLOAT          fRayLength = 180.0f;

	vecFarPos.Multiply( 180.0f );

	a_rOutPos.z = 0.0f;
	a_rOutPos.w = 1.0f;
	TMatrix44::RotateVector( a_rOutPos, pContext->GetWorldViewMatrix(), vecFarPos );

	// Behind the camera
	if ( a_rOutPos.z < pContext->GetProjectionParams().m_fNearClip )
		return TFALSE;

	TFLOAT fInvZ = 1.0f / a_rOutPos.z;
	a_rOutPos.x  = fInvZ * a_rOutPos.x * pContext->GetProjectionParams().m_Proj.x + pContext->GetProjectionParams().m_Centre.x;
	a_rOutPos.y  = fInvZ * a_rOutPos.y * pContext->GetProjectionParams().m_Proj.y + pContext->GetProjectionParams().m_Centre.y;
	a_rOutPos.z  = 1.0f;
	a_rOutPos.w  = 1.0f;

	*a_pOutVisible = TTRUE;

	ARayData oRay( vecCamPos, a_rcDirection, fRayLength );

	if ( a_bCollide && ACollisionManager::GetSingleton()->CollideRay( TNULL, oRay, fRayLength, m_uiIgnoreGroups, ACollisionObject::TYPE_IGNORED ) )
		*a_pOutVisible = TFALSE;

	TFLOAT fHalfWidth  = a_fWidth * 0.5f;
	TFLOAT fHalfHeight = a_fHeight * 0.5f;

	if ( -fHalfWidth < a_rOutPos.x && -fHalfHeight < a_rOutPos.y && fHalfWidth + pContext->GetWidth() > a_rOutPos.x && fHalfHeight + pContext->GetHeight() > a_rOutPos.y )
		return *a_pOutVisible;

	return TFALSE;
}

// $Barnyard: FUNCTION 0060e850
void ASunFlareEffect::UpdateQuad( const TVector4& a_rcScreenPos, TFLOAT a_fAlpha, TBOOL a_bMoon )
{
	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fSunSize  = TINT( pDisplayParams->uiWidth ) * m_fSunSize;
	TFLOAT fMoonSize = TINT( pDisplayParams->uiHeight ) * m_fMoonSize;
	TFLOAT fSize     = fMoonSize * 0.875f;

	ASysMesh* pMesh = a_bMoon ? m_pMoonMesh : m_pSunMesh;

	ASysMesh::LockBuffer oLockBuffer;
	TUtil::MemClear( &oLockBuffer, sizeof( oLockBuffer ) );
	pMesh->Lock( oLockBuffer );

	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	TUINT16*          pIndices  = oLockBuffer.IndexLock.pBuffer;

	TUINT32 uiColour;

	if ( !a_bMoon )
	{
		// The sun gets bigger towards the edges of the screen
		TVector3 vecFromCentre(
		    a_rcScreenPos.x / TINT( pDisplayParams->uiWidth ) - 0.5f,
		    a_rcScreenPos.y / TINT( pDisplayParams->uiHeight ) - 0.5f,
		    0.0f
		);

		TFLOAT fProgress = vecFromCentre.Magnitude() * 2.0f + 0.2f;
		TMath::Clip( fProgress, 0.0f, 1.0f );

		fSize = TMath::LERP( fMoonSize, fSunSize, fProgress ) * a_fAlpha;

		TUINT32 uiGrey = TUINT32( a_fAlpha * 255.0f );
		uiColour       = ( ( ( uiGrey | 0xffffff00 ) << 8 | uiGrey ) << 8 ) | uiGrey;
	}
	else
	{
		uiColour = ( TUINT32( a_fAlpha * 255.0f ) << 24 ) | 0xffffff;
	}

	TFLOAT fZ = a_rcScreenPos.z + 199.0f;

	pVertices[ 0 ].vPosition.Set( a_rcScreenPos.x + fSize, a_rcScreenPos.y - fSize, fZ );
	pVertices[ 0 ].uiColor = uiColour;
	pVertices[ 0 ].vUV.Set( 1.0f, 0.0f );

	pVertices[ 1 ].vPosition.Set( a_rcScreenPos.x - fSize, a_rcScreenPos.y - fSize, fZ );
	pVertices[ 1 ].uiColor = uiColour;
	pVertices[ 1 ].vUV.Set( 0.0f, 0.0f );

	pVertices[ 2 ].vPosition.Set( a_rcScreenPos.x + fSize, a_rcScreenPos.y + fSize, fZ );
	pVertices[ 2 ].uiColor = uiColour;
	pVertices[ 2 ].vUV.Set( 1.0f, 1.0f );

	pVertices[ 3 ].vPosition.Set( a_rcScreenPos.x - fSize, a_rcScreenPos.y + fSize, fZ );
	pVertices[ 3 ].uiColor = uiColour;
	pVertices[ 3 ].vUV.Set( 0.0f, 1.0f );

	// One quad as a strip with degenerate ends: 0 0 1 2 3 3
	TINT iNumIndices = 0;

	for ( TINT i = 0; i < 4; )
	{
		pIndices[ iNumIndices++ ] = i;
		pIndices[ iNumIndices++ ] = i;
		i++;
		pIndices[ iNumIndices++ ] = i;
		i++;
		pIndices[ iNumIndices++ ] = i;
		i++;
		pIndices[ iNumIndices++ ] = i;
		pIndices[ iNumIndices++ ] = i;
		i++;
	}

	pMesh->Unlock( 4, iNumIndices );
	pMesh->Validate();

	pMesh->SetMaterial( a_bMoon ? m_pMoonMaterial : m_pSunMaterial );
}

// $Barnyard: FUNCTION 0060e3e0
void ASunFlareEffect::UpdateFlare( const TVector4& a_rcScreenPos, TFLOAT a_fAlpha )
{
	TRenderInterface* pRender = TRenderInterface::GetSingleton();
	pRender->GetCurrentDisplayParams();

	TRenderContext* pContext = pRender->GetCurrentContext();

	ASysMesh::LockBuffer oLockBuffer;
	TUtil::MemClear( &oLockBuffer, sizeof( oLockBuffer ) );
	m_pFlareMesh->Lock( oLockBuffer );

	ASysMesh::Vertex* pVertices = TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] );
	TUINT16*          pIndices  = oLockBuffer.IndexLock.pBuffer;

	const TFLOAT fAngleStep = TMath::TWO_PI * 0.1f;

	TVector4 vecRayDir( 1.0f, 0.0f, 0.0f, 1.0f );
	TUINT32  uiColour = ( TUINT32( a_fAlpha * 255.0f ) << 24 ) | 0xe4d049;
	TFLOAT   fU       = -1.5f;

	// The sun in view space
	TVector4 vecSunView;
	TMatrix44::RotateVector( vecSunView, pContext->GetModelViewMatrix(), m_vecSunDir );

	TVector3* pRayDir2D = TREINTERPRETCAST( TVector3*, &vecRayDir );
	pRayDir2D->RotateZ( ( vecSunView.y + vecSunView.x ) * 0.6f );

	TVector4 vecInner;
	vecInner.Multiply( vecSunView, -2.0f );
	vecInner.z = -vecInner.z;

	vecSunView.Multiply( 180.0f );

	TFLOAT fX = a_rcScreenPos.x - pContext->GetWidth() * 0.5f;
	TFLOAT fY = a_rcScreenPos.y - pContext->GetHeight() * 0.5f;

	static TFLOAT s_fScaleX = 640.0f / pContext->GetWidth() * 1.6f;
	static TFLOAT s_fScaleY = 448.0f / pContext->GetHeight() * 2.3f;

	vecInner.z   = 1.6f;
	vecSunView.z = 170.0f;
	vecSunView.x = 1.0f / ( pContext->GetProjectionParams().m_Proj.x / pContext->GetProjectionParams().m_Centre.x ) * ( s_fScaleX * ( 1.0f / 3.0f ) ) * fX;
	vecSunView.y = 1.0f / ( pContext->GetProjectionParams().m_Proj.y / pContext->GetProjectionParams().m_Centre.y ) * ( s_fScaleY * ( 1.0f / 3.0f ) ) * fY;
	vecInner.x   = vecSunView.x * s_fFlareOffset;
	vecInner.y   = vecSunView.y * s_fFlareOffset;

	// The rays are longer when the sun is in the middle of the screen
	TFLOAT fLength = 0.9f - ( 1.0f - ( 1.0f - TMath::Abs( vecSunView.y / ( pContext->GetHeight() * 0.125f ) ) ) * ( 1.0f - TMath::Abs( vecSunView.x / ( pContext->GetWidth() * 0.125f ) ) ) );
	TMath::Clip( fLength, 0.0f, 1.0f );
	fLength = TMath::EaseInOut( fLength );

	TFLOAT fInnerScale = fLength * 6.0f + 4.0f;
	TINT   iNumVerts   = 0;

	for ( TINT i = 11; i != 0; i-- )
	{
		TVector4 vecPoint;

		// Inner point
		vecPoint.Multiply( vecRayDir, fInnerScale );
		vecPoint.x += vecInner.x;
		vecPoint.z = vecInner.z;
		vecPoint.y += vecInner.y;

		pVertices[ iNumVerts ].vPosition.Set( vecPoint.x, vecPoint.y, vecPoint.z );
		pVertices[ iNumVerts ].vUV.Set( fU, 0.2f );
		pVertices[ iNumVerts ].uiColor = uiColour;
		iNumVerts++;

		// Outer point
		vecPoint.Multiply( vecRayDir, 10.0f );
		vecPoint.x += vecSunView.x;
		vecPoint.z = vecSunView.z;
		vecPoint.y += vecSunView.y;

		pVertices[ iNumVerts ].vPosition.Set( vecPoint.x, vecPoint.y, vecPoint.z );
		pVertices[ iNumVerts ].vUV.Set( fU, 0.95f );
		pVertices[ iNumVerts ].uiColor = uiColour;
		iNumVerts++;

		pRayDir2D->RotateZ( fAngleStep );
		fU += 0.3f;
	}

	TINT iNumIndices = 0;

	for ( TINT i = 0; i < iNumVerts; i++ )
		pIndices[ iNumIndices++ ] = i;

	m_pFlareMesh->Unlock( iNumVerts, iNumIndices );
	m_pFlareMesh->Validate();
}
