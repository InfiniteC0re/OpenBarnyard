#include "pch.h"
#include <Render/TCameraObject.h>
#include "ADartBoard.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "Locale/ALocaleManager.h"
#include "Physics/ACollisionCommon.h"

#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>
#include <Math/TVector2.h>
#include <Render/TModel.h>
#include <Render/TModelManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( cog_large );
TPSTRING8_DECLARE( cog_small );
TPSTRING8_DECLARE( dartboard_inner );
TPSTRING8_DECLARE( dartboard_middle );
TPSTRING8_DECLARE( dartboard_outer );
TPSTRING8_DECLARE( dart_blue_inner );
TPSTRING8_DECLARE( dart_blue_inner01 );
TPSTRING8_DECLARE( dart_blue_inner02 );
TPSTRING8_DECLARE( dart_blue_middle );
TPSTRING8_DECLARE( dart_blue_middle01 );
TPSTRING8_DECLARE( dart_blue_middle02 );
TPSTRING8_DECLARE( dart_blue_middle03 );
TPSTRING8_DECLARE( dart_blue_middle04 );
TPSTRING8_DECLARE( dart_blue_outer );
TPSTRING8_DECLARE( dart_blue_outer01 );
TPSTRING8_DECLARE( dart_blue_outer02 );
TPSTRING8_DECLARE( dart_blue_outer03 );
TPSTRING8_DECLARE( dart_blue_outer04 );
TPSTRING8_DECLARE( dart_yellow_inner );
TPSTRING8_DECLARE( dart_yellow_inner01 );
TPSTRING8_DECLARE( dart_yellow_inner02 );
TPSTRING8_DECLARE( dart_yellow_middle );
TPSTRING8_DECLARE( dart_yellow_middle01 );
TPSTRING8_DECLARE( dart_yellow_middle02 );
TPSTRING8_DECLARE( dart_yellow_middle03 );
TPSTRING8_DECLARE( dart_yellow_middle04 );
TPSTRING8_DECLARE( dart_yellow_outer );
TPSTRING8_DECLARE( dart_yellow_outer01 );
TPSTRING8_DECLARE( dart_yellow_outer02 );
TPSTRING8_DECLARE( dart_yellow_outer03 );
TPSTRING8_DECLARE( dart_yellow_outer04 );

// Gravity of the falling darts and the knocked off targets (00773e28, 00773e2c)
static constexpr TFLOAT GRAVITY_TARGET = 98.0f;
static constexpr TFLOAT GRAVITY_DART   = 98.0f;

// Half the size of the board
static constexpr TFLOAT BOARD_HALF_SIZE = 5.0f;

// Hit radius of the targets of each ring (00773e1c..00773e24)
static constexpr TFLOAT TARGET_RADIUS_OUTER  = 0.1f;
static constexpr TFLOAT TARGET_RADIUS_MIDDLE = 0.15f;
static constexpr TFLOAT TARGET_RADIUS_INNER  = 0.05f;

// The model instance of the unit of a dart
static AModelInstance* GetDartModelInstance( AUnit* a_pUnit )
{
	return TSTATICCAST( AAnimatedUnit, a_pUnit )->GetAnimatableObject()->GetModelInstance();
}

//-----------------------------------------------------------------------------
// ADart
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0051f370
ADart::ADart()
    : m_iRing( ADartThrow::RING_NONE )
    , m_bFalling( TFALSE )
    , m_vVelocity( TVector4::VEC_ZERO )
    , m_fSpin( 0.0f )
    , m_bSpinning( TFALSE )
{
}

// $Barnyard: FUNCTION 0051f3b0
void ADart::SetDirection( const TVector4& a_rcDirection )
{
	TVector4 vDirection = a_rcDirection;

	static TFLOAT s_fAngle = TMath::PI * -0.5f;
	vDirection.RotateAroundAxis( TVector4::VEC_NEGY, s_fAngle );

	m_pUnit->GetSteer()->SetDirection( vDirection );
}

// $Barnyard: FUNCTION 0051ffd0
void ADart::Drop()
{
	m_bFalling = TTRUE;
	ADartBoard::GetNormal( m_vVelocity );
	m_bSpinning = TTRUE;
}

// $Barnyard: FUNCTION 00520c40
void ADart::Update()
{
	AModelInstance* pModel = GetDartModelInstance( m_pUnit );
	ASteer*         pSteer = m_pUnit->GetSteer();

	if ( m_bFalling == TTRUE )
		return;

	if ( m_bSpinning )
	{
		TMatrix44 mRotation;
		mRotation.Identity();
		mRotation.RotateX( m_fSpin );

		pModel->SetTransform( *pSteer->GetPosition(), *pSteer->GetDirection() );

		TMatrix44 mTransform;
		pModel->GetLocalTransform( mTransform );

		TMatrix44 mResult;
		mResult.Multiply( mRotation, mTransform );
		mTransform = mResult;

		pModel->SetTransform( mTransform );
	}
	else
	{
		pModel->SetTransform( *pSteer->GetPosition(), *pSteer->GetDirection() );
	}

	TMatrix44 mTransform;
	pModel->GetLocalTransform( mTransform );
	mTransform.Scale( m_fScale, m_fScale, m_fScale );
	pModel->SetTransform( mTransform );
}

// $Barnyard: FUNCTION 00520d60
TVector4& ADart::GetPosition( TVector4& a_rOutPosition )
{
	AModelInstance* pModel = GetDartModelInstance( m_pUnit );
	ASteer*         pSteer = m_pUnit->GetSteer();

	if ( m_bSpinning )
	{
		TMatrix44 mRotation;
		mRotation.Identity();
		mRotation.RotateX( m_fSpin );

		pModel->SetTransform( *pSteer->GetPosition(), *pSteer->GetDirection() );

		TMatrix44 mTransform;
		pModel->GetLocalTransform( mTransform );

		TMatrix44 mResult;
		mResult.Multiply( mRotation, mTransform );

		a_rOutPosition.x = mResult.m_f41;
		a_rOutPosition.y = mResult.m_f42;
		a_rOutPosition.z = mResult.m_f43;
		a_rOutPosition.w = 1.0f;
	}
	else
	{
		a_rOutPosition = *pSteer->GetPosition();
	}

	return a_rOutPosition;
}

// $Barnyard: FUNCTION 00520e80
void ADart::Reset()
{
	m_eState    = STATE_IDLE;
	m_bOnTarget = TFALSE;
	m_bFalling  = TFALSE;
	m_iScore    = 0;
	m_iRing     = ADartThrow::RING_NONE;
	m_vVelocity = TVector4::VEC_ZERO;
	m_fScale    = 1.0f;

	TMatrix44 mTransform;
	mTransform.Identity();
	GetDartModelInstance( m_pUnit )->SetTransform( mTransform );

	m_fSpin     = 0.0f;
	m_bSpinning = TFALSE;
}

// $Barnyard: FUNCTION 00521c40
void ADart::UpdateFalling( TFLOAT a_fDeltaTime, ADart* a_pDarts, TINT a_iNumDarts )
{
	for ( TINT i = 0; i < a_iNumDarts; i++ )
	{
		ADart& rDart = a_pDarts[ i ];

		if ( rDart.m_bFalling )
		{
			rDart.m_vVelocity.y += GRAVITY_DART * a_fDeltaTime;

			TMatrix44 mTranslation;
			mTranslation.Identity();
			mTranslation.GetTranslation().Multiply( rDart.m_vVelocity, a_fDeltaTime );

			TMatrix44 mTransform;
			GetDartModelInstance( rDart.m_pUnit )->GetLocalTransform( mTransform );

			TMatrix44 mCopy = mTransform;
			mTransform.Multiply( mTranslation, mCopy );

			GetDartModelInstance( rDart.m_pUnit )->SetTransform( mTransform );
		}
	}
}

//-----------------------------------------------------------------------------
// ADartThrow
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0051f2f0
void ADartThrow::Reset()
{
	m_fAimX      = 0.0f;
	m_fAimY      = 0.0f;
	m_fTimeUsed  = 0.0f;
	m_vTarget    = TVector4::VEC_ZERO;
	m_vDirection = TVector4::VEC_ZERO;
	m_iScore     = 0;
	m_iTeam      = TEAM_NONE;
	m_iTarget    = TARGET_NONE;
	m_iRing      = RING_NONE;
	m_iCategory  = 4;
	m_iCog       = COG_NONE;
}

// $Barnyard: FUNCTION 00520610
void ADartThrow::ComputeDirection()
{
	TVector4 vNormal;
	ADartBoard::GetNormal( vNormal );

	m_vTarget = TVector4::VEC_ZERO;

	// The aimed point of the board
	TVector4 vPoint = TVector4::VEC_ZERO;
	vPoint.y -= ( 1.0f * m_fAimY ) * BOARD_HALF_SIZE;

	TVector4 vSide;
	vSide.CrossProduct( vNormal, TVector4::VEC_NEGY );

	TVector4 vOffset;
	vOffset.Multiply( vSide, m_fAimX );
	vOffset.Multiply( BOARD_HALF_SIZE );

	vPoint.x += vOffset.x;
	vPoint.y += vOffset.y;
	vPoint.z += vOffset.z;

	const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	m_vDirection.x = vPoint.x - rcCameraPos.x;
	m_vDirection.y = vPoint.y - rcCameraPos.y;
	m_vDirection.z = vPoint.z - rcCameraPos.z;
	m_vDirection.w = vPoint.w;
	m_vDirection.Normalise();
}

//-----------------------------------------------------------------------------
// ADartBoard
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00520f50
ADartBoard::Target::Target()
{
}

// $Barnyard: FUNCTION 00520790
void ADartBoard::Target::ReadCollisionInfo( Target* a_pTarget )
{
	TModelCollisionData* pCollData = a_pTarget->oCollision.GetCollisionData( 0 );
	const TCHAR*         szName    = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( 0 ) ).strName;

	if ( T2String8::Compare( "coll_", szName, 5 ) == 0 )
	{
		if ( szName[ 5 ] == 'y' )
			a_pTarget->iTeam = ADartThrow::TEAM_YELLOW;

		if ( szName[ 5 ] == 'b' )
			a_pTarget->iTeam = ADartThrow::TEAM_BLUE;

		TCHAR szScore[ 4 ];
		TStringManager::String8Format( szScore, 3, "%c%c", szName[ 7 ], szName[ 8 ] );
		a_pTarget->iScore = T2String8::StringToInt( szScore );
	}
}

// $Barnyard: FUNCTION 00521e20
ADartBoard::ADartBoard()
    : m_iUnk1( 0 )
    , m_iUnk2( 0 )
    , m_iScoreMultiplier( 1 )
{
}

// $Barnyard: FUNCTION 00521020
ADartBoard::~ADartBoard()
{
}

// $Barnyard: FUNCTION 00521ee0
void ADartBoard::Create()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	m_aRings[ RING_INNER ].pModel  = AModelRepos::GetSingleton()->InstantiateModel( pManager->FindType( TPS8( dartboard_inner ) )->GetModel() );
	m_aRings[ RING_MIDDLE ].pModel = AModelRepos::GetSingleton()->InstantiateModel( pManager->FindType( TPS8( dartboard_middle ) )->GetModel() );
	m_aRings[ RING_OUTER ].pModel  = AModelRepos::GetSingleton()->InstantiateModel( pManager->FindType( TPS8( dartboard_outer ) )->GetModel() );

	for ( TINT i = 0; i < NUM_RINGS; i++ )
	{
		Ring& rRing = m_aRings[ i ];

		rRing.fScale = 1.0f;
		rRing.pModel->SetTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

		TMatrix44 mTransform;
		rRing.pModel->GetLocalTransform( mTransform );
		mTransform.Scale( rRing.fScale, rRing.fScale, rRing.fScale );
		rRing.pModel->SetTransform( mTransform );
		rRing.pModel->SetSkeletonUpdating( TTRUE );

		TSceneObject* pSceneObject = rRing.pModel->GetSceneObject();
		TModel*       pModel       = pSceneObject->GetModelRef()->GetModel();

		for ( TINT j = 0; j < pModel->GetNumCollisionMeshes(); j++ )
		{
			rRing.oCollisionSet.Create( pModel );
			rRing.oCollision.Create( &rRing.oCollisionSet, pSceneObject );
			rRing.oCollision.SetTransformObject( &pSceneObject->GetTransform() );
		}
	}

	CreateTargets();
	CreateCogs();
}

// $Barnyard: FUNCTION 005210b0
void ADartBoard::CreateTargets()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	m_aRings[ RING_OUTER ].fAngle       = 0.0f;
	m_aRings[ RING_OUTER ].iNumTargets  = 0;
	m_aRings[ RING_MIDDLE ].fAngle      = 0.0f;
	m_aRings[ RING_MIDDLE ].iNumTargets = 0;
	m_aRings[ RING_INNER ].fAngle       = 0.0f;
	m_aRings[ RING_INNER ].iNumTargets  = 0;

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
		m_aTargets[ i ].fScale = 1.0f;

	// Inner ring: the bullseyes
	for ( TINT i = 0; i < 4; i++ )
		m_aRings[ RING_INNER ].apTargets[ m_aRings[ RING_INNER ].iNumTargets++ ] = &m_aTargets[ i ];

	// Middle ring
	for ( TINT i = 4; i < 12; i++ )
		m_aRings[ RING_MIDDLE ].apTargets[ m_aRings[ RING_MIDDLE ].iNumTargets++ ] = &m_aTargets[ i ];

	// Outer ring: blue then yellow
	for ( TINT i = 16; i < 20; i++ )
		m_aRings[ RING_OUTER ].apTargets[ m_aRings[ RING_OUTER ].iNumTargets++ ] = &m_aTargets[ i ];

	for ( TINT i = 12; i < 16; i++ )
		m_aRings[ RING_OUTER ].apTargets[ m_aRings[ RING_OUTER ].iNumTargets++ ] = &m_aTargets[ i ];

	const TPString8* apLocators[ NUM_TARGETS ] = {
		&TPS8( dart_blue_inner01 ),
		&TPS8( dart_blue_inner02 ),
		&TPS8( dart_yellow_inner01 ),
		&TPS8( dart_yellow_inner02 ),
		&TPS8( dart_blue_middle01 ),
		&TPS8( dart_blue_middle02 ),
		&TPS8( dart_blue_middle03 ),
		&TPS8( dart_blue_middle04 ),
		&TPS8( dart_yellow_middle01 ),
		&TPS8( dart_yellow_middle02 ),
		&TPS8( dart_yellow_middle03 ),
		&TPS8( dart_yellow_middle04 ),
		&TPS8( dart_yellow_outer01 ),
		&TPS8( dart_yellow_outer02 ),
		&TPS8( dart_yellow_outer03 ),
		&TPS8( dart_yellow_outer04 ),
		&TPS8( dart_blue_outer01 ),
		&TPS8( dart_blue_outer02 ),
		&TPS8( dart_blue_outer03 ),
		&TPS8( dart_blue_outer04 ),
	};

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
		m_aTargets[ i ].strLocator = *apLocators[ i ];

	struct TargetGroup
	{
		const TPString8* pType;
		const TCHAR*     szBone;
		TINT             iFirst;
		TINT             iCount;
		TINT             iRing;
		TINT             iCategory;
		TFLOAT           fRadius;
	};

	const TargetGroup aGroups[] = {
		{ &TPS8( dart_blue_inner ), "blue_inner_center", 0, 2, RING_INNER, 2, TARGET_RADIUS_INNER },
		{ &TPS8( dart_yellow_inner ), "yellow_inner_center", 2, 2, RING_INNER, 2, TARGET_RADIUS_INNER },
		{ &TPS8( dart_blue_middle ), "blue_middle_center", 4, 4, RING_MIDDLE, 0, TARGET_RADIUS_MIDDLE },
		{ &TPS8( dart_yellow_middle ), "yellow_middle_center", 8, 4, RING_MIDDLE, 0, TARGET_RADIUS_MIDDLE },
		{ &TPS8( dart_yellow_outer ), "yellow_outer_center", 12, 4, RING_OUTER, 1, TARGET_RADIUS_OUTER },
		{ &TPS8( dart_blue_outer ), "blue_outer_center", 16, 4, RING_OUTER, 1, TARGET_RADIUS_OUTER },
	};

	for ( TINT i = 0; i < TARRAYSIZE( aGroups ); i++ )
	{
		const TargetGroup&     rcGroup = aGroups[ i ];
		AAnimatableObjectType* pType   = pManager->FindType( *rcGroup.pType );

		for ( TINT j = rcGroup.iFirst; j < rcGroup.iFirst + rcGroup.iCount; j++ )
		{
			Target& rTarget = m_aTargets[ j ];

			rTarget.pModel = AModelRepos::GetSingleton()->InstantiateModel( pType->GetModel() );

			// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
			rTarget.iBone     = rTarget.pModel->GetSkeleton()->GetBoneID( rcGroup.szBone, 0 );
			rTarget.iRing     = rcGroup.iRing;
			rTarget.iCategory = rcGroup.iCategory;
			rTarget.fRadius   = rcGroup.fRadius;
		}
	}

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		TSceneObject* pSceneObject = rTarget.pModel->GetSceneObject();
		rTarget.oCollisionSet.Create( pSceneObject->GetModelRef()->GetModel() );
		rTarget.oCollision.Create( &rTarget.oCollisionSet, pSceneObject );

		TMatrix44 mTransform;
		ATerrainInterface::GetSingleton()->GetLocatorTransform( rTarget.strLocator, mTransform );
		mTransform.Scale( rTarget.fScale, rTarget.fScale, rTarget.fScale );

		rTarget.pModel->SetTransform( mTransform );
		rTarget.pModel->SetSkeletonUpdating( TTRUE );

		rTarget.pDart = TNULL;
		rTarget.oCollision.SetTransformObject( &pSceneObject->GetTransform() );
		rTarget.vVelocity   = TVector4::VEC_ZERO;
		rTarget.vPosition   = TVector4::VEC_ZERO;
		rTarget.fTime       = 0.0f;
		rTarget.bKnockedOff = TFALSE;

		Target::ReadCollisionInfo( &rTarget );
	}
}

// $Barnyard: FUNCTION 00520420
void ADartBoard::CreateCogs()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	AAnimatableObjectType* pLargeType = pManager->FindType( TPS8( cog_large ) );
	m_aCogs[ 0 ].pModel               = AModelRepos::GetSingleton()->InstantiateModel( pLargeType->GetModel() );
	m_aCogs[ 1 ].pModel               = AModelRepos::GetSingleton()->InstantiateModel( pLargeType->GetModel() );
	m_aCogs[ 0 ].fSpeed               = TMath::PI / 6.0f;
	m_aCogs[ 1 ].fSpeed               = -TMath::PI / 6.0f;

	AAnimatableObjectType* pSmallType = pManager->FindType( TPS8( cog_small ) );
	m_aCogs[ 2 ].pModel               = AModelRepos::GetSingleton()->InstantiateModel( pSmallType->GetModel() );
	m_aCogs[ 3 ].pModel               = AModelRepos::GetSingleton()->InstantiateModel( pSmallType->GetModel() );
	m_aCogs[ 2 ].fSpeed               = -TMath::PI / 3.0f;
	m_aCogs[ 3 ].fSpeed               = TMath::PI / 3.0f;

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	pTerrain->GetLocatorTransform( "cog_large01", m_aCogs[ 0 ].mTransform );
	pTerrain->GetLocatorTransform( "cog_large02", m_aCogs[ 1 ].mTransform );
	pTerrain->GetLocatorTransform( "cog_small01", m_aCogs[ 2 ].mTransform );
	pTerrain->GetLocatorTransform( "cog_small02", m_aCogs[ 3 ].mTransform );

	for ( TINT i = 0; i < NUM_COGS; i++ )
	{
		Cog& rCog = m_aCogs[ i ];

		rCog.pModel->SetTransform( rCog.mTransform );
		rCog.pModel->SetSkeletonUpdating( TTRUE );

		TSceneObject* pSceneObject = rCog.pModel->GetSceneObject();
		rCog.oCollisionSet.Create( pSceneObject->GetModelRef()->GetModel() );
		rCog.oCollision.Create( &rCog.oCollisionSet, pSceneObject );
		rCog.oCollision.SetTransformObject( &pSceneObject->GetTransform() );
	}
}

// $Barnyard: FUNCTION 00520580
void ADartBoard::Destroy()
{
	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_aTargets[ i ].pModel );
		m_aTargets[ i ].pModel = TNULL;
	}

	for ( TINT i = 0; i < NUM_RINGS; i++ )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_aRings[ i ].pModel );
		m_aRings[ i ].pModel = TNULL;
	}

	for ( TINT i = 0; i < NUM_COGS; i++ )
		AModelRepos::GetSingleton()->DestroyModelInstance( m_aCogs[ i ].pModel );
}

// $Barnyard: FUNCTION 0051f6d0
void ADartBoard::SetSkeletonUpdating( TBOOL a_bUpdating )
{
	for ( TINT i = 0; i < NUM_COGS; i++ )
		m_aCogs[ i ].pModel->SetSkeletonUpdating( a_bUpdating );

	for ( TINT i = 0; i < NUM_RINGS; i++ )
		m_aRings[ i ].pModel->SetSkeletonUpdating( a_bUpdating );

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
		m_aTargets[ i ].pModel->SetSkeletonUpdating( a_bUpdating );
}

// $Barnyard: FUNCTION 0051f9f0
void ADartBoard::ResetTargets()
{
	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		m_aTargets[ i ].bKnockedOff = TFALSE;
		m_aTargets[ i ].bActive     = TTRUE;
		m_aTargets[ i ].fTime       = 0.0f;
	}
}

// $Barnyard: FUNCTION 0051fa20
void ADartBoard::UpdateCogs( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_COGS; i++ )
	{
		Cog& rCog = m_aCogs[ i ];

		TMatrix44 mTransform;
		rCog.pModel->GetLocalTransform( mTransform );

		static TFLOAT s_fUnused = 0.0f;

		TMatrix44 mRotation;
		mRotation.Identity();
		mRotation.RotateX( s_fUnused * a_fDeltaTime );

		TMatrix44 mCopy = mTransform;
		mTransform.Multiply( mRotation, mCopy );

		TMatrix44 mSpin;
		mSpin.Identity();
		mSpin.RotateX( a_fDeltaTime * rCog.fSpeed );

		mCopy = mTransform;
		mTransform.Multiply( mCopy, mSpin );

		rCog.pModel->SetTransform( mTransform );
	}
}

// $Barnyard: FUNCTION 00521a80
void ADartBoard::UpdateTargets( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( !rTarget.bKnockedOff )
			continue;

		rTarget.vPosition.x += a_fDeltaTime * rTarget.vVelocity.x;
		TFLOAT fVelocityY = a_fDeltaTime * rTarget.vVelocity.y;
		rTarget.vPosition.y += a_fDeltaTime * a_fDeltaTime * 4.9f + fVelocityY + fVelocityY;
		rTarget.vVelocity.y += GRAVITY_TARGET * a_fDeltaTime;
		rTarget.fTime += a_fDeltaTime;

		TMatrix44 mTranslation;
		mTranslation.Identity();
		mTranslation.GetTranslation().Multiply( rTarget.vVelocity, a_fDeltaTime );

		TMatrix44 mTransform;
		rTarget.pModel->GetLocalTransform( mTransform );

		TMatrix44 mCopy = mTransform;
		mTransform.Multiply( mTranslation, mCopy );

		rTarget.pModel->SetTransform( mTransform );
		rTarget.oCollision.SetTransformObject( &rTarget.pModel->GetTransform() );

		if ( rTarget.pDart )
		{
			// The dart stuck in the target falls with it
			AModelInstance* pDartModel = GetDartModelInstance( rTarget.pDart->m_pUnit );

			TMatrix44 mDartTransform;
			pDartModel->GetLocalTransform( mDartTransform );

			TMatrix44 mDartCopy = mDartTransform;
			mDartTransform.Multiply( mTranslation, mDartCopy );

			pDartModel->SetTransform( mDartTransform );
		}
	}
}

// $Barnyard: FUNCTION 00520010
void ADartBoard::RotateRing( TINT a_iRing, TFLOAT a_fAngle )
{
	Ring& rRing = m_aRings[ a_iRing ];

	rRing.fAngle += a_fAngle;
	rRing.fAngle = fmod( rRing.fAngle, TMath::TWO_PI );

	TMatrix44 mTransform;
	mTransform.Identity();
	mTransform.Scale( rRing.fScale, rRing.fScale, rRing.fScale );
	mTransform.RotateX( rRing.fAngle );
	rRing.pModel->SetTransform( mTransform );
	rRing.oCollision.SetTransformObject( &rRing.pModel->GetTransform() );

	for ( TINT i = 0; i < rRing.iNumTargets; i++ )
	{
		Target* pTarget = rRing.apTargets[ i ];

		if ( !pTarget->bKnockedOff )
		{
			TMatrix44 mTargetTransform;
			ATerrainInterface::GetSingleton()->GetLocatorTransform( pTarget->strLocator, mTargetTransform );
			mTargetTransform.Scale( pTarget->fScale, pTarget->fScale, pTarget->fScale );
			mTargetTransform.RotateX( rRing.fAngle );

			pTarget->pModel->SetTransform( mTargetTransform );
			pTarget->oCollision.SetTransformObject( &pTarget->pModel->GetTransform() );
		}
	}
}

// $Barnyard: FUNCTION 0051fb30
void ADartBoard::CastThrow( ADartThrow& a_rThrow, TBOOL a_bUpdateTarget, TBOOL a_bScore )
{
	if ( a_bScore == TTRUE )
		a_rThrow.m_iTarget = ADartThrow::TARGET_NONE;

	const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();
	TVector4        vDirection  = a_rThrow.m_vDirection;

	TFLOAT                    fDistance = 10000.0f;
	TBOOL                     bHit      = TFALSE;
	ACollisionModelRayHitList oHitList;

	ARayData oRay( rcCameraPos, vDirection, 10000.0f );
	TINT     iClosest = -1;

	for ( TINT i = 0; i < NUM_TARGETS && !bHit; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( rTarget.oCollision.CollideRay( rcCameraPos, vDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TTRUE ) != TTRUE )
			continue;

		TFLOAT fClosest = fDistance;

		for ( TINT k = 0; k < oHitList.iNumHits; k++ )
		{
			if ( fClosest >= oHitList.aHits[ k ].fDistance )
			{
				iClosest = k;
				fClosest = oHitList.aHits[ k ].fDistance;
			}
		}

		ACollisionModelRayHitList::Hit& rHit = oHitList.aHits[ iClosest ];

#ifdef BARNYARD_COMMUNITY_PATCH
		// The original passes the index of the hit as the index of the collision model
		TModelCollisionData* pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
#else
		TModelCollisionData* pCollData = rHit.pInstance->GetCollisionData( iClosest );
#endif
		const TCHAR* szName = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).strName;

		if ( T2String8::Compare( "coll_", szName, 5 ) != 0 )
			continue;

		TBOOL bYellow = TFALSE;

		if ( szName[ 5 ] == 'y' )
		{
			bYellow = TTRUE;

			if ( a_bScore == TTRUE )
				a_rThrow.m_iTeam = ADartThrow::TEAM_YELLOW;
		}

		if ( szName[ 5 ] == 'b' )
		{
			if ( a_bScore == TTRUE )
				a_rThrow.m_iTeam = ADartThrow::TEAM_BLUE;
		}
		else if ( bYellow != TTRUE )
		{
			continue;
		}

		TCHAR szScore[ 4 ];
		TStringManager::String8Format( szScore, 3, "%c%c", szName[ 7 ], szName[ 8 ] );
		TINT iScore = T2String8::StringToInt( szScore );

		if ( a_bUpdateTarget == TTRUE )
		{
			TVector4 vOffset;
			vOffset.Multiply( vDirection, rHit.fDistance - 0.05f );
			a_rThrow.m_vTarget.Add( rcCameraPos, vOffset );
		}

		if ( a_bScore == TTRUE )
			a_rThrow.m_iScore = iScore * m_iScoreMultiplier;

		bHit = TTRUE;

		if ( a_bScore == TTRUE )
		{
			a_rThrow.m_iTarget   = i;
			a_rThrow.m_iCategory = rTarget.iCategory;
		}
	}

	if ( !bHit )
	{
		a_rThrow.m_iScore = 0;
		a_rThrow.m_iTeam  = ADartThrow::TEAM_NONE;

		for ( TINT i = 0; i < NUM_RINGS; i++ )
		{
			if ( m_aRings[ i ].oCollision.CollideRay( rcCameraPos, vDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TTRUE ) )
			{
				a_rThrow.m_iRing = i;

				TVector4 vOffset;
				vOffset.Multiply( vDirection, fDistance - 0.05f );

				bHit                 = TTRUE;
				a_rThrow.m_vTarget.x = vOffset.x + rcCameraPos.x;
				a_rThrow.m_vTarget.y = vOffset.y + rcCameraPos.y;
				a_rThrow.m_vTarget.z = vOffset.z + rcCameraPos.z;
				a_rThrow.m_vTarget.w = rcCameraPos.w;
			}
		}

		if ( !bHit )
		{
			for ( TINT i = 0; i < NUM_COGS; i++ )
			{
				if ( m_aCogs[ i ].oCollision.CollideRay( rcCameraPos, vDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG( 0 ), TTRUE ) )
					a_rThrow.m_iCog = i;
			}
		}
	}
}

// $Barnyard: FUNCTION 00520820
void ADartBoard::ComputeTarget( ADartThrow& a_rThrow )
{
	CastThrow( a_rThrow, TTRUE, TFALSE );

	if ( a_rThrow.m_iRing == ADartThrow::RING_NONE && a_rThrow.m_iTarget == ADartThrow::TARGET_NONE )
	{
		// Nothing has been hit, aim at the plane of the board
		TVector4 vNormal;
		GetNormal( vNormal );

		TFLOAT fOffsetY    = a_rThrow.m_fAimY * BOARD_HALF_SIZE;
		a_rThrow.m_vTarget = TVector4::VEC_ZERO;
		a_rThrow.m_vTarget.y -= fOffsetY;

		TVector4 vSide;
		vSide.CrossProduct( vNormal, TVector4::VEC_NEGY );

		TVector4 vOffset;
		vOffset.Multiply( vSide, a_rThrow.m_fAimX );
		vOffset.Multiply( BOARD_HALF_SIZE );

		a_rThrow.m_vTarget.x += vOffset.x;
		a_rThrow.m_vTarget.y += vOffset.y;
		a_rThrow.m_vTarget.z += vOffset.z;
	}
}

// $Barnyard: FUNCTION 00520940
TBOOL ADartBoard::WillHitTarget( TINT a_iTarget, TFLOAT a_fAimY, TFLOAT a_fAimX, TFLOAT a_fTime )
{
	ADartThrow oThrow;
	oThrow.m_fAimX = a_fAimX;
	oThrow.m_fAimY = a_fAimY;
	oThrow.ComputeDirection();
	ComputeTarget( oThrow );

	Target& rTarget = m_aTargets[ a_iTarget ];

	// Move the target to where it will be in a_fTime seconds
	TMatrix44 mOriginal;
	rTarget.pModel->GetLocalTransform( mOriginal );

	TMatrix44 mTransform;
	rTarget.pModel->GetLocalTransform( mTransform );

	TMatrix44 mRotation;
	mRotation.Identity();
	mRotation.RotateX( a_fTime * m_aRings[ rTarget.iRing ].fSpeed );

	TMatrix44 mResult;
	mResult.Multiply( mRotation, mTransform );
	mTransform = mResult;

	rTarget.pModel->SetTransform( mTransform );
	rTarget.oCollision.SetTransformObject( &rTarget.pModel->GetTransform() );

	TFLOAT                    fDistance = 1000.0f;
	ACollisionModelRayHitList oHitList;

	TBOOL bHit = rTarget.oCollision.CollideRay(
	    ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation(),
	    oThrow.m_vDirection,
	    fDistance,
	    &oHitList,
	    TCollisionCommon::TOSHICGROUPFLAG( 0 ),
	    TTRUE
	);

	// Put it back
	rTarget.pModel->SetTransform( mOriginal );
	rTarget.oCollision.SetTransformObject( &rTarget.pModel->GetTransform() );

	return bHit;
}

// $Barnyard: FUNCTION 0051ff50
void ADartBoard::KnockOffTarget( TINT a_iTarget, ADart* a_pDart )
{
	Target& rTarget = m_aTargets[ a_iTarget ];

	if ( rTarget.bKnockedOff != TTRUE )
	{
		rTarget.bKnockedOff = TTRUE;

		TVector4 vNormal;
		GetNormal( vNormal );
		rTarget.vVelocity.Multiply( vNormal, 10.0f );

		if ( a_pDart )
		{
			rTarget.pDart        = a_pDart;
			a_pDart->m_bOnTarget = TTRUE;
		}
	}
}

// $Barnyard: FUNCTION 0051f7e0
TINT ADartBoard::FindBestTarget( TINT a_iTeam )
{
	TINT iBestTarget = ADartThrow::TARGET_NONE;
	TINT iBestScore  = -1;

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( rTarget.iTeam == a_iTeam && rTarget.bKnockedOff == TFALSE && rTarget.iScore > iBestScore )
		{
			iBestScore  = rTarget.iScore;
			iBestTarget = i;
		}
	}

	return iBestTarget;
}

// $Barnyard: FUNCTION 00520ad0
TINT ADartBoard::FindClosestTarget( TINT a_iTeam, TFLOAT a_fAimX, TFLOAT a_fAimY )
{
	TINT   iBestScore = -1;
	TFLOAT fClosest   = 100.0f;
	TINT   iClosest   = ADartThrow::TARGET_NONE;

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( rTarget.iTeam == a_iTeam && rTarget.bKnockedOff == TFALSE && rTarget.iScore >= iBestScore )
			iBestScore = rTarget.iScore;
	}

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		Target& rTarget = m_aTargets[ i ];

		if ( rTarget.iTeam == a_iTeam && rTarget.bKnockedOff == TFALSE && rTarget.iScore >= iBestScore )
		{
			TVector4 vPosition;
			GetTargetPosition( vPosition, i, 0.0f );

			TFLOAT fTargetX;
			TFLOAT fTargetY;
			GetAimFromPosition( vPosition, fTargetX, fTargetY );

			TFLOAT fDistance = TVector2::Distance( TVector2( fTargetX, fTargetY ), TVector2( a_fAimX, a_fAimY ) );

			if ( fDistance < fClosest )
			{
				fClosest = fDistance;
				iClosest = i;
			}
		}
	}

	return iClosest;
}

// $Barnyard: FUNCTION 0051f8b0
TVector4& ADartBoard::GetTargetPosition( TVector4& a_rOutPosition, TINT a_iTarget, TFLOAT a_fTime )
{
	Target& rTarget = m_aTargets[ a_iTarget ];

	TMatrix44 mRotation;
	mRotation.Identity();
	mRotation.RotateX( a_fTime * m_aRings[ rTarget.iRing ].fSpeed );

	TSkeletonInstance* pSkeleton = rTarget.pModel->GetSkeletonInstance();

	TMatrix44 mTransform;
	mTransform.Identity();

	TMatrix44 mBone;
	pSkeleton->GetBoneTransformCurrent( rTarget.iBone, mBone );
	rTarget.pModel->GetTransform().GetLocalMatrixImp( mTransform );

	TMatrix44 mCopy = mTransform;
	mTransform.Multiply( mRotation, mCopy );

	mCopy = mTransform;
	mTransform.Multiply( mCopy, mBone );

	a_rOutPosition = mTransform.GetTranslation();
	return a_rOutPosition;
}

// $Barnyard: FUNCTION 0051f5b0
const TWCHAR* ADartBoard::GetHitText( const ADartThrow& a_rcThrow, TINT a_iTeam )
{
	if ( a_rcThrow.m_iTarget != ADartThrow::TARGET_NONE && m_aTargets[ a_rcThrow.m_iTarget ].iTeam != a_iTeam )
		return ALocaleManager::Interface()->GetString( 780 );

	switch ( a_rcThrow.m_iScore )
	{
		case 2:
			return ALocaleManager::Interface()->GetString( 779 );
		case 5:
		case 15:
			return ALocaleManager::Interface()->GetString( 778 );
		case 10:
		case 50:
			return ALocaleManager::Interface()->GetString( 775 );
		case 20:
			return ALocaleManager::Interface()->GetString( 777 );
		case 25:
			return ALocaleManager::Interface()->GetString( 776 );
		default:
			return ALocaleManager::Interface()->GetString( 781 );
	}
}

// $Barnyard: FUNCTION 0051f740
TVector4& ADartBoard::GetNormal( TVector4& a_rOutNormal )
{
	static TVector4 s_vNormal = TVector4::VEC_NEGX;

	a_rOutNormal = s_vNormal;
	return a_rOutNormal;
}

// $Barnyard: FUNCTION 00520160
TBOOL ADartBoard::GetCorners( TVector4& a_rOutTop, TVector4& a_rOutRight, TVector4& a_rOutLeft, TVector4& a_rOutBottom )
{
	TVector4 vCentre = TVector4::VEC_ZERO;

	TVector4 vNormal;
	GetNormal( vNormal );

	TVector4 vSide;
	vSide.CrossProduct( vNormal, TVector4::VEC_NEGY );

	TVector4 vOffset;

	vOffset.Multiply( TVector4::VEC_NEGY, BOARD_HALF_SIZE );
	a_rOutTop.Set( vCentre.x + vOffset.x, vCentre.y + vOffset.y, vCentre.z + vOffset.z, vCentre.w );

	vOffset.Multiply( TVector4::VEC_NEGY, -BOARD_HALF_SIZE );
	a_rOutBottom.Set( vCentre.x + vOffset.x, vCentre.y + vOffset.y, vCentre.z + vOffset.z, vCentre.w );

	vOffset.Multiply( vSide, BOARD_HALF_SIZE );
	a_rOutRight.Set( vCentre.x + vOffset.x, vCentre.y + vOffset.y, vCentre.z + vOffset.z, vCentre.w );

	vOffset.Multiply( vSide, -BOARD_HALF_SIZE );
	a_rOutLeft.Set( vCentre.x + vOffset.x, vCentre.y + vOffset.y, vCentre.z + vOffset.z, vCentre.w );

	return TTRUE;
}

// $Barnyard: FUNCTION 005203b0
void ADartBoard::GetAimFromPosition( const TVector4& a_rcPosition, TFLOAT& a_rOutAimX, TFLOAT& a_rOutAimY )
{
	TVector4 vTop;
	TVector4 vRight;
	TVector4 vLeft;
	TVector4 vBottom;
	GetCorners( vTop, vRight, vLeft, vBottom );

	a_rOutAimY = a_rcPosition.y / ( TMath::Abs( vBottom.y ) + TMath::Abs( vTop.y ) ) * -2.0f;
	a_rOutAimX = ( a_rcPosition.z / ( TMath::Abs( vLeft.z ) + TMath::Abs( vRight.z ) ) ) * 2.0f;
}
