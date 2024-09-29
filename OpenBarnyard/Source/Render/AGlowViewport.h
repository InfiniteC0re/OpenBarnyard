#pragma once
#include "Cameras/AViewport.h"

#include <Math/TMatrix44.h>
#include <Math/TPlane.h>
#include <Render/TRenderContext.h>
#include <Toshi/TSceneObject.h>

class AGlowViewport :
    public Toshi::TSingleton<AGlowViewport>
{
public:
	class GlowObject
	{
	public:
		GlowObject();
		~GlowObject() = default;

		void Setup(
		    const Toshi::TMatrix44&                        a_rcTransform,
		    const Toshi::TRenderContext::VIEWPORTPARAMS&   a_rcViewportParams,
		    const Toshi::TRenderContext::PROJECTIONPARAMS& a_rcProjectionParams,
		    Toshi::TRenderContext::CameraMode              a_eCameraMode
		);

		TBOOL IsEnabled() const { return m_bEnabled; }
		TBOOL IsNightLight() const { return m_bIsNightLight; }

	public:
		Toshi::TMatrix44                        m_oMVP;
		TINT                                    Unknown1;
		Toshi::TRenderContext::VIEWPORTPARAMS   m_oViewportParams;
		Toshi::TRenderContext::PROJECTIONPARAMS m_oProjectionParams;
		Toshi::TRenderContext::CameraMode       m_eCameraMode;
		TINT                                    m_iAttachBone;
		Toshi::TSceneObject*                    m_pSceneObject;
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

	// Gets glow object from free list and returns pointer to it
	GlowObject* CreateGlowObject();

	// Removes glow object from the used list
	void RemoveGlowObject( GlowObject* a_pGlowObject );

	// Obtains IDs of glow objects that can see the sphere
	void GetInfluencingLightIDs( const Toshi::TSphere& a_rcSphere, Toshi::TLightIDList& a_rLightIDs );

	// Prepares created glow objects for rendering
	void Update();

private:
	TINT        m_iMaxNumObjects;
	GlowObject* m_pAllocObjects;
	GlowObject* m_pHeadUsedObject;
	GlowObject* m_pHeadFreeObject;
};
