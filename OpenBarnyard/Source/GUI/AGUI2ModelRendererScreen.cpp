#include "pch.h"
#include "AGUI2ModelRendererScreen.h"
#include "AGUI2.h"
#include "AGUISystem.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "Render/ARenderer.h"
#include "Tasks/ARootTask.h"

#include <GUI/T2GUI.h>
#include <Render/TViewport.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TSceneObject.h>
#include <Toshi/TScheduler.h>
#include <Toshi/TSystem.h>

#ifdef TRENDERINTERFACE_DX8
#  include <Platform/DX8/TRenderInterface_DX8.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006457c0
AGUI2ModelRendererScreen::AGUI2ModelRendererScreen()
    : m_iNumViews( 0 )
    , m_bEnabled( TTRUE )
{
	ms_pSingleton = this;

	// Look through the camera of the main viewport
	TViewport*     pViewport = ARenderer::GetSingleton()->GetMainViewport();
	TCameraObject* pCamera   = pViewport->GetRenderContext()->GetCameraObject();

	if ( !pCamera )
		pCamera = ARenderer::GetSingleton()->GetCameraObject();

	CreateFromViewport( pViewport, pCamera, TFALSE );
	AGUI2::GetRootElement()->AddChildTail( *this );
}

// $Barnyard: FUNCTION 00646230
// $Barnyard: FUNCTION 00646340
AGUI2ModelRendererScreen::~AGUI2ModelRendererScreen()
{
	if ( m_oUpdateTask.IsLinked() )
		g_oSystemManager.GetScheduler()->GetTree()->Remove( m_oUpdateTask, TFALSE );

	ms_pSingleton = TNULL;
}

// $Barnyard: FUNCTION 00645780
// $Barnyard: FUNCTION 00645790
AGUI2ModelRendererScreen::UpdateTask::~UpdateTask()
{
}

// $Barnyard: FUNCTION 00645900
TBOOL AGUI2ModelRendererScreen::UpdateTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	// Stay above the other elements
	AGUI2ModelRendererScreen* pScreen = AGUI2ModelRendererScreen::ms_pSingleton;
	pScreen->RemoveSelf();
	AGUI2::GetRootElement()->AddChildTail( *pScreen );

	return TTask::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00646170
void AGUI2ModelRendererScreen::SetActive( TBOOL a_bActive )
{
	TNodeTree<TTask>* pTree = g_oSystemManager.GetScheduler()->GetTree();

	if ( a_bActive )
	{
		if ( !m_oUpdateTask.IsLinked() )
		{
			pTree->InsertAtRootHead( &m_oUpdateTask );
			m_oUpdateTask.Create();
			pTree->ReInsert( ARootTask::GetSingleton(), &m_oUpdateTask );
		}

		if ( !m_oUpdateTask.IsActive() )
			m_oUpdateTask.Activate( TTRUE );

		RemoveSelf();
		AGUI2::GetRootElement()->AddChildTail( *this );
	}
	else if ( m_oUpdateTask.IsLinked() )
	{
		pTree->Remove( m_oUpdateTask, TFALSE );
	}
}

// $Barnyard: FUNCTION 00645770
void AGUI2ModelRendererScreen::RenderObjectCallback( TModelInstance* a_pInstance, void* a_pUserData )
{
	Object* pObject = TSTATICCAST( Object, a_pUserData );
	pObject->fnRenderCallback( a_pInstance, pObject->pRenderCallbackData );
}

// $Barnyard: FUNCTION 00645dc0
void AGUI2ModelRendererScreen::AddObjectWithoutColours( AAnimatableObject* a_pObject )
{
	Object oObject;
	oObject.pObject        = a_pObject;
	oObject.bVisible       = TTRUE;
	oObject.vLightColour   = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
	oObject.vAmbientColour = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

	TModelInstance* pInstance   = a_pObject->GetModelInstance()->GetSceneObject()->GetInstance();
	oObject.fnRenderCallback    = pInstance->GetCustomRenderMethod();
	oObject.pRenderCallbackData = pInstance->GetCustomRenderUserData();

	m_pObjects->aObjects[ m_pObjects->iNumObjects++ ] = oObject;
	Object* pObject                                   = &m_pObjects->aObjects[ m_pObjects->iNumObjects - 1 ];

	// NOTE: only the attached objects get the callback here
	T2_FOREACH( pObject->pObject->GetAttachedObjects(), it )
	{
		it->GetModelInstance()->GetSceneObject()->GetInstance()->SetCustomRenderMethod( RenderObjectCallback, pObject );
	}
}

// $Barnyard: FUNCTION 00645ff0
void AGUI2ModelRendererScreen::AddObject( AAnimatableObject* a_pObject, const TMatrix44& a_rcWorldView, const TMatrix44& a_rcCamera, const TMatrix44& a_rcLightDirection, TFLOAT a_fFOV )
{
	View& rView           = m_aViews[ m_iNumViews ];
	rView.mWorldView      = a_rcWorldView;
	rView.mCamera         = a_rcCamera;
	rView.mLightDirection = a_rcLightDirection;
	rView.fFOV            = a_fFOV;
	m_iNumViews++;

	AddObjectWithoutColours( a_pObject );
}

// $Barnyard: FUNCTION 006462c0
void AGUI2ModelRendererScreen::AddObject( AAnimatableObject* a_pObject )
{
	TRenderContext* pContext = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext();
	TCameraObject*  pCamera  = pContext->GetCameraObject();

	if ( !pCamera )
		pCamera = ARenderer::GetSingleton()->GetCameraObject();

	TMatrix44 mLightDirection = TRenderInterface::GetSingleton()->GetLightDirection();
	AddObject( a_pObject, pContext->GetWorldViewMatrix(), pCamera->GetTransformObject().GetMatrix(), mLightDirection, pCamera->GetFOV() );
}

// $Barnyard: FUNCTION 00645940
void AGUI2ModelRendererScreen::RemoveObject( AAnimatableObject* a_pObject )
{
	RemoveView( AGUI2ModelRenderer::RemoveObject( a_pObject ) );
}

// $Barnyard: FUNCTION 00645870
void AGUI2ModelRendererScreen::RemoveView( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < m_iNumViews - 1; i++ )
		m_aViews[ i ] = m_aViews[ i + 1 ];

	m_iNumViews--;
}

// $Barnyard: FUNCTION 00645c00
void AGUI2ModelRendererScreen::GetObjectWorldTransform( AAnimatableObject* a_pObject, TMatrix44& a_rOutTransform )
{
	View& rView = m_aViews[ FindObject( a_pObject ) ];

	TRenderContext* pContext = ARenderer::GetSingleton()->GetMainViewport()->GetRenderContext();
	TCameraObject*  pCamera  = pContext->GetCameraObject();

	if ( !pCamera )
		pCamera = ARenderer::GetSingleton()->GetCameraObject();

	a_pObject->GetModelInstance()->GetLocalTransform( a_rOutTransform );

	TMatrix44 mWorldViewInv = pContext->GetWorldViewMatrix();
	TMatrix44 mCameraInv    = pCamera->GetTransformObject().GetMatrix();
	mWorldViewInv.Invert( mWorldViewInv );
	mCameraInv.Invert( mCameraInv );

	// NOTE: these are calculated and not used
	TMatrix44 mViewWorldViewInv = rView.mWorldView;
	TMatrix44 mViewCameraInv    = rView.mCamera;
	mViewWorldViewInv.Invert( mViewWorldViewInv );
	mViewCameraInv.Invert( mViewCameraInv );

	TMatrix44 mTemp = a_rOutTransform;
	a_rOutTransform.Multiply( mTemp, rView.mWorldView );

	mTemp = a_rOutTransform;
	a_rOutTransform.Multiply( mTemp, rView.mCamera );

	mTemp = a_rOutTransform;
	a_rOutTransform.Multiply( mTemp, mCameraInv );

	mTemp = a_rOutTransform;
	a_rOutTransform.Multiply( mTemp, mWorldViewInv );
}

// $Barnyard: FUNCTION 00645980
void AGUI2ModelRendererScreen::RenderObjects()
{
	TRenderD3DInterface* pRender         = TRenderD3DInterface::Interface();
	auto&                rTransformStack = pRender->GetTransforms();
	TRenderContext*      pContext        = m_pViewport->GetRenderContext();
	TCameraObject*       pOldCamera      = pContext->GetCameraObject();
	TMatrix44            mOldLightDir    = pRender->GetLightDirection();
	TFLOAT               fOldFOV         = 0.0f;

	if ( pOldCamera )
		fOldFOV = pOldCamera->GetFOV();

	for ( TINT i = 0; i < m_pObjects->iNumObjects; i++ )
	{
		Object& rObject = m_pObjects->aObjects[ i ];
		View&   rView   = m_aViews[ i ];

		if ( !rObject.bVisible )
			continue;

		// Look at the object the way it was seen
		rTransformStack.Push( rView.mWorldView );

		pContext->SetCameraObject( m_pCamera );
		m_pCamera->SetFOV( rView.fFOV );
		pRender->SetLightDirectionMatrix( rView.mLightDirection );
		pContext->GetCameraObject()->GetTransformObject().SetMatrix( rView.mCamera );

		TSceneObject* pSceneObject = rObject.pObject->GetModelInstance()->GetSceneObject();
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

		rTransformStack.Pop();
	}

	pRender->SetLightDirectionMatrix( mOldLightDir );
	pContext->SetCameraObject( pOldCamera );

	if ( pOldCamera )
		pOldCamera->SetFOV( fOldFOV );
}

// $Barnyard: FUNCTION 00646090
void AGUI2ModelRendererScreen::Render()
{
	if ( !m_bEnabled || !GetNumObjects() )
		return;

	TRenderD3DInterface* pRender      = TRenderD3DInterface::Interface();
	TViewport*           pGUIViewport = AGUISystem::GetSingleton()->GetRenderObject( 0 )->GetViewport();
	TRenderContext*      pOldContext  = pRender->GetCurrentContext();

	// Stop rendering the GUI...
	T2GUI::GetRenderer()->ResetRenderer();
	pGUIViewport->End();

	// ...render the objects...
	pRender->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	m_pViewport->Begin();
	m_pCamera->Render();
	RenderObjects();
	m_pViewport->End();
	pRender->FlushShaders();
	pRender->SetCurrentRenderContext( pOldContext );

	// ...and continue with the GUI
	TBOOL bAllowDepthClear = pGUIViewport->AllowDepthClear( TFALSE );
	pGUIViewport->Begin();
	pGUIViewport->AllowDepthClear( bAllowDepthClear );

	T2GUI::GetRenderer()->PrepareRenderer();
	T2GUIElement::Render();
}
