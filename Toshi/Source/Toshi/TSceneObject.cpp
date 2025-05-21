#include "ToshiPCH.h"
#include "TSceneObject.h"
#include "Render/TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006cb4f0
TSceneObject::TSceneObject()
{
	m_SomeVector2.y = 1.0f;
}

TSceneObject::~TSceneObject()
{
	DestroyModelInstance();
}

// $Barnyard: FUNCTION 006cb530
void TSceneObject::Create( TManagedModel* a_pModelPtr )
{
	m_eFlags |= 0b00001000;
	m_pModelRef      = a_pModelPtr;
	m_pModelInstance = a_pModelPtr->GetModel()->CreateInstance();
	TTODO( "Initialise some unknown values" );
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

// $Barnyard: FUNCTION 006cb590
TBOOL TSceneObject::Render( TUINT a_uiClipFlags, const TVector3& a_rBounding )
{
	TIMPLEMENT();
	auto pContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	m_pModelInstance->SetLOD( 0 );

	auto uiOldClipFlags = pContext->SetClipFlags( a_uiClipFlags );

	m_TransformObject.Push();
	m_pModelInstance->Render();
	m_TransformObject.Pop();

	pContext->SetClipFlags( uiOldClipFlags );

	return TTRUE;
}

// $Barnyard: FUNCTION 006cb9b0
TBOOL TSceneObject::RenderIfVisible()
{
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
