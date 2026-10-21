#include "pch.h"
#include <Render/TCameraObject.h>
#include "AChickenCoopDefenderGame.h"
#include "AChickenCoopDefender.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "GUI/AGUI2MiniMap.h"
#include "Render/ASysShader/ASysShader.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASkinShader/ASkinMaterial.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMesh.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#include "Render/ABackend.h"

#include <Render/TRenderInterface.h>
#include <GUI/T2GUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( Goal1 );
TPSTRING8_DECLARE( Goal2 );
TPSTRING8_DECLARE( Goal3 );

TVector4 AChickenCoopDefenderGame::ms_aGoals[ NUM_GOALS ];

ASysMaterial* AChickenCoopDefenderHotSpot::ms_apMaterials[ MESH_NUMOF ];
TINT          AChickenCoopDefenderHotSpot::ms_iNumInstances;

// Colours of the kinds of hot spots (RGB), 0x007225e0
static constexpr TUINT8 s_aKindColours[ 4 ][ 3 ] = {
	{ 140, 40, 124 },
	{ 255, 175, 25 },
	{ 70, 190, 30 },
	{ 60, 150, 224 },
};

//-----------------------------------------------------------------------------
// AChickenCoopDefenderHotSpot
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00516040
AChickenCoopDefenderHotSpot::AChickenCoopDefenderHotSpot()
{
	eState      = STATE_IDLE;
	fScoredTime = 0.0f;
	fRadius     = 0.0f;
	iValue      = 0;
	bClosed     = TTRUE;
	iKind       = -1;
	iArea       = -1;
	vPosition   = TVector4::VEC_ZERO;
	fThrowAngle = 0.0f;
	fSpin       = 0.0f;
	pGame       = TNULL;

	for ( TINT i = 0; i < MESH_NUMOF; i++ )
		apMeshes[ i ] = TNULL;

	pModel = TNULL;
	ms_iNumInstances++;
}

// $Barnyard: FUNCTION 005129b0
AChickenCoopDefenderHotSpot::~AChickenCoopDefenderHotSpot()
{
	for ( TINT i = 0; i < MESH_NUMOF; i++ )
	{
		if ( apMeshes[ i ] )
		{
			apMeshes[ i ]->DestroyResource();
			apMeshes[ i ] = TNULL;
		}
	}

	// The materials are shared by all hot spots
	if ( --ms_iNumInstances == 0 )
	{
		for ( TINT i = 0; i < MESH_NUMOF; i++ )
		{
			if ( ms_apMaterials[ i ] )
			{
				ms_apMaterials[ i ]->Destroy();
				ms_apMaterials[ i ] = TNULL;
			}
		}
	}

	if ( pModel )
	{
		g_pModelRepos->DestroyModelInstance( pModel );
		pModel = TNULL;
	}
}

// Creates a quad mesh using the shared material a_rpMaterial (created with the texture a_szTexture if there's none yet).
// NOTE: inlined three times by AChickenCoopDefenderHotSpot::Create (same as AChasingChicksHotSpot)
static ASysMesh* CreateQuadMesh( ASysMaterial*& a_rpMaterial, const TCHAR* a_szTexture )
{
	TBOOL     bFailed = TFALSE;
	ASysMesh* pMesh   = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( pMesh && pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 4, 4 ) )
	{
		if ( !a_rpMaterial )
		{
			a_rpMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );

			// NOTE: the material is kept even if its creation failed (the following hot spots use it)
			if ( !a_rpMaterial || !a_rpMaterial->Create( ASysMaterial::BLENDMODE_1 ) )
			{
				bFailed = TTRUE;
			}
			else
			{
				a_rpMaterial->SetTextureNum( 1 );
				a_rpMaterial->SetTexture( T2GUI::GetRenderer()->GetTexture( a_szTexture ) );
			}
		}

		pMesh->SetMaterial( a_rpMaterial );

		if ( !bFailed )
			return pMesh;
	}

	if ( pMesh )
		pMesh->DestroyResource();

	return TNULL;
}

// $Barnyard: FUNCTION 00513540
void AChickenCoopDefenderHotSpot::Create( AChickenCoopDefenderGame* a_pGame, TINT a_iArea )
{
	pGame = a_pGame;
	iArea = a_iArea;

	AInteriorNetwork* pNetwork = a_pGame->pScoringNetwork;
	AInteriorArea*    pArea    = &pNetwork->m_pINetwork->m_pAreas[ a_iArea ];

	pArea->GetPosition( vPosition );
	vPosition.y -= 0.25f;

	TMatrix44 mTransform = TMatrix44::IDENTITY;

	// The kind of the hot spot is told by the name of its area
	// NOTE: the kinds are the other way around than in Chasing Chicks
	const TPString8& rcName = *pArea->m_strName;

	if ( rcName == TPS8( Goal3 ) )
	{
		fRadius = a_pGame->pGameData->aHotSpotKinds[ 1 ].fRadius;
		iValue  = a_pGame->pGameData->aHotSpotKinds[ 1 ].iValue;
		iKind   = 1;
	}
	else if ( rcName == TPS8( Goal2 ) )
	{
		fRadius = a_pGame->pGameData->aHotSpotKinds[ 2 ].fRadius;
		iValue  = a_pGame->pGameData->aHotSpotKinds[ 2 ].iValue;
		iKind   = 2;
	}
	else if ( rcName == TPS8( Goal1 ) )
	{
		fRadius = a_pGame->pGameData->aHotSpotKinds[ 3 ].fRadius;
		iValue  = a_pGame->pGameData->aHotSpotKinds[ 3 ].iValue;
		iKind   = 3;
	}
	else
	{
		fRadius = a_pGame->pGameData->aHotSpotKinds[ 0 ].fRadius;
		iValue  = a_pGame->pGameData->aHotSpotKinds[ 0 ].iValue;

		// The angle from the hot spot up to the goal of the same name (8 units lower)
		AInteriorGoal* pGoal = a_pGame->pScoringNetwork->FindGoalInArea( *pArea->m_strName, -1 );

		if ( pGoal )
		{
			TVector4 vToGoal;
			pGoal->GetPosition( vToGoal );
			vToGoal.x = vToGoal.x - vPosition.x;
			vToGoal.y = vToGoal.y - 8.0f - vPosition.y;
			vToGoal.z = vToGoal.z - vPosition.z;

			fThrowAngle = TMath::Abs( TMath::ATan( vToGoal.y / vToGoal.MagnitudeXZ() ) );
		}

		iKind = 0;
	}

	a_pGame->aClosedHotSpots[ iKind ].PushBack( this );

	// NOTE: retail builds this transform but never uses it
	TFLOAT fScale = fRadius * 0.5f;
	mTransform.Scale( fScale, fScale, fScale );
	mTransform.SetTranslation( vPosition );

	apMeshes[ MESH_RING ]  = CreateQuadMesh( ms_apMaterials[ MESH_RING ], "HUD\\Golf_Ball_Ring.tga" );
	apMeshes[ MESH_GLOW ]  = CreateQuadMesh( ms_apMaterials[ MESH_GLOW ], "HUD\\Context_Glow.tga" );
	apMeshes[ MESH_ARROW ] = CreateQuadMesh( ms_apMaterials[ MESH_ARROW ], "HUD\\Chick_Arrow.tga" );

	pModel = g_pModelRepos->InstantiateNewModel( TPS8D( "data\\models\\ccd_hotspotmarker" ), TNULL );

	if ( pModel )
	{
		pModel->SetUnlit();
		pModel->GetSceneObject()->GetTransform().SetMode( TTransformObject::Mode::Matrix );
		pModel->SetGlow( TTRUE );

		TModelInstance* pInstance = pModel->GetSceneObject()->GetInstance();
		TMaterial*      pMaterial = pInstance->GetModel()->GetLOD( pInstance->GetLOD() ).ppMeshes[ 0 ]->GetMaterial();
		pMaterial->SetFlags( TMaterial::FLAGS_NO_CULL, TFALSE );

#ifdef TRENDERINTERFACE_DX8
		if ( pMaterial->IsA( &TGetClass( ASkinMaterialHAL ) ) )
			TSTATICCAST( ASkinMaterial, pMaterial )->SetBlendMode( ASkinMaterial::BLENDMODE_ALPHA );
#endif
	}
}

// $Barnyard: FUNCTION 00513440
TBOOL AChickenCoopDefenderHotSpot::SetClosed( TBOOL a_bClosed )
{
	if ( a_bClosed == bClosed )
		return TFALSE;

	Remove();

	if ( a_bClosed )
	{
		// Flashes
		fScoredTime = 1.0f;
		eState      = STATE_SCORED;
		pGame->aClosedHotSpots[ iKind ].PushBack( this );
	}
	else
	{
		pGame->aOpenHotSpots[ iKind ].PushBack( this );
	}

	bClosed = a_bClosed;
	return TTRUE;
}

void AChickenCoopDefenderHotSpot::SetState( STATE_ a_eState )
{
	eState = a_eState;

	if ( a_eState == STATE_SCORED )
		fScoredTime = 1.0f;
}

// Fills a_pVertices/a_pIndices with a horizontal quad of the half size a_fSize transformed by a_rcTransform.
// NOTE: inlined twice by AChickenCoopDefenderHotSpot::Render (same as AChasingChicksHotSpot)
static void FillQuad( ASysMesh::Vertex* a_pVertices, TIndexType* a_pIndices, const TMatrix44& a_rcTransform, TFLOAT a_fSize, TUINT32 a_uiColour )
{
	static constexpr TFLOAT s_aCorners[ 4 ][ 2 ] = {
		{ -1.0f, 1.0f },
		{ -1.0f, -1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, -1.0f },
	};

	for ( TINT i = 0; i < 4; i++ )
	{
		TVector4 vCorner( s_aCorners[ i ][ 0 ] * a_fSize, 0.0f, s_aCorners[ i ][ 1 ] * a_fSize, 1.0f );
		TVector4 vTransformed;
		TMatrix44::TransformVector( vTransformed, a_rcTransform, vCorner );

		a_pVertices[ i ].vPosition.x = vTransformed.x;
		a_pVertices[ i ].vPosition.y = vTransformed.y;
		a_pVertices[ i ].vPosition.z = vTransformed.z;
		a_pVertices[ i ].uiColor     = a_uiColour;
		a_pVertices[ i ].vUV.x       = TFLOAT( i / 2 );
		a_pVertices[ i ].vUV.y       = TFLOAT( i % 2 );
	}

	a_pIndices[ 0 ] = 0;
	a_pIndices[ 1 ] = 1;
	a_pIndices[ 2 ] = 2;
	a_pIndices[ 3 ] = 3;
}

// $Barnyard: FUNCTION 00513520
// $Barnyard: FUNCTION 00512a70
void AChickenCoopDefenderHotSpot::Render( TFLOAT a_fDeltaTime )
{
	const TUINT8* pColour  = s_aKindColours[ iKind ];
	TUINT32       uiColour = 0xFFFFFFFF;

	switch ( eState )
	{
		case STATE_IDLE:
		case STATE_INSIDE:
			uiColour = 0xFF000000 | ( pColour[ 0 ] << 16 ) | ( pColour[ 1 ] << 8 ) | pColour[ 2 ];
			break;
		case STATE_SCORED:
			// Fades out
			uiColour = ( TUINT32( TUINT8( TUINT32( fScoredTime * 255.0f ) ) ) << 24 ) | ( pColour[ 0 ] << 16 ) | ( pColour[ 1 ] << 8 ) | pColour[ 2 ];
			break;
	}

	TMatrix44 mTransform;

	// The ring spins, faster while flashing
	if ( ASysMesh* pMesh = apMeshes[ MESH_RING ] )
	{
		ASysMesh::LockBuffer oLockBuffer;
		pMesh->Lock( oLockBuffer );

		switch ( eState )
		{
			case STATE_IDLE:
			case STATE_INSIDE:
				fSpin += TMath::TWO_PI * a_fDeltaTime * 0.25f;
				break;
			case STATE_SCORED:
				fSpin += ( 2.0f - fScoredTime ) * TMath::TWO_PI * a_fDeltaTime;
				break;
		}

		mTransform.Identity();
		TMatrix44::TransformVector( mTransform.GetTranslation(), mTransform, vPosition );
		mTransform.RotateY( fSpin );

		FillQuad( TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] ), oLockBuffer.IndexLock.pBuffer, mTransform, fRadius, uiColour );

		pMesh->Unlock( 4, 4 );
		pMesh->Validate();
		pMesh->Render();
	}

	// The glow pulses, grows while flashing
	if ( ASysMesh* pMesh = apMeshes[ MESH_GLOW ] )
	{
		ASysMesh::LockBuffer oLockBuffer;
		pMesh->Lock( oLockBuffer );

		mTransform.Identity();
		TMatrix44::TransformVector( mTransform.GetTranslation(), mTransform, vPosition );

		TFLOAT fScale;

		switch ( eState )
		{
			case STATE_IDLE:
			case STATE_INSIDE:
				fScale = TMath::Sin( TMath::TWO_PI * *pGame->pElapsedTime ) * 0.25f + 0.5f;
				mTransform.Scale( fScale, fScale, fScale );
				break;
			case STATE_SCORED:
				fScale = 1.5f - fScoredTime;
				mTransform.Scale( fScale, fScale, fScale );
				break;
		}

		FillQuad( TREINTERPRETCAST( ASysMesh::Vertex*, oLockBuffer.VertexLock.apStreams[ 0 ] ), oLockBuffer.IndexLock.pBuffer, mTransform, fRadius, uiColour );

		pMesh->Unlock( 4, 4 );
		pMesh->Validate();
		pMesh->Render();
	}
}

// $Barnyard: FUNCTION 00513530
// $Barnyard: FUNCTION 00512fb0
void AChickenCoopDefenderHotSpot::PostRender( TFLOAT a_fDeltaTime )
{
	if ( pModel )
	{
		const TUINT8* pColour = s_aKindColours[ iKind ];
		TVector4      vColour( pColour[ 0 ] * ( 1.0f / 255.0f ), pColour[ 1 ] * ( 1.0f / 255.0f ), pColour[ 2 ] * ( 1.0f / 255.0f ), 1.0f );

		TRenderInterface* pRender  = TRenderInterface::GetSingleton();
		TRenderContext*   pContext = pRender->GetCurrentContext();

		// Lit in the colour of the kind
		TMatrix44 mOldLightColour = pRender->GetLightColour();
		TVector4  vOldAmbient     = pContext->GetAmbientColour();

		TMatrix44 mLightColour;
		mLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) = vColour;
		mLightColour.AsBasisVector4( BASISVECTOR_UP ).Set( 0.0f, 0.0f, 0.0f, 0.0f );
		mLightColour.AsBasisVector4( BASISVECTOR_FORWARD ).Set( 0.0f, 0.0f, 0.0f, 0.0f );
		mLightColour.AsBasisVector4( BASISVECTOR_TRANSLATION ).Set( 0.0f, 0.0f, 0.0f, 0.0f );

		TFLOAT fScale = pGame->pHotSpotData->fScale * fRadius;

		TMatrix44 mTransform;
		mTransform.Identity();
		TMatrix44::TransformVector( mTransform.GetTranslation(), mTransform, vPosition );
		mTransform.Scale( fScale, pGame->pHotSpotData->fHeight, fScale );

		pRender->SetLightColourMatrix( mLightColour );
		pContext->SetAmbientColour( vColour );

		TSceneObject* pSceneObject = pModel->GetSceneObject();
		pSceneObject->GetTransform().SetMatrix( mTransform );

		// Fades in with the distance to the camera
		TFLOAT fDistance = TVector4::DistanceXZ( ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(), vPosition ) - pGame->pHotSpotData->fFadeStart;
		TFLOAT fOpacity  = TMath::Max( fDistance, 0.0f ) / pGame->pHotSpotData->fFadeRange;

		pSceneObject->GetFadeParams().y = TMath::Min( fOpacity, pGame->pHotSpotData->fMaxOpacity );
		pSceneObject->EnableOpacity();

		TModelLOD& rLOD                  = pSceneObject->GetInstance()->GetModel()->GetLOD( 0 );
		TBOOL      bIsStaticInstanceMesh = rLOD.iNumMeshes > 0 && rLOD.ppMeshes[ 0 ]->IsA( &TGetClass( AStaticInstanceMesh ) );

		// The light comes from the camera
		TMatrix44 mOldLightDirection = pRender->GetLightDirection();
		TMatrix44 mLightDirection    = mOldLightDirection;

		const TMatrix44& rcCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		TVector4&        rLight   = mLightDirection.AsBasisVector4( BASISVECTOR_RIGHT );
		rLight.x                  = -rcCamera.m_f31 + rcCamera.m_f11 + rcCamera.m_f21;
		rLight.y                  = -rcCamera.m_f32 + rcCamera.m_f12 + rcCamera.m_f22;
		rLight.z                  = -rcCamera.m_f33 + rcCamera.m_f13 + rcCamera.m_f23;
		rLight.Normalise();

		// ... in the space of the scaled model (static instance meshes aren't scaled)
		if ( !bIsStaticInstanceMesh )
		{
			rLight.x *= fScale;
			rLight.y *= pGame->pHotSpotData->fHeight;
			rLight.z *= fScale;
		}

		pRender->SetLightDirectionMatrix( mLightDirection );

		pSceneObject->EnableSkeletonUpdate();
		pSceneObject->Render( 0, vPosition.AsVector3() );
		pSceneObject->DisableSkeletonUpdate();

		pRender->SetLightDirectionMatrix( mOldLightDirection );
		pSceneObject->DisableOpacity();

		pContext->SetAmbientColour( vOldAmbient );
		pRender->SetLightColourMatrix( mOldLightColour );
	}

	if ( eState == STATE_SCORED )
	{
		fScoredTime -= a_fDeltaTime;

		if ( fScoredTime < 0.0f )
		{
			fScoredTime = 0.0f;
			eState      = STATE_IDLE;
		}
	}
}

// $Barnyard: FUNCTION 005128f0
void AChickenCoopDefenderHotSpot::Update( TFLOAT a_fDeltaTime, TINT a_iIcon )
{
	if ( a_iIcon > -1 )
	{
		// 0x007836a8
		static TUINT32 s_aIconColours[ 4 ] = { 0xFFDF40C5, 0xFFFFAF19, 0xFF46BE1E, 0xFF3C96E0 };

		pGame->pMap->SetIconTransform( a_iIcon, vPosition, TVector4::VEC_POSX );
		pGame->pMap->SetIconColour( a_iIcon, s_aIconColours[ iKind ] );

		TBOOL bCarrying = pGame->ePlayerState == AChickenCoopDefender::PLAYERSTATE_CARRY_HEN || pGame->ePlayerState == AChickenCoopDefender::PLAYERSTATE_CARRY_EGG;
		pGame->pMap->SetIconVisible( a_iIcon, bCarrying );
	}
}

//-----------------------------------------------------------------------------
// AChickenCoopDefenderGame
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00517cb0
AChickenCoopDefenderGame::AChickenCoopDefenderGame( TFLOAT* a_pElapsedTime, AGUI2TargetCounter* a_pCounter, AGUI2LifeHud* a_pLifeHud, AGUI2MiniMap* a_pMap )
    : ePhase( 0 )
    , iUnk4( 0 )
    , iNumThieves( 0 )
    , iNumHensLost( 0 )
    , iNumHensCarried( 0 )
    , iUnk14( 0 )
    , iNumEggsCarried( 0 )
    , iNumEggsLost( 0 )
    , iNumRaccoonHits( 0 )
    , iNumHits( 0 )
    , iBonus( 0 )
    , iWave( 0 )
    , iNumThievesAtCoop( 0 )
    , iUnk34( 0 )
    , iLives( 0 )   // NOTE: not initialised by retail
    , iNumLeft( 0 ) // NOTE: not initialised by retail
    , pCounter( a_pCounter )
    , pLifeHud( a_pLifeHud )
    , ePlayerState( 0 )
    , pNearThief( TNULL )
    , pItem( TNULL )
    , iThrowTarget( -1 )
    , vThrowDirection( TVector4::VEC_POSX )
    , pHotSpot( TNULL )
    , fHitCooldown( -0.1f )
    , bDefeated( TFALSE )
    , bPlayerHit( TFALSE )
    , pThiefData( TNULL )
    , pElapsedTime( a_pElapsedTime )
    , pMap( a_pMap )
    , fTime( 0.0f )
    , pHanna( TNULL )
    , pHotSpots( TNULL )
    , pMapIcons( TNULL )
    , pScoringNetwork( TNULL )
{
}

// $Barnyard: FUNCTION 00517ff0
AChickenCoopDefenderGame::~AChickenCoopDefenderGame()
{
}

void AChickenCoopDefenderGame::ResetCamera()
{
	AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
	pHelper->SetTarget( oPlayer );
	pHelper->SetHeightOffset( 1.7f );
	pHelper->SetDefaultPitch( 11.0f );
	pHelper->SetTargetPitch( 11.0f );
	pHelper->SetDefaultDistance( 5.0f );
	pHelper->SetMaxPitch( 22.0f );
	pHelper->SetDefaultMaxPitch( 22.0f );
	pHelper->SetCollisionRadius( 0.43f );
	pHelper->SetSmallCollisionRadius( 0.2f );
	pHelper->ResetPosition( TTRUE );

	ePhase = AChickenCoopDefender::PHASE_PLAYING;
}
