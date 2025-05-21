#include "pch.h"
#include "AGlowViewport.h"
#include "Helpers/AGameTimeManager.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 006086f0
AGlowViewport::AGlowViewport( TINT a_iMaxNumGlowObjects )
    : m_iMaxNumObjects( a_iMaxNumGlowObjects ), m_pHeadFreeObject( TNULL )
{
	TASSERT( a_iMaxNumGlowObjects < ( 1 << 8 * sizeof( TLightID ) ) - 1 );
	m_pAllocObjects = new GlowObject[ a_iMaxNumGlowObjects ];

	// Initialise all objects except the last one
	for ( TLightID i = 0; i < a_iMaxNumGlowObjects - 1; i++ )
	{
		m_pAllocObjects[ i ].m_iID         = i;
		m_pAllocObjects[ i ].m_pNextObject = &m_pAllocObjects[ i + 1 ];
	}

	// Initialise the last object
	m_pAllocObjects[ a_iMaxNumGlowObjects - 1 ].m_iID         = a_iMaxNumGlowObjects - 1;
	m_pAllocObjects[ a_iMaxNumGlowObjects - 1 ].m_pNextObject = TNULL;

	m_pHeadUsedObject = TNULL;
	m_pHeadFreeObject = m_pAllocObjects;
}

// $Barnyard: FUNCTION 006087f0
AGlowViewport::~AGlowViewport()
{
	delete[] m_pAllocObjects;
}

AGlowViewport::GlowObject* AGlowViewport::CreateGlowObject()
{
	if ( m_pHeadFreeObject == TNULL )
		return TNULL;

	// Get the glow object to return
	GlowObject* pGlowObject = m_pHeadFreeObject;

	// Update head free object
	m_pHeadFreeObject = pGlowObject->m_pNextObject;

	// Update it's next object to be the one that was previously created
	pGlowObject->m_pNextObject = m_pHeadUsedObject;

	// Update head used object
	m_pHeadUsedObject = pGlowObject;

	return pGlowObject;
}

// $Barnyard: FUNCTION 00608560
void AGlowViewport::RemoveGlowObject( GlowObject* a_pGlowObject )
{
	if ( a_pGlowObject == TNULL )
		return;

	TASSERT( m_pHeadUsedObject != TNULL );

	GlowObject* pPrevObject = TNULL;
	GlowObject* pGlowObject = m_pHeadUsedObject;
	for ( ; pGlowObject != TNULL; pGlowObject = pGlowObject->m_pNextObject )
	{
		if ( pGlowObject == a_pGlowObject )
			break;

		pPrevObject = pGlowObject;
	}

	// Verificate we found the needed object
	TASSERT( pGlowObject == a_pGlowObject );

	if ( pPrevObject != TNULL )
	{
		// Removing some linked object
		pPrevObject->m_pNextObject = pGlowObject->m_pNextObject;
		pGlowObject->m_pNextObject = m_pHeadFreeObject;
		m_pHeadFreeObject          = pGlowObject;
	}
	else
	{
		// Removing head object
		m_pHeadUsedObject          = m_pHeadUsedObject->m_pNextObject;
		pGlowObject->m_pNextObject = m_pHeadFreeObject;
		m_pHeadFreeObject          = pGlowObject;
	}
}

// $Barnyard: FUNCTION 00608be0
void AGlowViewport::GetInfluencingLightIDs( const TSphere& a_rcSphere, TLightIDList& a_rLightIDs )
{
	a_rLightIDs.Reset();

	TINT iNumLights = 0;
	for ( GlowObject* pGlowObject = m_pHeadUsedObject; pGlowObject != TNULL; pGlowObject = pGlowObject->m_pNextObject )
	{
		// Check if the object is enabled, visible and affects the sphere
		if ( pGlowObject->IsEnabled() &&
		     ( pGlowObject->IsNightLight() || AGameTimeManager::GetSingleton()->IsItNight() ) &&
		     TRenderContext::CullSphereToFrustumSimple( a_rcSphere, pGlowObject->m_oFrustum, WORLDPLANE_NUMOF ) )
		{
			// The object is influencing the specified sphere
			a_rLightIDs[ iNumLights++ ] = pGlowObject->m_iID;

			if ( iNumLights >= TLightIDList::MAX_NUM_LIGHTS )
				return;
		}
	}
}

// $Barnyard: FUNCTION 00608810
void AGlowViewport::Update()
{
	TRenderContext* pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	// Clear list of visible lights
	pRenderContext->ClearLightIDs();

	// Update the used glow objects and add lights to the render context if necessary
	for ( GlowObject* pGlowObject = m_pHeadUsedObject; pGlowObject != TNULL; pGlowObject = pGlowObject->m_pNextObject )
	{
		if ( TSceneObject* pSceneObject = pGlowObject->m_pSceneObject )
		{
			// Set light at bone position and add it to the render context
			TModelInstance* pModelInstance = pSceneObject->GetInstance();

			if ( !pModelInstance )
				continue;

			TSkeletonInstance* pSkeletonInstance = pModelInstance->GetSkeletonInstance();

			// Get local bone transform
			TMatrix44 oBoneTransform;
			pSkeletonInstance->GetBoneTransformCurrent( pGlowObject->m_iAttachBone, oBoneTransform );

			// Get model instance transform to calculate world bone transform later
			TMatrix44 oLightTransform;
			pSceneObject->GetTransform().GetLocalMatrixImp( oLightTransform );

			// Get world bone transform
			oLightTransform.Multiply( oBoneTransform );

			// Swap forward and up axes
			TVector4 vNewForwardVec = oLightTransform.AsBasisVector4( BASISVECTOR_UP );
			vNewForwardVec.Negate4();

			oLightTransform.AsBasisVector4( BASISVECTOR_UP )      = oLightTransform.AsBasisVector4( BASISVECTOR_FORWARD );
			oLightTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vNewForwardVec;

			// Append glow object's transform
			oLightTransform.Multiply( pGlowObject->m_oTransform );

			switch ( pGlowObject->m_eTransformType )
			{
				case -3:
					oLightTransform.Scale( -1.0f, -1.0f, -1.0f );
					break;
				case -2:
					oLightTransform.Scale( -1.0f, -1.0f, -1.0f );
				case 1:
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_UP ), oLightTransform.AsBasisVector4( BASISVECTOR_FORWARD ) );
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_RIGHT ), oLightTransform.AsBasisVector4( BASISVECTOR_UP ) );
					break;
				case -1:
					oLightTransform.Scale( -1.0f, -1.0f, -1.0f );
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_RIGHT ), oLightTransform.AsBasisVector4( BASISVECTOR_FORWARD ) );
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_RIGHT ), oLightTransform.AsBasisVector4( BASISVECTOR_UP ) );
					break;
				case 0:
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_RIGHT ), oLightTransform.AsBasisVector4( BASISVECTOR_FORWARD ) );
					TVector4::Swap( oLightTransform.AsBasisVector4( BASISVECTOR_RIGHT ), oLightTransform.AsBasisVector4( BASISVECTOR_UP ) );
					break;
			}

			// Set the transform to the glow object
			oLightTransform.InvertOrthogonal();
			pGlowObject->Setup( oLightTransform, pGlowObject->m_oViewportParams, pGlowObject->m_oProjectionParams, pGlowObject->m_eCameraMode );

			// Finally, add light to the context
			pRenderContext->AddLight( pGlowObject->m_iID );
		}
		else
		{
			// Set position of this light far away because no model is bound to the object
			TVector4 vLightDir( 1.0f, -1.1f, 0.0f );
			vLightDir.Normalise();
			vLightDir.Negate3();

			TQuaternion qLightRot;
			qLightRot.SetVectorDirection( vLightDir.AsVector3(), TVector3::VEC_POSZ );

			TVector4 vLightPos = vLightDir;
			vLightPos.Negate3();
			vLightPos *= 100.0f;

			TMatrix44 oMVP(
			    1.0f, 0.0f, 0.0f, 0.0f,
			    0.0f, 0.0f, 1.0f, 0.0f,
			    0.0f, -1.0f, 0.0f, 0.0f,
			    0.0f, 0.0f, 0.0f, 1.0f
			);

			TMatrix44::TransformVector( oMVP.GetTranslation(), oMVP, vLightPos );
			pGlowObject->m_oMVP = oMVP;
		}
	}
}

// $Barnyard: FUNCTION 00608630
AGlowViewport::GlowObject::GlowObject()
{
	Unknown1         = 0;
	m_pSceneObject   = TNULL;
	m_iAttachBone    = -1;
	m_eTransformType = 2;

	m_oTransform    = TMatrix44::IDENTITY;
	m_bIsNightLight = TTRUE;

	TRenderContext::VIEWPORTPARAMS oViewportParams;
	oViewportParams.fX      = 0.0f;
	oViewportParams.fY      = 0.0f;
	oViewportParams.fMinZ   = 0.0f;
	oViewportParams.fMaxZ   = 1.0f;
	oViewportParams.fWidth  = 1.0f;
	oViewportParams.fHeight = 1.0f;

	TRenderContext::PROJECTIONPARAMS oProjectionParams;
	oProjectionParams.SetFromFOV(
	    oViewportParams.fWidth,
	    oViewportParams.fHeight,
	    TMath::DegToRad( 45.0f ),
	    0.1f,
	    30.0f
	);

	Setup( TMatrix44::IDENTITY, oViewportParams, oProjectionParams, TRenderContext::CameraMode_Perspective );
}

// $Barnyard: FUNCTION 006081d0
void AGlowViewport::GlowObject::Setup( const Toshi::TMatrix44& a_rcTransform, const Toshi::TRenderContext::VIEWPORTPARAMS& a_rcViewportParams, const Toshi::TRenderContext::PROJECTIONPARAMS& a_rcProjectionParams, Toshi::TRenderContext::CameraMode a_eCameraMode )
{
	m_oViewportParams   = a_rcViewportParams;
	m_oProjectionParams = a_rcProjectionParams;
	m_eCameraMode       = a_eCameraMode;

	TMatrix44 oProjection;
	TFrustum  oFrustum;

	if ( a_eCameraMode == TRenderContext::CameraMode_Orthographic )
	{
		TRenderContext::ComputeOrthographicProjection( oProjection, a_rcViewportParams, a_rcProjectionParams );
		TRenderContext::ComputeOrthographicFrustum( oFrustum, a_rcViewportParams, a_rcProjectionParams );
	}
	else
	{
		TRenderContext::ComputePerspectiveProjection( oProjection, a_rcViewportParams, a_rcProjectionParams );
		TRenderContext::ComputePerspectiveFrustum( oFrustum, a_rcViewportParams, a_rcProjectionParams );
	}

	TMatrix44 oTransformInverted;
	oTransformInverted.Invert( a_rcTransform );

	for ( TINT i = 0; i < WORLDPLANE_NUMOF; i++ )
	{
		TMatrix44::TransformPlaneOrthogonal( m_oFrustum[ i ], oTransformInverted, oFrustum[ i ] );
	}

	m_oMVP.Multiply( oProjection, a_rcTransform );
}
