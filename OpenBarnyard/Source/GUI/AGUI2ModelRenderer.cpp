#include "pch.h"
#include "AGUI2ModelRenderer.h"
#include "AGUISystem.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Memory/AMemory.h"

#include <GUI/T2GUI.h>
#include <Render/TViewport.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TSceneObject.h>

#include <Render/TBackend.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Ambient colour of the viewport (0x007798cc)
static const TVector4 s_vViewportAmbient = { 1.0f, 0.4f, 0.4f, 0.0f };

// NOTE: inlined in retail, the default state of an object slot
static void ResetObject( AGUI2ModelRenderer::Object& a_rObject )
{
	a_rObject.pObject             = TNULL;
	a_rObject.vLightColour        = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	a_rObject.vAmbientColour      = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	a_rObject.bVisible            = TTRUE;
	a_rObject.fnRenderCallback    = TNULL;
	a_rObject.pRenderCallbackData = TNULL;
}

// $Barnyard: FUNCTION 00644700
AGUI2ModelRenderer::ObjectList::ObjectList()
{
	iNumObjects = 0;

	for ( TINT i = 0; i < MAX_OBJECTS; i++ )
		ResetObject( aObjects[ i ] );
}

// $Barnyard: FUNCTION 00644750
void AGUI2ModelRenderer::ObjectList::RemoveLast()
{
	ResetObject( aObjects[ --iNumObjects ] );
}

// $Barnyard: FUNCTION 00644990
void AGUI2ModelRenderer::ObjectList::Remove( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < iNumObjects - 1; i++ )
		aObjects[ i ] = aObjects[ i + 1 ];

	ResetObject( aObjects[ --iNumObjects ] );
}

// $Barnyard: FUNCTION 006447d0
AGUI2ModelRenderer::AGUI2ModelRenderer()
    : m_pViewport( TNULL )
    , m_pCamera( TNULL )
    , m_bCreated( TFALSE )
    , m_vLightDirection( 1.0f, -1.0f, -1.0f, 0.0f )
    , m_bRender( TTRUE )
    , m_fViewportWidth( 0.0f )
    , m_fViewportHeight( 0.0f )
{
	m_vLightDirection.Normalise();
	m_pObjects = new ObjectList();
}

// $Barnyard: FUNCTION 00644a50
// $Barnyard: FUNCTION 00644b50
AGUI2ModelRenderer::~AGUI2ModelRenderer()
{
	DestroyObjects();

	if ( m_pViewport )
		delete m_pViewport;

	if ( m_pCamera )
		delete m_pCamera;

	delete m_pObjects;
}

// $Barnyard: FUNCTION 006441c0
void AGUI2ModelRenderer::UpdateViewport()
{
	T2GUITransform oTransform;
	GetScreenTransform( oTransform );

	m_pViewport->SetWidth( oTransform.GetScaleX() * m_fViewportWidth );
	m_pViewport->SetHeight( oTransform.GetScaleY() * m_fViewportHeight );
	m_pViewport->SetX( oTransform.GetTranslation().x - m_pViewport->GetWidth() * 0.5f );
	m_pViewport->SetY( oTransform.GetTranslation().y - m_pViewport->GetHeight() * 0.5f );
}

// $Barnyard: FUNCTION 00644270
TBOOL AGUI2ModelRenderer::CreateFromViewport( TViewport* a_pViewport, TCameraObject* a_pCamera, TBOOL a_bClearBackground )
{
	m_pViewport = new TViewport( TTRUE );
	m_pViewport->SetX( a_pViewport->GetX() );
	m_pViewport->SetY( a_pViewport->GetY() );
	m_pViewport->SetWidth( a_pViewport->GetWidth() );
	m_pViewport->SetHeight( a_pViewport->GetHeight() );

	TRenderContext* pSourceContext = a_pViewport->GetRenderContext();
	TRenderContext* pContext       = m_pViewport->GetRenderContext();
	pContext->SetCameraMode( pSourceContext->GetCameraMode() );
	pContext->GetUnk16C() = pSourceContext->GetUnk16C();
	pContext->SetAmbientColour( pSourceContext->GetAmbientColour() );
	pContext->SetFlag( TRenderContext::FLAG_DISTANCEFADE1, pSourceContext->GetFlags() & TRenderContext::FLAG_DISTANCEFADE1 );

	m_pViewport->AllowDepthClear( TTRUE );
	m_pViewport->AllowBackgroundClear( a_bClearBackground );
	m_pViewport->SetBackgroundColor( 0, 0, 0, 0 );
	m_pViewport->Enable( TTRUE );
	m_pViewport->EnableDefaultBeginRender( TTRUE );
	m_pViewport->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

	// The same camera
	m_pCamera = new TCameraObject();
	m_pCamera->SetFOV( a_pCamera->GetFOV() );
	m_pCamera->GetTransformObject().SetMatrix( a_pCamera->GetTransformObject().GetMatrix() );
	m_pCamera->SetNear( a_pCamera->GetNear() );
	m_pCamera->SetFar( a_pCamera->GetFar() );
	m_pCamera->SetProjectionCentreX( a_pCamera->GetProjectionCentreX() );
	m_pCamera->SetProjectionCentreY( a_pCamera->GetProjectionCentreY() );

	m_fViewportWidth  = m_pViewport->GetWidth();
	m_fViewportHeight = m_pViewport->GetHeight();
	SetDimensions( a_pViewport->GetWidth(), a_pViewport->GetHeight() );

	m_bCreated = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006444e0
TBOOL AGUI2ModelRenderer::Create( TFLOAT a_fWidth, TFLOAT a_fHeight, TBOOL a_bClearBackground, TBOOL a_bCameraMode )
{
	m_pViewport = new TViewport( TTRUE );
	m_pViewport->SetX( 0.0f );
	m_pViewport->SetY( 0.0f );
	m_pViewport->SetWidth( a_fWidth );
	m_pViewport->SetHeight( a_fHeight );
	m_pViewport->AllowDepthClear( TTRUE );
	m_pViewport->AllowBackgroundClear( a_bClearBackground );
	m_pViewport->SetBackgroundColor( 0, 0, 0, 0 );
	m_pViewport->Enable( TTRUE );
	m_pViewport->EnableDefaultBeginRender( TTRUE );
	m_pViewport->SetMemoryAllocatorBlock( AMemory::GetMemBlock( AMemory::POOL_Viewport ) );

	if ( a_bCameraMode == TTRUE )
		m_pViewport->GetRenderContext()->SetCameraMode( TRenderContext::CameraMode( 1 ) );

	m_pCamera = new TCameraObject();
	m_pCamera->SetFOV( ACamera::sm_fCurrentFOV );
	m_pCamera->GetTransformObject().SetMatrix( TMatrix44::IDENTITY );
	m_pCamera->SetNear( 0.5f );
	m_pCamera->SetFar( 200.0f );
	m_pCamera->SetProjectionCentreX( 0.5f );
	m_pCamera->SetProjectionCentreY( 0.5f );

	m_fViewportWidth  = m_pViewport->GetWidth();
	m_fViewportHeight = m_pViewport->GetHeight();
	SetDimensions( a_fWidth, a_fHeight );

	m_bCreated = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006446b0
void AGUI2ModelRenderer::SetCameraMatrix( const TMatrix44& a_rcMatrix )
{
	m_pCamera->GetTransformObject().SetMatrix( a_rcMatrix );
}

// $Barnyard: FUNCTION 006448a0
void AGUI2ModelRenderer::SetCameraPosition( const TVector4& a_rcPosition )
{
	TMatrix44 mTransform                                 = TMatrix44::IDENTITY;
	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcPosition;

	m_pCamera->GetTransformObject().SetMatrix( mTransform );
}

// $Barnyard: FUNCTION 006446d0
void AGUI2ModelRenderer::SetLightDirection( const TVector4& a_rcDirection )
{
	m_vLightDirection = a_rcDirection;
	m_vLightDirection.Normalise();
}

// $Barnyard: FUNCTION 00644900
void AGUI2ModelRenderer::DestroyObjects()
{
	while ( m_pObjects->iNumObjects != 0 )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pObjects->aObjects[ m_pObjects->iNumObjects - 1 ].pObject );
		m_pObjects->RemoveLast();
	}
}

// $Barnyard: FUNCTION 00644970
void AGUI2ModelRenderer::SetObjectVisible( TINT a_iIndex, TBOOL a_bVisible )
{
	Object& rObject = m_pObjects->aObjects[ a_iIndex ];

	if ( rObject.pObject )
		rObject.bVisible = a_bVisible;
}

// $Barnyard: FUNCTION 00644ad0
TINT AGUI2ModelRenderer::FindObject( AAnimatableObject* a_pObject )
{
	for ( TINT i = 0; i < m_pObjects->iNumObjects; i++ )
	{
		if ( m_pObjects->aObjects[ i ].pObject == a_pObject )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00644b70
void AGUI2ModelRenderer::Tick( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_pObjects->iNumObjects; i++ )
	{
		Object& rObject = m_pObjects->aObjects[ i ];

		if ( rObject.bVisible != TTRUE )
			continue;

		TSceneObject* pSceneObject = rObject.pObject->GetModelInstance()->GetSceneObject();
		pSceneObject->Update( a_fDeltaTime );

		// The attached objects need the bones of the object
		if ( !rObject.pObject->GetAttachedObjects().IsEmpty() )
			pSceneObject->GetInstance()->GetSkeletonInstance()->UpdateState( TFALSE );
	}
}

// $Barnyard: FUNCTION 00644070
void AGUI2ModelRenderer::RenderObjectCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	Object*           pObject = TSTATICCAST( Object, a_pUserData );
	TRenderInterface* pRender = TRenderInterface::GetSingleton();

	TMatrix44 mOldLightColour = pRender->GetLightColour();

	TMatrix44 mLightColour;
	TUtil::MemSet( &mLightColour, 0, sizeof( mLightColour ) );
	mLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) = pObject->vLightColour;

	TRenderContext* pContext      = pRender->GetCurrentContext();
	TVector4        vecOldAmbient = pContext->GetAmbientColour();
	pContext->SetAmbientColour( pObject->vAmbientColour );

	pRender->SetLightColourMatrix( mLightColour );

	// Render the model the way it's rendered normally
	pObject->fnRenderCallback( a_pInstance, pObject->pRenderCallbackData );

	pRender->GetCurrentContext()->SetAmbientColour( vecOldAmbient );
	pRender->SetLightColourMatrix( mOldLightColour );
}

// $Barnyard: FUNCTION 00645040
void AGUI2ModelRenderer::AddObject( AAnimatableObject* a_pObject, const TVector4* a_pLightColour, const TVector4* a_pAmbientColour )
{
	Object oObject;
	oObject.pObject        = a_pObject;
	oObject.bVisible       = TTRUE;
	oObject.vLightColour   = a_pLightColour ? *a_pLightColour : TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	oObject.vAmbientColour = a_pAmbientColour ? *a_pAmbientColour : TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

	// Render the object with its own colours
	TModelInstance* pInstance   = a_pObject->GetModelInstance()->GetSceneObject()->GetInstance();
	oObject.fnRenderCallback    = pInstance->GetCustomRenderMethod();
	oObject.pRenderCallbackData = pInstance->GetCustomRenderUserData();

	m_pObjects->aObjects[ m_pObjects->iNumObjects++ ] = oObject;
	Object* pObject                                   = &m_pObjects->aObjects[ m_pObjects->iNumObjects - 1 ];

	pInstance->SetCustomRenderMethod( RenderObjectCallback, pObject );

	T2_FOREACH( pObject->pObject->GetAttachedObjects(), it )
	{
		it->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( RenderObjectCallback, pObject );
	}
}

// $Barnyard: FUNCTION 00645220
TINT AGUI2ModelRenderer::RemoveObject( AAnimatableObject* a_pObject )
{
	TINT iIndex = FindObject( a_pObject );

	if ( iIndex == -1 )
		return -1;

	Object& rObject = m_pObjects->aObjects[ iIndex ];
	a_pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	// Restore the render callbacks
	rObject.pObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( rObject.fnRenderCallback, rObject.pRenderCallbackData );

	T2_FOREACH( rObject.pObject->GetAttachedObjects(), it )
	{
		it->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( rObject.fnRenderCallback, rObject.pRenderCallbackData );
	}

	m_pObjects->Remove( iIndex );
	return iIndex;
}

// $Barnyard: FUNCTION 006453a0
void AGUI2ModelRenderer::RemoveAllObjects()
{
	while ( m_pObjects->iNumObjects != 0 )
	{
		Object& rObject = m_pObjects->aObjects[ m_pObjects->iNumObjects - 1 ];
		rObject.pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		rObject.pObject->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( rObject.fnRenderCallback, rObject.pRenderCallbackData );

		m_pObjects->RemoveLast();
	}
}

// $Barnyard: FUNCTION 00645420
void AGUI2ModelRenderer::SetAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < m_pObjects->iNumObjects; i++ )
		m_pObjects->aObjects[ i ].pObject->GetModelInstance()->SetOpacity( a_fAlpha );

	T2GUIElement::SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 006454d0
void AGUI2ModelRenderer::SetObjectOpacity( TINT a_iIndex, TFLOAT a_fOpacity )
{
	AAnimatableObject* pObject = m_pObjects->aObjects[ a_iIndex ].pObject;
	pObject->GetModelInstance()->SetOpacity( a_fOpacity );

	T2_FOREACH( pObject->GetAttachedObjects(), it )
	{
		it->GetModelInstance()->SetOpacity( a_fOpacity );
	}
}

// $Barnyard: FUNCTION 00644cb0
void AGUI2ModelRenderer::RenderObjects()
{
	TRenderInterface* pRender = TRenderInterface::GetSingleton();

	// The light comes from m_vLightDirection...
	TMatrix44 mOldLightDirection = pRender->GetLightDirection();

	TMatrix44 mLightDirection;
	TUtil::MemSet( &mLightDirection, 0, sizeof( mLightDirection ) );
	mLightDirection.m_f11 = m_vLightDirection.x;
	mLightDirection.m_f12 = m_vLightDirection.y;
	mLightDirection.m_f13 = m_vLightDirection.z;
	pRender->SetLightDirectionMatrix( mLightDirection );

	// ...and it's white
	TMatrix44 mOldLightColour = pRender->GetLightColour();

	TMatrix44 mLightColour;
	TUtil::MemSet( &mLightColour, 0, sizeof( mLightColour ) );
	mLightColour.m_f11 = 1.0f;
	mLightColour.m_f12 = 1.0f;
	mLightColour.m_f13 = 1.0f;
	mLightColour.m_f14 = 1.0f;
	pRender->SetLightColourMatrix( mLightColour );

	for ( TINT i = 0; i < m_pObjects->iNumObjects; i++ )
	{
		Object& rObject = m_pObjects->aObjects[ i ];

		if ( !rObject.bVisible )
			continue;

		TSceneObject* pSceneObject = rObject.pObject->GetModelInstance()->GetSceneObject();

		// Scale the light direction like the model
		TMatrix44 mTransform;
		pSceneObject->GetTransform().GetLocalMatrixImp( mTransform );

		TFLOAT   fScale       = mTransform.AsBasisVector4( BASISVECTOR_RIGHT ).Magnitude();
		TVector4 vecDirection = m_vLightDirection;
		vecDirection.Multiply( fScale );

		TUtil::MemSet( &mLightDirection, 0, sizeof( mLightDirection ) );
		mLightDirection.m_f11 = vecDirection.x;
		mLightDirection.m_f12 = vecDirection.y;
		mLightDirection.m_f13 = vecDirection.z;
		pRender->SetLightDirectionMatrix( mLightDirection );

		pSceneObject->EnableSkeletonUpdate();
		pSceneObject->RenderIfVisible();
		pSceneObject->DisableSkeletonUpdate();

		T2_FOREACH( rObject.pObject->GetAttachedObjects(), it )
		{
			it->UpdateAttachmentTransform();

			TSceneObject* pAttachedSceneObject = it->GetModelInstance()->GetSceneObject();
			pAttachedSceneObject->EnableSkeletonUpdate();
			pAttachedSceneObject->RenderIfVisible();
			pAttachedSceneObject->DisableSkeletonUpdate();
		}
	}

	pRender->SetLightDirectionMatrix( mOldLightDirection );
	pRender->SetLightColourMatrix( mOldLightColour );
}

// $Barnyard: FUNCTION 006455e0
void AGUI2ModelRenderer::Render()
{
	if ( m_pObjects->iNumObjects == 0 || !m_bRender )
		return;

	TRenderD3DInterface* pRender      = TRenderD3DInterface::Interface();
	TViewport*           pGUIViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();
	TRenderContext*      pOldContext  = pRender->GetCurrentContext();

	// Stop rendering the GUI...
	T2GUI::GetRenderer()->ResetRenderer();
	pGUIViewport->End();

	// ...render the objects into the viewport...
	pRender->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	m_pViewport->GetRenderContext()->SetAmbientColour( s_vViewportAmbient );
	UpdateViewport();

	m_pViewport->Begin();
	m_pCamera->Render();

	auto& rTransformStack = pRender->GetTransforms();
	rTransformStack.Push( m_pViewport->GetRenderContext()->GetWorldViewMatrix() );

	TCameraObject* pOldCamera = m_pViewport->GetRenderContext()->GetCameraObject();
	m_pViewport->GetRenderContext()->SetCameraObject( m_pCamera );
	RenderObjects();
	m_pViewport->GetRenderContext()->SetCameraObject( pOldCamera );

	rTransformStack.Pop();

	m_pViewport->End();
	pRender->FlushShaders();
	pRender->SetCurrentRenderContext( pOldContext );

	// ...and continue with the GUI
	TBOOL bAllowDepthClear = pGUIViewport->AllowDepthClear( TTRUE );
	pGUIViewport->Begin();
	pGUIViewport->AllowDepthClear( bAllowDepthClear );

	T2GUI::GetRenderer()->PrepareRenderer();
	T2GUIElement::Render();
}
