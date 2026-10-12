#include "pch.h"
#include "AGUI2DirectionArrow.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"

#include <Render/TCameraObject.h>
#include <Math/TMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( car_arrow_green );

// $Barnyard: FUNCTION 00638770
AGUI2DirectionArrow::AGUI2DirectionArrow()
{
}

// $Barnyard: FUNCTION 006387c0
// $Barnyard: FUNCTION 00638890
AGUI2DirectionArrow::~AGUI2DirectionArrow()
{
	m_oModelRenderer.DestroyObjects();
	m_oModelRenderer.RemoveSelf();
}

// $Barnyard: FUNCTION 00638b60
void AGUI2DirectionArrow::Tick( TFLOAT a_fDeltaTime )
{
	m_oModelRenderer.GetObject( 0 )->GetModelInstance()->SetDirection( m_vDirection );

	TMatrix44 matTransform;
	m_oModelRenderer.GetObject( 0 )->GetModelInstance()->GetLocalTransform( matTransform );
	matTransform.Scale( 3.0f, 3.0f, 3.0f );
	m_oModelRenderer.GetObject( 0 )->GetModelInstance()->SetTransform( matTransform );
}

// $Barnyard: FUNCTION 00638850
void AGUI2DirectionArrow::SetArrowVisible( TBOOL a_bVisible )
{
	m_oModelRenderer.SetVisible( a_bVisible == TTRUE );
	SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 006389c0
void AGUI2DirectionArrow::Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_oModelRenderer.Create( a_fWidth, a_fHeight, TFALSE, TTRUE );
	SetDimensions( a_fWidth, a_fHeight );

	AAnimatableObjectType* pType   = AAnimatableObjectManager::GetSingleton()->FindType( TPS8( car_arrow_green ) );
	AAnimatableObject*     pObject = AAnimatableObjectManager::GetSingleton()->CreateAnimatableObject( pType, TNULL, 0 );
	pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	pObject->GetModelInstance()->SetOpacity( 1.0f );

	m_oModelRenderer.AddObject( pObject, TNULL, TNULL );
	AddChildTail( m_oModelRenderer );
	m_oModelRenderer.UpdateViewport();
	m_oModelRenderer.GetCamera()->SetFOV( TMath::PI / 3.0f );

	// The camera looks at the arrow from above and behind
	TMatrix44 matCamera;
	matCamera.Identity();

	TVector4 vCameraPos( 0.0f, -0.5f, -1.0f, 1.0f );
	TVector4 vCameraDir( -vCameraPos.x, -vCameraPos.y, -vCameraPos.z, 1.0f );
	vCameraDir.Normalise();

	matCamera.GetTranslation() = vCameraPos;
	matCamera.LookAtDirection( vCameraDir, TVector4( 0.0f, -1.0f, 0.0f, 1.0f ) );
	m_oModelRenderer.SetCameraMatrix( matCamera );
}

// $Barnyard: FUNCTION 006388b0
void AGUI2DirectionArrow::PointAt( const TVector4& a_rcFrom, const TVector4& a_rcTo )
{
	TVector4 vDirection( a_rcTo.x - a_rcFrom.x, 0.0f, a_rcTo.z - a_rcFrom.z, a_rcTo.w );
	vDirection.Normalise();

	TVector4 vCameraDir;
	ACameraManager::GetSingleton()->GetCurrentCamera()->GetDirection( vCameraDir );
	vCameraDir.y = 0.0f;
	vCameraDir.Normalise();

	m_vDirection = TVector4( 0.0f, 0.0f, 1.0f, 1.0f );

	TFLOAT fAngle = vDirection.GetAngleBetweenVector( vCameraDir );

	if ( vCameraDir.z * vDirection.x - vDirection.z * vCameraDir.x < 0.0f )
		fAngle = TMath::TWO_PI - fAngle;

	m_vDirection.AsVector3().RotateY( fAngle );
}
