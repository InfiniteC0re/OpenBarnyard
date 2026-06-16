#pragma once
#include "THookedSingleton.h"

#include <Math/TMatrix44.h>
#include <Math/TPlane.h>
#include <Render/TRenderContext.h>
#include <Toshi/TSceneObject.h>

class AGlowViewport : public THookedSingleton<AGlowViewport, 0x0079b180>
{
public:
	class GlowObject
	{
	public:
		GlowObject();
		~GlowObject() = default;

		void Setup(
			const Toshi::TMatrix44& a_rcTransform,
			const Toshi::TRenderContext::VIEWPORTPARAMS& a_rcViewportParams,
			const Toshi::TRenderContext::PROJECTIONPARAMS& a_rcProjectionParams,
			Toshi::TRenderContext::CameraMode              a_eCameraMode
		)
		{
			m_oViewportParams   = a_rcViewportParams;
			m_oProjectionParams = a_rcProjectionParams;
			m_eCameraMode       = a_eCameraMode;

			Toshi::TMatrix44 oProjection;
			Toshi::TFrustum oFrustum;

			if ( a_eCameraMode == Toshi::TRenderContext::CameraMode_Orthographic )
			{
				Toshi::TRenderContext::ComputeOrthographicProjection( oProjection, a_rcViewportParams, a_rcProjectionParams );
				Toshi::TRenderContext::ComputeOrthographicFrustum( oFrustum, a_rcViewportParams, a_rcProjectionParams );
			}
			else
			{
				Toshi::TRenderContext::ComputePerspectiveProjection( oProjection, a_rcViewportParams, a_rcProjectionParams );
				Toshi::TRenderContext::ComputePerspectiveFrustum( oFrustum, a_rcViewportParams, a_rcProjectionParams );
			}

			Toshi::TMatrix44 oTransformInverted;
			oTransformInverted.Invert( a_rcTransform );

			for ( TINT i = 0; i < Toshi::WORLDPLANE_NUMOF; i++ )
			{
				Toshi::TMatrix44::TransformPlaneOrthogonal( m_oFrustum[ i ], oTransformInverted, oFrustum[ i ] );
			}

			m_oMVP.Multiply( oProjection, a_rcTransform );
		}

		TBOOL IsEnabled() const { return m_bEnabled; }
		TBOOL IsNightLight() const { return m_bIsNightLight; }

	public:
		Toshi::TMatrix44                        m_oMVP;
		TINT                                    Unknown1;
		Toshi::TRenderContext::VIEWPORTPARAMS   m_oViewportParams;
		Toshi::TRenderContext::PROJECTIONPARAMS m_oProjectionParams;
		Toshi::TRenderContext::CameraMode       m_eCameraMode;
		Toshi::TSceneObject*                    m_pSceneObject;
		TINT                                    m_iAttachBone;
		TINT                                    m_eTransformType;
		Toshi::TFrustum                         m_oFrustum;
		TBOOL                                   m_bIsNightLight;
		TBOOL                                   m_bEnabled;
		Toshi::TMatrix44                        m_oTransform;
		GlowObject*                             m_pNextObject;
		Toshi::TLightID                         m_iID;
	};

public:
	// constructors/destructor
	AGlowViewport( TINT a_iMaxNumGlowingObjects );
	~AGlowViewport();

	// Obtains IDs of glow objects that can see the sphere
	void GetInfluencingLightIDs(const Toshi::TSphere& a_rcSphere, Toshi::TLightIDList& a_rLightIDs)
	{
		a_rLightIDs.Reset();

		TINT iNumLights = 0;
		for ( GlowObject* pGlowObject = m_pHeadUsedObject; pGlowObject != TNULL; pGlowObject = pGlowObject->m_pNextObject )
		{
			// Check if the object is enabled, visible and affects the sphere
			if ( pGlowObject->IsEnabled() &&
			     ( !pGlowObject->IsNightLight() || *(TINT*)( *(TCHAR**)( 0x00783d3c ) + 0x34 ) == 4 ) &&
			     Toshi::TRenderContext::CullSphereToFrustumSimple( a_rcSphere, pGlowObject->m_oFrustum, 6 ) )
			{
				// The object is influencing the specified sphere
				a_rLightIDs[ iNumLights++ ] = pGlowObject->m_iID;

				if ( iNumLights >= Toshi::TLightIDList::MAX_NUM_LIGHTS )
					return;
			}
		}
	}

	GlowObject* CreateGlowObject()
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

	void RemoveGlowObject( GlowObject* a_pGlowObject )
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

public:
	TINT        m_iMaxNumObjects;
	GlowObject* m_pAllocObjects;
	GlowObject* m_pHeadUsedObject;
	GlowObject* m_pHeadFreeObject;
};
