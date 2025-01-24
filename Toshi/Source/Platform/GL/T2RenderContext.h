#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "T2Viewport_GL.h"
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
	};

public:
	T2RenderContext();
	~T2RenderContext();

	TMatrix44&       GetProjectionMatrix() { return m_matProjection; }
	const TMatrix44& GetProjectionMatrix() const { return m_matProjection; }

public:
	static void ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static void ComputePerspectiveFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static TBOOL CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes );
	static TINT  CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );

private:
	TMatrix44 m_matProjection;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
