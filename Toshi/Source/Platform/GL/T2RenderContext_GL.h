#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "T2Viewport_GL.h"
#  include "T2GLTexture_GL.h"
#  include "T2Shader_GL.h"
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

	TBOOL SetShaderProgram( const T2Shader& a_rcShaderProgram )
	{
		if ( m_uiCurrentShaderProgram != a_rcShaderProgram.GetProgram() )
		{
			a_rcShaderProgram.Use();
			m_uiCurrentShaderProgram = a_rcShaderProgram.GetProgram();

			return TTRUE;
		}

		return TFALSE;
	}

	GLuint GetTexture2D( TINT a_iTextureIndex )
	{
		return m_aCurrentTextures[ a_iTextureIndex ];
	}

	void SetTexture2D( TINT a_iTextureIndex, GLuint a_uiTexture )
	{
		if ( m_aCurrentTextures[ a_iTextureIndex ] != a_uiTexture )
		{
			if ( m_iCurrentTextureUnit != a_iTextureIndex )
			{
				glActiveTexture( GL_TEXTURE0 + a_iTextureIndex );
				m_iCurrentTextureUnit = a_iTextureIndex;
			}

			glBindTexture( GL_TEXTURE_2D, a_uiTexture );
			m_aCurrentTextures[ a_iTextureIndex ] = a_uiTexture;
		}
	}

	void SetTexture2D( TINT a_iTextureIndex, const T2GLTexture& a_rcTexture )
	{
		SetTexture2D( a_iTextureIndex, a_rcTexture.GetHandle() );
	}

	void ResetTexture2D( TINT a_iTextureIndex )
	{
		if ( m_iCurrentTextureUnit != a_iTextureIndex )
		{
			glActiveTexture( GL_TEXTURE0 + a_iTextureIndex );
			glBindTexture( GL_TEXTURE_2D, NULL );

			m_iCurrentTextureUnit                 = a_iTextureIndex;
			m_aCurrentTextures[ a_iTextureIndex ] = -1;
		}
	}

public:
	static void ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static void ComputePerspectiveFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );
	static void ComputeOrthographicFrustum( Frustum& a_rcFrustum, const T2Viewport& a_rViewportParams, const Projection& a_rProjParams );

	static TBOOL CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes );
	static TINT  CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );

private:
	TMatrix44 m_matProjection;
	GLuint    m_uiCurrentShaderProgram = -1;
	TINT      m_iCurrentTextureUnit    = 0;
	GLuint    m_aCurrentTextures[ 8 ];
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
