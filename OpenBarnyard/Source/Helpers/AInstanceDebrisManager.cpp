#include "pch.h"
#include "AInstanceDebrisManager.h"
#include "AInstanceManager.h"
#include "Terrain/ATerrainLocator.h"
#include "Physics/ABarnyardCollision.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00539ab0
AInstanceDebrisManager::DebrisList::DebrisList()
    : iNumUsed( 0 )
{
	for ( TINT i = 0; i < MAX_DEBRIS; i++ )
		llFree.PushFront( &aDebris[ i ] );
}

// $Barnyard: FUNCTION 00539790
AInstanceDebrisManager::DebrisList::~DebrisList()
{
}

// $Barnyard: FUNCTION 00539b40
AInstanceDebrisManager::AInstanceDebrisManager()
    : m_fUnk8a4( 1.0f )
{
	AInstanceManager::GetSingleton()->CreateDynamicInstances( MAX_DEBRIS );
}

// $Barnyard: FUNCTION 00539a60
AInstanceDebrisManager::~AInstanceDebrisManager()
{
	AInstanceManager::GetSingleton()->DestroyDynamicInstances();
}

// $Barnyard: FUNCTION 00539810
TBOOL AInstanceDebrisManager::Debris::Update( TFLOAT a_fDeltaTime )
{
	AInstanceManager::DynamicInstanceEntry* pEntry = TREINTERPRETCAST( AInstanceManager::DynamicInstanceEntry*, pInstance );

	// Transform of the locator list the instance belongs to
	TMatrix44 matLocal;
	pEntry->pLocatorList->GetWorldTransform().GetLocalMatrixImp( matLocal );

	TMatrix44 matStart;
	matStart.Multiply( matLocal, pEntry->matTransform );

	TMatrix44 matMoved = pEntry->matTransform;
	matMoved.RotateZ( a_fDeltaTime * fRotSpeedZ );
	matMoved.RotateX( a_fDeltaTime * fRotSpeedX );
	matMoved.RotateY( a_fDeltaTime * fRotSpeedY );

	TVector4 vMovement;
	vMovement.Multiply( vVelocity, a_fDeltaTime );
	TMatrix44::TransformVector( matMoved.GetTranslation(), matMoved, vMovement );

	fRotSpeedY += a_fDeltaTime * 0.2f;
	vVelocity.y += a_fDeltaTime * 0.98f;

	TMatrix44 matEnd;
	matEnd.Multiply( matLocal, matMoved );

	APhysics::ASweptSphere oSphere;
	oSphere.vOrigin    = TVector4( 0.0f, 0.0f, 0.3f, 1.0f );
	oSphere.vDirection = TVector4( 0.0f, 0.0f, 1.0f, 1.0f );
	oSphere.fLength    = 10.0f;
	oSphere.fRadius    = 0.3f;

	TINT iNumSteps = APhysics::CalculateNumSteps( a_fDeltaTime, 0.1f / fRotSpeedY );

	TMatrix44 matResult;
	TVector4  vNormal;
	if ( APhysics::SweepTransform( oSphere, ABarnyardCollision::GetSingleton(), matStart, matEnd, matResult, &vNormal, iNumSteps, 6 ) )
	{
		vVelocity.y = 0.0f;
		fRotSpeedY  = 0.0f;
	}

	// Back to the space of the locator list
	matLocal.Invert( matLocal );

	TMatrix44 matInvLocal = matLocal;
	TMatrix44 matNew;
	matNew.Multiply( matInvLocal, matResult );
	pEntry->matTransform = matNew;

	return TFALSE;
}
