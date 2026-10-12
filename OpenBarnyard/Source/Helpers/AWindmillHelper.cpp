#include "pch.h"
#include "AWindmillHelper.h"
#include "AReplayManager.h"
#include "ARandom.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Render/ARenderer.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionObjectMesh.h"

#include <Render/TModel.h>

#ifdef TRENDERINTERFACE_DX8
#  include "Platform/DX8/ASkinShader/ASkinMaterial_DX8.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( windmill_tail );
TPSTRING8_DECLARE( windmill_blades );
TPSTRING8_DECLARE( windmillcowtip_tail );
TPSTRING8_DECLARE( windmillcowtip_blades );
TPSTRING8_DECLARE( weathervane_base );
TPSTRING8_DECLARE( weathervane_pointer );
TPSTRING8_DECLARE( waterwheel );

// 00783ed8, the spin speed of the windmill blades (radians per second)
static const TFLOAT s_fWindmillBladesSpeed = TMath::PI;
// 00783ee0, the turn speed of the windmill heads
static const TFLOAT s_fWindmillTurnSpeed = TMath::PI * 0.025f;
// 00783edc, the turn speed of the weather vane pointer
static const TFLOAT s_fWeatherVaneTurnSpeed = TMath::PI * 0.1f;
// 00783ee4, the spin speed of the water wheel
static const TFLOAT s_fWaterWheelSpeed = TMath::PI * 0.25f;

TBOOL ASimAnimModelHelper::IsInAnimationRange( AModelInstance* a_pModel )
{
	TVector4 vecCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	TVector4 vecModel;
	a_pModel->GetPosition( vecModel );

	TFLOAT fFar = ARenderer::GetSingleton()->GetFar();
	return TVector4::DistanceSq( vecCamera, vecModel ) - 4.0f <= fFar * fFar;
}

// Turns off the HD lighting of the skin materials of the model
// NOTE: inlined into Windmill::Create and WeatherVane::Create in retail
static void DisableHDLighting( AModelInstance* a_pModel )
{
	TModel* pModel = a_pModel->GetSceneObject()->GetModelRef()->GetModel();

	for ( TINT i = 0; i < pModel->GetNumLODs(); i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT k = 0; k < rLOD.iNumMeshes; k++ )
		{
#ifdef TRENDERINTERFACE_DX8
			TMaterial* pMaterial = rLOD.ppMeshes[ k ]->GetMaterial();

			if ( pMaterial->IsA( &TGetClass( ASkinMaterialHAL ) ) )
				TSTATICCAST( ASkinMaterialHAL, pMaterial )->SetHDLighting( TFALSE );
#endif
		}
	}
}

// Sets the distances the model fades out between
static void SetFadeDistances( AModelInstance* a_pModel, TFLOAT a_fFadeStart, TFLOAT a_fFadeEnd, TFLOAT a_fInvFadeLength )
{
	TSceneObject* pSceneObject         = a_pModel->GetSceneObject();
	pSceneObject->GetFadeDistances().x = a_fFadeStart;
	pSceneObject->GetFadeDistances().y = a_fFadeEnd;
	pSceneObject->GetFadeParams().x    = a_fInvFadeLength;
}

// $Barnyard: FUNCTION 0054c6e0
AWindmillHelper::Windmill::Windmill()
{
	Reset();
}

// NOTE: inlined into the constructor and into PopWindmill in retail
void AWindmillHelper::Windmill::Reset()
{
	pBlades      = TNULL;
	pTail        = TNULL;
	fScale       = 1.0f;
	iBladesBone  = -1;
	fBladesAngle = 0.0f;
	fYaw         = 0.0f;
	fTargetYaw   = 0.0f;
}

// $Barnyard: FUNCTION 0054cb20
void AWindmillHelper::Windmill::Create( const TCHAR* a_szLocator, const TPString8& a_rcTailType, const TPString8& a_rcBladesType, TFLOAT a_fScale, TFLOAT a_fFadeStart, TFLOAT a_fFadeEnd )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	fScale                                    = a_fScale;

	// The head
	pTail = pAnimObjManager->FindType( a_rcTailType )->Instantiate();
	DisableHDLighting( pTail );

	TMatrix44 matTransform;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_szLocator, matTransform );
	matTransform.RotateZ( ms_fWindDirection );

	fTargetYaw = ms_fWindDirection;
	fYaw       = ms_fWindDirection;

	if ( fScale != 1.0f )
		matTransform.Scale( fScale, fScale, fScale );

	pTail->SetTransform( matTransform );
	pTail->SetSkeletonUpdating( TTRUE );

	TFLOAT fInvFadeLength = 1.0f / ( a_fFadeEnd - a_fFadeStart );
	SetFadeDistances( pTail, a_fFadeStart, a_fFadeEnd, fInvFadeLength );

	// The blades, attached to the bone of the head called like the blades type
	iBladesBone = pTail->GetBoneIndex( a_rcBladesType );
	pBlades     = pAnimObjManager->FindType( a_rcBladesType )->Instantiate();

	TMatrix44 matBone;
	pTail->GetBoneTransform( iBladesBone, matBone );

	if ( fScale != 1.0f )
		matBone.Scale( fScale, fScale, fScale );

	pBlades->SetTransform( matBone );
	pBlades->SetSkeletonUpdating( TTRUE );
	SetFadeDistances( pBlades, a_fFadeStart, a_fFadeEnd, fInvFadeLength );
}

// $Barnyard: FUNCTION 0054c930
void AWindmillHelper::Windmill::Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling )
{
	if ( !pBlades->IsVisible() && !pTail->IsVisible() )
	{
		fTargetYaw = ms_fWindDirection;
		fYaw       = ms_fWindDirection;
		return;
	}

	if ( a_bDistanceCulling && !IsInAnimationRange( pTail ) )
	{
		// NOTE: retail doesn't turn the head here, so it points to the old direction once it's back in range
		fTargetYaw = ms_fWindDirection;
		fYaw       = ms_fWindDirection;
		return;
	}

	// Pick a new direction around the wind once the head got there
	if ( fTargetYaw == fYaw )
		fTargetYaw = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -0.1f, 0.1f ) + ms_fWindDirection;

	TINT iSign = ( fTargetYaw <= fYaw ) ? -1 : 1;

	TFLOAT fStep = TFLOAT( iSign ) * s_fWindmillTurnSpeed * a_fDeltaTime;
	TFLOAT fDiff = fTargetYaw - fYaw;

	if ( TMath::Abs( fStep ) > TMath::Abs( fDiff ) )
		fStep = fDiff;

	TMatrix44 matTransform;
	pTail->GetLocalTransform( matTransform );
	matTransform.RotateZ( fStep );
	pTail->SetTransform( matTransform );
	fYaw += fStep;

	// Spin the blades
	TMatrix44 matBone;
	pTail->GetBoneTransform( iBladesBone, matBone );

	fBladesAngle = fmodf( s_fWindmillBladesSpeed * a_fDeltaTime + fBladesAngle, TMath::TWO_PI );
	matBone.RotateY( -fBladesAngle );

	if ( fScale != 1.0f )
		matBone.Scale( fScale, fScale, fScale );

	pBlades->SetTransform( matBone );
}

// $Barnyard: FUNCTION 0054c710
AWindmillHelper::WeatherVane::WeatherVane()
    : pBase( TNULL )
    , pPointer( TNULL )
    , fYaw( 0.0f )
    , fTargetYaw( 0.0f )
{
}

// $Barnyard: FUNCTION 0054ce80
void AWindmillHelper::WeatherVane::Create( const TCHAR* a_szLocator )
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	pBase = pAnimObjManager->FindType( TPS8( weathervane_base ) )->Instantiate();
	DisableHDLighting( pBase );

	TMatrix44 matTransform;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_szLocator, matTransform );
	pBase->SetTransform( matTransform );
	pBase->SetSkeletonUpdating( TTRUE );

	TINT iPointerBone = pBase->GetBoneIndex( TPS8( weathervane_pointer ) );
	pPointer          = pAnimObjManager->FindType( TPS8( weathervane_pointer ) )->Instantiate();

	TMatrix44 matBone;
	pBase->GetBoneTransform( iPointerBone, matBone );
	matBone.RotateZ( ms_fWindDirection );

	fTargetYaw = ms_fWindDirection;
	fYaw       = ms_fWindDirection;

	pPointer->SetTransform( matBone );
	pPointer->SetSkeletonUpdating( TTRUE );

	pCollision = new ACollisionObjectMesh();
	pCollision->Create( pBase->GetSceneObject() );
}

// $Barnyard: FUNCTION 0054cd20
void AWindmillHelper::WeatherVane::Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling )
{
	if ( !pPointer->IsVisible() || ( a_bDistanceCulling && !IsInAnimationRange( pPointer ) ) )
	{
		fTargetYaw = ms_fWindDirection;
		fYaw       = ms_fWindDirection;
		return;
	}

	if ( fTargetYaw == fYaw )
		fTargetYaw = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( -0.05f, 0.05f ) + ms_fWindDirection;

	TINT iSign = ( fTargetYaw <= fYaw ) ? -1 : 1;

	TFLOAT fStep = TFLOAT( iSign ) * s_fWeatherVaneTurnSpeed * a_fDeltaTime;
	TFLOAT fDiff = fTargetYaw - fYaw;

	if ( TMath::Abs( fStep ) > TMath::Abs( fDiff ) )
		fStep = fDiff;

	TMatrix44 matTransform;
	pPointer->GetLocalTransform( matTransform );
	matTransform.RotateZ( fStep );
	pPointer->SetTransform( matTransform );
	fYaw += fStep;
}

// $Barnyard: FUNCTION 0054d100
void AWindmillHelper::WaterWheel::Create( const TCHAR* a_szLocator )
{
	pModel = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( waterwheel ) )->Instantiate();

	TMatrix44 matTransform;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( a_szLocator, matTransform );
	pModel->SetTransform( matTransform );
	pModel->SetSkeletonUpdating( TTRUE );
}

// $Barnyard: FUNCTION 0054d040
void AWindmillHelper::WaterWheel::Update( TFLOAT a_fDeltaTime, TBOOL a_bDistanceCulling )
{
	if ( !pModel->IsVisible() )
		return;

	if ( a_bDistanceCulling && !IsInAnimationRange( pModel ) )
		return;

	TMatrix44 matTransform;
	pModel->GetLocalTransform( matTransform );
	matTransform.RotateX( -( s_fWaterWheelSpeed * a_fDeltaTime ) );
	pModel->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 0054d320
AWindmillHelper::AWindmillHelper()
    : m_iNumWindmills( 0 )
    , m_iNumWeatherVanes( 0 )
    , m_iNumWaterWheels( 0 )
    , m_iNumBarnSigns( 0 )
    , m_bDistanceCulling( TTRUE )
{
}

// $Barnyard: FUNCTION 0054c7a0
// $Barnyard: FUNCTION 0054d3e0
AWindmillHelper::~AWindmillHelper()
{
}

// $Barnyard: FUNCTION 0054d400
TBOOL AWindmillHelper::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( TINT i = m_iNumWindmills - 1; i >= 0; i-- )
		m_aWindmills[ i ].Update( a_fDeltaTime, m_bDistanceCulling );

	for ( TINT i = m_iNumWeatherVanes - 1; i >= 0; i-- )
		m_aWeatherVanes[ i ].Update( a_fDeltaTime, m_bDistanceCulling );

	for ( TINT i = m_iNumWaterWheels - 1; i >= 0; i-- )
		m_aWaterWheels[ i ].Update( a_fDeltaTime, m_bDistanceCulling );

	for ( TINT i = m_iNumBarnSigns - 1; i >= 0; i-- )
	{
		AModelInstance* pSign = m_aBarnSigns[ i ].GetModelInstance();

		if ( pSign && pSign->IsUpdatingSkeleton() )
			m_aBarnSigns[ i ].Update( a_fDeltaTime, m_bDistanceCulling );
	}

	// The stars of the minigame replays spin with the farm objects
	AReplayManager::GetSingleton()->RotateStars( a_fDeltaTime );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054d4e0
void AWindmillHelper::DestroyWindmills()
{
	while ( m_iNumWindmills != 0 )
	{
		Windmill& rWindmill = m_aWindmills[ m_iNumWindmills - 1 ];

		g_pModelRepos->DestroyModelInstance( rWindmill.pBlades );
		rWindmill.pBlades = TNULL;
		g_pModelRepos->DestroyModelInstance( rWindmill.pTail );
		rWindmill.pTail = TNULL;

		PopWindmill();
	}
}

// $Barnyard: FUNCTION 0054d540
void AWindmillHelper::DestroyWeatherVanes()
{
	while ( m_iNumWeatherVanes != 0 )
	{
		WeatherVane& rVane = m_aWeatherVanes[ m_iNumWeatherVanes - 1 ];

		rVane.pCollision->UnlinkCollisionObject();

		if ( rVane.pCollision )
			delete rVane.pCollision;

		g_pModelRepos->DestroyModelInstance( rVane.pBase );
		rVane.pBase = TNULL;
		g_pModelRepos->DestroyModelInstance( rVane.pPointer );
		rVane.pPointer = TNULL;

		PopWeatherVane();
	}
}

// $Barnyard: FUNCTION 0054d5b0
void AWindmillHelper::DestroyWaterWheels()
{
	while ( m_iNumWaterWheels != 0 )
	{
		WaterWheel& rWheel = m_aWaterWheels[ m_iNumWaterWheels - 1 ];

		g_pModelRepos->DestroyModelInstance( rWheel.pModel );
		rWheel.pModel = TNULL;

		PopWaterWheel();
	}
}

// NOTE: the Pop methods are T2Vector<T, N>::PopBack instances in retail (the popped slot gets a default constructed T)
// $Barnyard: FUNCTION 0054c840
void AWindmillHelper::PopWindmill()
{
	m_iNumWindmills--;
	m_aWindmills[ m_iNumWindmills ].Reset();
}

// $Barnyard: FUNCTION 0054c890
void AWindmillHelper::PopWeatherVane()
{
	// NOTE: the collision pointer gets an uninitialised value in retail
	m_iNumWeatherVanes--;
	m_aWeatherVanes[ m_iNumWeatherVanes ]            = WeatherVane();
	m_aWeatherVanes[ m_iNumWeatherVanes ].pCollision = TNULL;
}

// $Barnyard: FUNCTION 0054c8d0
void AWindmillHelper::PopWaterWheel()
{
	m_iNumWaterWheels--;
	m_aWaterWheels[ m_iNumWaterWheels ].pModel = TNULL;
}

// $Barnyard: FUNCTION 0054c8e0
void AWindmillHelper::PopBarnSign()
{
	m_iNumBarnSigns--;
	m_aBarnSigns[ m_iNumBarnSigns ] = ABarnSign();
}

// $Barnyard: FUNCTION 0054d6b0
void AWindmillHelper::SetBarnSignAnimated( TBOOL a_bAnimated )
{
	if ( m_iNumBarnSigns > 0 && m_aBarnSigns[ 0 ].GetModelInstance() )
		m_aBarnSigns[ 0 ].GetModelInstance()->SetSkeletonUpdating( a_bAnimated );
}

// $Barnyard: FUNCTION 0054d6d0
void AWindmillHelper::OnDestroy()
{
	DestroyWindmills();
	DestroyWeatherVanes();
	DestroyWaterWheels();

	while ( m_iNumBarnSigns != 0 )
	{
		m_aBarnSigns[ m_iNumBarnSigns - 1 ].Destroy();
		PopBarnSign();
	}
}

// $Barnyard: FUNCTION 0054d630
void AWindmillHelper::CreateCowTippingWindmill()
{
	TASSERT( m_iNumWindmills < MAX_NUM_WINDMILLS );
	m_aWindmills[ m_iNumWindmills++ ].Create( "wmillcowtip", TPS8( windmillcowtip_tail ), TPS8( windmillcowtip_blades ), 1.0f, 80.0f, 100.0f );
	m_bDistanceCulling = TFALSE;
}

// $Barnyard: FUNCTION 0054d670
void AWindmillHelper::AddWindmill( const TCHAR* a_szLocator, TFLOAT a_fScale, TFLOAT a_fFadeStart, TFLOAT a_fFadeEnd )
{
	TASSERT( m_iNumWindmills < MAX_NUM_WINDMILLS );
	m_aWindmills[ m_iNumWindmills++ ].Create( a_szLocator, TPS8( windmill_tail ), TPS8( windmill_blades ), a_fScale, a_fFadeStart, a_fFadeEnd );
}

// $Barnyard: FUNCTION 0054d7c0
void AWindmillHelper::CreateBarnSign()
{
	TASSERT( m_iNumBarnSigns < TARRAYSIZE( m_aBarnSigns ) );
	m_aBarnSigns[ m_iNumBarnSigns++ ].SpawnAtLocator( "loc_barnsign" );
}

// $Barnyard: FUNCTION 0054d7e0
void AWindmillHelper::CreateFarmObjects()
{
	// NOTE: AddWindmill is inlined here in retail
	AddWindmill( "wmill_01", 1.0f, 80.0f, 100.0f );
	AddWindmill( "wmill_02", 0.7f, 80.0f, 100.0f );

	TASSERT( m_iNumWeatherVanes < MAX_NUM_WEATHERVANES );
	m_aWeatherVanes[ m_iNumWeatherVanes++ ].Create( "wvane_01" );

	TASSERT( m_iNumWaterWheels < MAX_NUM_WATERWHEELS );
	m_aWaterWheels[ m_iNumWaterWheels++ ].Create( "wwheel" );

	// NOTE: inlined in retail
	CreateBarnSign();

	m_bDistanceCulling = TTRUE;
}
