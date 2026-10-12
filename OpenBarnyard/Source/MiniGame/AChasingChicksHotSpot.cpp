#include "pch.h"
#include "AChasingChicksGame.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "GUI/AGUI2MiniMap.h"
#include "Render/ASysShader/ASysShader.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASkinShader/ASkinMaterial.h"
#include "Render/AStaticInstanceShader/AStaticInstanceMesh.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "Units/AChasingChicksChick.h"
#include "Units/AUnitManager.h"
#include "UnitMovement/Steering/ASteer.h"

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif

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

AChasingChicksGame* AChasingChicksGame::ms_pCurrent = TNULL;

ASysMaterial* AChasingChicksHotSpot::ms_apMaterials[ MESH_NUMOF ];
TINT          AChasingChicksHotSpot::ms_iNumInstances;

// Colours of the kinds of pens (RGB), 0x0072218c
static constexpr TUINT8 s_aKindColours[ 4 ][ 3 ] = {
	{ 140, 40, 124 },
	{ 255, 175, 25 },
	{ 70, 190, 30 },
	{ 60, 150, 224 },
};

// $Barnyard: FUNCTION 0050e5d0
AChasingChicksHotSpot::AChasingChicksHotSpot()
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

// $Barnyard: FUNCTION 0050c810
AChasingChicksHotSpot::~AChasingChicksHotSpot()
{
	for ( TINT i = 0; i < MESH_NUMOF; i++ )
	{
		if ( apMeshes[ i ] )
		{
			apMeshes[ i ]->DestroyResource();
			apMeshes[ i ] = TNULL;
		}
	}

	// The materials are shared by all pens
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
// NOTE: inlined three times by AChasingChicksHotSpot::Create
static ASysMesh* CreateQuadMesh( ASysMaterial*& a_rpMaterial, const TCHAR* a_szTexture )
{
	TBOOL     bFailed = TFALSE;
	ASysMesh* pMesh   = ASysShader::GetSingleton()->CreateMesh( TNULL );

	if ( pMesh && pMesh->CreatePools( ASysMesh::FLAGS_DEFAULT, 4, 4 ) )
	{
		if ( !a_rpMaterial )
		{
			a_rpMaterial = ASysShader::GetSingleton()->CreateMaterial( TNULL );

			// NOTE: the material is kept even if its creation failed (the following pens use it)
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

// $Barnyard: FUNCTION 0050d810
void AChasingChicksHotSpot::Create( AChasingChicksGame* a_pGame, TINT a_iArea )
{
	pGame = a_pGame;
	iArea = a_iArea;

	AInteriorNetwork* pNetwork = a_pGame->pScoringNetwork;
	AInteriorArea*    pArea    = &pNetwork->m_pINetwork->m_pAreas[ a_iArea ];

	pArea->GetPosition( vPosition );
	vPosition.y -= 0.25f;

	TMatrix44 mTransform = TMatrix44::IDENTITY;

	// The kind of the pen is told by the name of its area
	const TPString8& rcName = *pArea->m_strName;

	if ( rcName == TPS8( Goal1 ) )
	{
		fRadius = a_pGame->pGameData->aChickKinds[ 1 ].fPenRadius;
		iValue  = a_pGame->pGameData->aChickKinds[ 1 ].iValue;
		iKind   = 1;
	}
	else if ( rcName == TPS8( Goal2 ) )
	{
		fRadius = a_pGame->pGameData->aChickKinds[ 2 ].fPenRadius;
		iValue  = a_pGame->pGameData->aChickKinds[ 2 ].iValue;
		iKind   = 2;
	}
	else if ( rcName == TPS8( Goal3 ) )
	{
		fRadius = a_pGame->pGameData->aChickKinds[ 3 ].fPenRadius;
		iValue  = a_pGame->pGameData->aChickKinds[ 3 ].iValue;
		iKind   = 3;
	}
	else
	{
		fRadius = a_pGame->pGameData->aChickKinds[ 0 ].fPenRadius;
		iValue  = a_pGame->pGameData->aChickKinds[ 0 ].iValue;

		// The angle from the pen up to the goal of the same name (8 units lower)
		AInteriorGoal* pGoal = a_pGame->pScoringNetwork->FindGoalInArea( *pArea->m_strName, -1 );

		if ( pGoal )
		{
			TVector4 vToGoal;
			pGoal->GetPosition( vToGoal );
			vToGoal.x = vToGoal.x - vPosition.x;
			vToGoal.y = vToGoal.y - 8.0f - vPosition.y;
			vToGoal.z = vToGoal.z - vPosition.z;

			// The elevation (the horizontal distance, 006c7520)
			fThrowAngle = TMath::Abs( TMath::ATan( vToGoal.y / vToGoal.MagnitudeXZ() ) );
		}

		iKind = 0;
	}

	a_pGame->aClosedPens[ iKind ].PushBack( this );

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

// $Barnyard: FUNCTION 0050d530
TBOOL AChasingChicksHotSpot::SetClosed( TBOOL a_bClosed )
{
	if ( a_bClosed == bClosed )
		return TFALSE;

	Remove();

	if ( a_bClosed )
		pGame->aClosedPens[ iKind ].PushBack( this );
	else
		pGame->aOpenPens[ iKind ].PushBack( this );

	bClosed = a_bClosed;
	return TTRUE;
}

// $Barnyard: FUNCTION 0050c7f0
void AChasingChicksHotSpot::SetState( STATE_ a_eState )
{
	eState = a_eState;

	if ( a_eState == STATE_SCORED )
		fScoredTime = 1.0f;
}

// Fills a_pVertices/a_pIndices with a horizontal quad of the half size a_fSize transformed by a_rcTransform.
// NOTE: inlined twice by AChasingChicksHotSpot::Render
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

// $Barnyard: FUNCTION 0050d610
// $Barnyard: FUNCTION 0050c900
void AChasingChicksHotSpot::Render( TFLOAT a_fDeltaTime )
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

// $Barnyard: FUNCTION 0050d620
// $Barnyard: FUNCTION 0050ce40
void AChasingChicksHotSpot::PostRender( TFLOAT a_fDeltaTime )
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

// $Barnyard: FUNCTION 0050d2d0
void AChasingChicksHotSpot::Update( TFLOAT a_fDeltaTime, TINT a_iIcon )
{
	if ( a_iIcon > -1 )
	{
		// 0x007835b8
		static TUINT32 s_aIconColours[ 4 ] = { 0xFFDF40C5, 0xFFFFAF19, 0xFF46BE1E, 0xFF3C96E0 };

		pGame->pMap->SetIconTransform( a_iIcon, vPosition, TVector4::VEC_POSX );
		pGame->pMap->SetIconColour( a_iIcon, s_aIconColours[ iKind ] );
		pGame->pMap->SetIconVisible( a_iIcon, pGame->aPlayers[ 0 ].eState == AChasingChicksPlayer::STATE_CARRY );
	}
}

// $Barnyard: FUNCTION 0050d370
AChasingChicksChick* AChasingChicksGame::FindChickNear( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfRadius )
{
	TFLOAT               fClosestSq = a_rfRadius * a_rfRadius;
	AChasingChicksChick* pClosest   = TNULL;

	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( a_rcPosition, a_rfRadius, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	if ( iNumFound == 0 )
		return TNULL;

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit && pUnit->IsA( &TGetClass( AChasingChicksChick ) ) )
		{
			AChasingChicksChick::STATE eState = TSTATICCAST( AChasingChicksChick, pUnit )->GetState();

			if ( eState >= AChasingChicksChick::STATE_FREE && ( eState <= AChasingChicksChick::STATE_FLEEING || eState == AChasingChicksChick::STATE_JUMPING ) )
			{
				TVector4 vToChick;
				pUnit->GetSteer()->CopyPosition( vToChick );
				vToChick.x -= a_rcPosition.x;
				vToChick.y -= a_rcPosition.y;
				vToChick.z -= a_rcPosition.z;

				TFLOAT fChickRadius = pUnit->GetSteer()->GetRadius();
				TFLOAT fDistanceSq  = vToChick.MagnitudeSq() - fChickRadius * fChickRadius;

				if ( fDistanceSq < fClosestSq )
				{
					fClosestSq = fDistanceSq;
					pClosest   = TSTATICCAST( AChasingChicksChick, pUnit );
				}
			}
		}
	}

	if ( pClosest )
		a_rfRadius = TMath::Sqrt( fClosestSq );

	return pClosest;
}
