#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "T2Viewport_GL.h"
#  include "T2GLTexture_GL.h"
#  include "T2Shader_GL.h"
#  include "T2Camera_GL.h"
#  include "Math/TMatrix44.h"
#  include "Math/TSphere.h"

TOSHI_NAMESPACE_START

typedef TUINT32 FRUSTUMPLANE;
enum FRUSTUMPLANE_ : FRUSTUMPLANE
{
	FRUSTUMPLANE_LEFT,
	FRUSTUMPLANE_RIGHT,
	FRUSTUMPLANE_BOTTOM,
	FRUSTUMPLANE_TOP,
	FRUSTUMPLANE_NEAR,
	FRUSTUMPLANE_FAR,
	FRUSTUMPLANE_NUMOF
};

//-----------------------------------------------------------------------------
// Note: This is a custom class.
//-----------------------------------------------------------------------------
class T2RenderContext
{
public:
	using Frustum = TPlane[ FRUSTUMPLANE_NUMOF ];

	struct Projection
	{
		TVector2 vecCenter;
		TVector2 vecProj;
		TFLOAT   fNearClip;
		TFLOAT   fFarClip;

		void SetFromFOV( TFLOAT a_fViewportWidth, TFLOAT a_fViewportHeight, TFLOAT a_fFOV, TFLOAT a_fNearPlane, TFLOAT a_fFarPlane );
	};

public:
	T2RenderContext();
	~T2RenderContext();

	void UpdateCamera();

	void      SetCamera( T2Camera& a_rCamera ) { m_pCamera = &a_rCamera; }
	T2Camera* GetCamera() const { return m_pCamera; }

	T2Viewport& GetViewport() { return m_oViewport; }
	const T2Viewport& GetViewport() const { return m_oViewport; }

	TMatrix44&       GetProjectionMatrix() { return m_matProjection; }
	const TMatrix44& GetProjectionMatrix() const { return m_matProjection; }

	TMatrix44&       GetViewMatrix() { return m_matViewMatrix; }
	const TMatrix44& GetViewMatrix() const { return m_matViewMatrix; }

	void  SetViewMatrix( const TMatrix44& a_rMatrix );
	TBOOL SetShaderProgram( const T2Shader& a_rcShaderProgram );

	GLuint GetTexture2D( TINT a_iTextureIndex );
	void   SetTexture2D( TINT a_iTextureIndex, GLuint a_uiTexture );
	void   SetTexture2D( TINT a_iTextureIndex, const T2GLTexture& a_rcTexture );
	void   ResetTexture2D( TINT a_iTextureIndex );

	void ForceRefreshFeatures();

	void EnableDepthTest( TBOOL a_bEnable );
	void EnableBlend( TBOOL a_bEnable );

public:
	static void ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static void ComputePerspectiveFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static TBOOL CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes );
	static TINT  CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );

private:
	TMatrix44 m_matProjection;
	TMatrix44 m_matViewMatrix;
	GLuint    m_uiCurrentShaderProgram;

	Projection m_oProjectionParams;

	TINT      m_iCurrentTextureUnit;
	GLuint    m_aCurrentTextures[ 8 ];

	T2Viewport m_oViewport;
	T2Camera* m_pCamera;

	// Render parameters
	TBOOL m_bForceRefreshFeatures : 1;
	TBOOL m_bDepthTest : 1;
	TBOOL m_bBlend : 1;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
