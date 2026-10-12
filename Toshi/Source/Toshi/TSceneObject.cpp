#include "ToshiPCH.h"
#include "TSceneObject.h"
#include "Render/TRenderInterface.h"
#include "Render/TCameraObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006cb4f0
TSceneObject::TSceneObject()
{
	m_vFadeParams.y = 1.0f;
}

TSceneObject::~TSceneObject()
{
	DestroyModelInstance();
}

// $Barnyard: FUNCTION 006cb530
void TSceneObject::Create( TManagedModel* a_pModelPtr )
{
	m_eFlags |= 0b00001000;
	m_pModelRef        = a_pModelPtr;
	m_pModelInstance   = a_pModelPtr->GetModel()->CreateInstance();
	m_aUnk1[ 0 ]       = 0;
	m_aUnk1[ 1 ]       = 0xFF;
	m_aUnk1[ 2 ]       = 0xFF;
	m_aUnk1[ 3 ]       = 0xFF;
	m_aUnk1[ 4 ]       = 0x80;
	m_aUnk1[ 5 ]       = 0;
	m_aUnk1[ 6 ]       = 0;
	m_aUnk1[ 7 ]       = 0;
	m_vFadeDistances.x = 0.0f;
	m_vFadeDistances.y = 0.0f;
	m_vFadeParams.x    = 1.0f;
}

// $Barnyard: FUNCTION 006cbab0
void TSceneObject::Delete()
{
	if ( this )
		delete this;
}

// $Barnyard: FUNCTION 006cb580
void TSceneObject::Update( TFLOAT a_fDeltaTime )
{
	m_pModelInstance->Update( a_fDeltaTime );
}

// Distances at which the two distance fades of the render context start and end
static TFLOAT s_fContextFade1Start = 17.0f; // 0x0077d0b4
static TFLOAT s_fContextFade1End   = 23.0f; // 0x0077d0b8
static TFLOAT s_fContextFade2Start = 19.5f; // 0x0077d0bc
static TFLOAT s_fContextFade2End   = 23.0f; // 0x0077d0c0

// $Barnyard: FUNCTION 006cb590
TBOOL TSceneObject::Render( TUINT a_uiClipFlags, const TVector3& a_rBounding )
{
	auto pContext       = TRenderInterface::GetSingleton()->GetCurrentContext();
	auto pCamera        = pContext->GetCameraObject();
	auto uiOldClipFlags = pContext->GetClipFlags();

	TFLOAT fDistance = TVector3::Distance( pCamera->GetTransformObject().GetTranslation(), a_rBounding );

	if ( m_eFlags & FLAGS_OPACITY )
	{
		// Fully transparent
		if ( m_vFadeParams.y <= 0.0f )
			return TFALSE;
	}
	else if ( ( m_eFlags & FLAGS_DISTANCE_CULLING ) && fDistance >= m_vFadeDistances.x && m_vFadeDistances.x < m_vFadeDistances.y && fDistance > m_vFadeDistances.y )
	{
		// Too far away
		return TFALSE;
	}

	if ( m_vFadeDistances.x == m_vFadeDistances.y )
	{
		// Calculate default fade distances based on the camera clip planes
		TFLOAT fFar   = pCamera->GetFar();
		TFLOAT fNear  = pCamera->GetNear();
		TFLOAT fRange = fFar - fNear;

		m_vFadeDistances.x = 0.2f * fRange + fNear;
		m_vFadeDistances.y = fNear + fRange * 0.25f;
		m_vFadeParams.x    = 1.0f / ( m_vFadeDistances.y - m_vFadeDistances.x );
	}

	// Select LOD
	TModel* pModel       = m_pModelInstance->GetModel();
	TINT    iLOD         = 0;
	TFLOAT  fLODDistance = fDistance;
	TFLOAT  fFOV         = pCamera->GetFOV();

	if ( fFOV < 1.0f )
		fLODDistance = fDistance - ( 1.0f - fFOV ) * 16.0f;

	for ( TINT i = pModel->GetNumLODs() - 1; i > 0; i-- )
	{
		// LOD i is used beyond the distance at index i - 1
		if ( fLODDistance > pModel->m_aLODDistances[ i - 1 ] )
		{
			iLOD = i;
			break;
		}
	}

	if ( iLOD >= pModel->GetNumLODs() )
		iLOD = pModel->GetNumLODs() - 1;

	// Update distance fades of the context
	if ( fDistance >= s_fContextFade1End )
	{
		pContext->SetFlag( TRenderContext::FLAG_DISTANCEFADE1, TFALSE );
		pContext->GetUnk16C().w = 0.0f;
	}
	else
	{
		pContext->SetFlag( TRenderContext::FLAG_DISTANCEFADE1, TTRUE );
		pContext->GetUnk16C().w = ( fDistance <= s_fContextFade1Start ) ? 1.0f : 1.0f - ( fDistance - s_fContextFade1Start ) / ( s_fContextFade1End - s_fContextFade1Start );
	}

	if ( fDistance >= s_fContextFade2End )
	{
		pContext->SetFlag( TRenderContext::FLAG_DISTANCEFADE2, TFALSE );
		pContext->SetUnk3AC( 0.0f );
	}
	else
	{
		pContext->SetFlag( TRenderContext::FLAG_DISTANCEFADE2, TTRUE );
		pContext->SetUnk3AC( ( fDistance <= s_fContextFade2Start ) ? 1.0f : 1.0f - ( fDistance - s_fContextFade2Start ) / ( s_fContextFade2End - s_fContextFade2Start ) );
	}

	m_pModelInstance->SetLOD( iLOD );

	m_TransformObject.Push();
	pContext->SetClipFlags( a_uiClipFlags );

	TFLOAT fOldShadeCoeff = pContext->GetShadeCoeff();
	TFLOAT fOldAlphaBlend = pContext->GetAlphaBlend();
	pContext->SetShadeCoeff( m_fShadeCoeff );

	TBOOL bAlphaBlend = TFALSE;
	if ( m_eFlags & FLAGS_OPACITY )
	{
		pContext->SetAlphaBlend( m_vFadeParams.y );
		bAlphaBlend = TTRUE;
	}

	m_pModelInstance->Render();

	if ( bAlphaBlend )
		pContext->SetAlphaBlend( fOldAlphaBlend );

	pContext->SetClipFlags( uiOldClipFlags );
	pContext->SetShadeCoeff( fOldShadeCoeff );
	m_TransformObject.Pop();

	return TTRUE;
}

// $Barnyard: FUNCTION 006cb9b0
TBOOL TSceneObject::RenderIfVisible()
{
	// FLAGS_SKELETON_UPDATE also hides the object (e.g. the padlocks of the open gates)
	if ( !( m_eFlags & FLAGS_SKELETON_UPDATE ) )
		return TFALSE;

	auto  pContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	auto  pModel   = m_pModelInstance->GetModel();
	auto& lod      = pModel->GetLOD( 0 );

	TVector4 transformScale = m_TransformObject.GetScale();
	TSphere  bounding       = lod.BoundingSphere;
	bounding.AsVector4().Multiply( TVector4(
	    transformScale.x,
	    transformScale.y,
	    transformScale.z,
	    TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z )
	) );

	TMatrix44 transformMatrix;
	m_TransformObject.GetLocalMatrixImp( transformMatrix );

	TMatrix44::TransformVector( bounding.GetOrigin(), transformMatrix, bounding.GetOrigin() );

	if ( bounding.GetRadius() > 0.0f )
	{
		auto pWorldPlanes = pContext->GetWorldPlanes();

		if ( !pContext->CullSphereToFrustumSimple( bounding, pWorldPlanes, 6 ) )
		{
			return TFALSE;
		}
	}

	return Render( pContext->GetClipFlags(), bounding.GetOrigin() );
}

// $Barnyard: FUNCTION 006cb8c0
TBOOL TSceneObject::ShouldAnimateThisFrame()
{
	if ( !IsSkeletonUpdateEnabled() )
		return TFALSE;

	auto  pContext = TRenderInterface::GetSingleton()->GetCurrentContext();
	auto  pModel   = m_pModelInstance->GetModel();
	auto& lod      = pModel->GetLOD( 0 );

	TVector4 transformScale = m_TransformObject.GetScale();
	TSphere  bounding       = lod.BoundingSphere;
	bounding.AsVector4().Multiply( TVector4(
	    transformScale.x,
	    transformScale.y,
	    transformScale.z,
	    TMath::Max( TMath::Max( transformScale.x, transformScale.y ), transformScale.z )
	) );

	if ( bounding.GetRadius() > 0.0f )
	{
		TMatrix44 transformMatrix;
		m_TransformObject.GetLocalMatrixImp( transformMatrix );
		TMatrix44::TransformVector( bounding.GetOrigin(), transformMatrix, bounding.GetOrigin() );

		if ( !pContext->CullSphereToFrustumSimple( bounding, pContext->GetWorldPlanes(), 6 ) )
			return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006cb890
void TSceneObject::DestroyModelInstance()
{
	if ( m_pModelInstance )
	{
		m_pModelInstance->Delete();
		m_pModelInstance = TNULL;
		m_eFlags &= ~0b00001000;
	}
}

TOSHI_NAMESPACE_END
