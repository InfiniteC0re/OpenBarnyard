#pragma once
#include "Math/Math.h"
#include "Math/TSphere.h"
#include "Toshi/TStack.h"

namespace Toshi {

	class TRenderInterface;
	class TSkeletonInstance;

	class TRenderContext
	{
	public:
		typedef TUINT32 FLAG;
		enum FLAG_ : FLAG
		{
			FLAG_DIRTY = BITFLAG(0),
			FLAG_FOG = BITFLAG(1),
			FLAG_HAS_MODELWORLDMATRIX = BITFLAG(2),
			FLAG_HAS_VIEWWORLDMATRIX = BITFLAG(3),
			FLAG_UNK3 = BITFLAG(4),
			FLAG_UNK4 = BITFLAG(5),
			FLAG_HAS_WORLDPLANES = BITFLAG(6),
			FLAG_UNK6 = BITFLAG(7),
			FLAG_DIRTY_WORLDMODELMATRIX = BITFLAG(8),
			FLAG_DIRTY_VIEWMODELMATRIX = BITFLAG(9),
		};

		typedef TUINT32 CameraMode;
		enum CameraMode_ : CameraMode
		{
			CameraMode_Perspective,
			CameraMode_Orthographic,
		};

		typedef TUINT32 WORLDPLANE;
		enum WORLDPLANE_ : WORLDPLANE
		{
			WORLDPLANE_LEFT,
			WORLDPLANE_RIGHT,
			WORLDPLANE_BOTTOM,
			WORLDPLANE_TOP,
			WORLDPLANE_NEAR,
			WORLDPLANE_FAR,
		};

		struct Params
		{
			float fX;
			float fY;
			float fWidth;
			float fHeight;
			float fMinZ;
			float fMaxZ;
		};

		struct PROJECTIONPARAMS
		{
			TVector2 m_Centre;
			TVector2 m_Proj;
			float m_fNearClip;
			float m_fFarClip;
		};

	protected:
		void SetDirty(TBOOL a_bDirty);
		void SetFlag(FLAG a_eFlag, TBOOL a_bEnable);

		void EnableFog(TBOOL a_bEnable);
		void SetFogDistance(TFLOAT a_fStart, TFLOAT a_fEnd);

	public:
		TRenderContext(TRenderInterface* pRender);

		virtual void SetModelViewMatrix(const TMatrix44& a_rMatrix);
		virtual void SetWorldViewMatrix(const TMatrix44& a_rMatrix);
		virtual void Update() = 0;

		void SetProjectionParams(const PROJECTIONPARAMS& params);

		void SetViewportParameters(const Params& params)
		{
			m_oParams = params;
			m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6))) | FLAG_DIRTY;
		}

		void SetCameraMode(CameraMode cameraMode)
		{
			m_eCameraMode = cameraMode;
			m_eFlags = (m_eFlags & (~(FLAG_UNK3 | FLAG_UNK4 | FLAG_HAS_WORLDPLANES | FLAG_UNK6))) | FLAG_DIRTY;
		}

		Params& GetParams() { return m_oParams; }
		float GetX() const { return m_oParams.fX; }
		float GetY() const { return m_oParams.fY; }
		float GetWidth() const { return m_oParams.fWidth; }
		float GetHeight() const { return m_oParams.fHeight; }

		TBOOL IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
		TBOOL IsDirty() const { return m_eFlags & FLAG_DIRTY; }

		TSkeletonInstance* GetSkeletonInstance() const
		{
			return m_pCurrentSkeletonInstance;
		}

		void SetSkeletonInstance(TSkeletonInstance* a_pSkeletonInstance)
		{
			m_pCurrentSkeletonInstance = a_pSkeletonInstance;
		}

		TUINT GetClipFlags() const
		{
			return m_eClipFlags;
		}

		TUINT SetClipFlags(TUINT a_uiClipFlags)
		{
			auto uiOldFlags = m_eClipFlags;
			m_eClipFlags = a_uiClipFlags;
			return uiOldFlags;
		}

		const TPlane* GetWorldPlanes();

		static TBOOL CullSphereToFrustumSimple(const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes);
		static TINT CullSphereToFrustum(const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask);

		const TMatrix44& GetViewWorldMatrix();
		const TMatrix44& GetWorldModelMatrix();
		const TMatrix44& GetModelWorldMatrix();
		const TMatrix44& GetViewModelMatrix();
		TMatrix44& GetModelViewMatrix() { return m_oModelViewMatrix; }
		TMatrix44& GetWorldViewMatrix() { return m_oWorldViewMatrix; }

		TFLOAT GetAlphaBlend() const { return m_fAlphaBlend; }
		void SetAlphaBlend(TFLOAT val) { m_fAlphaBlend = val; }

		const PROJECTIONPARAMS& GetProjectionParams() const
		{
			return m_ProjParams;
		}

	protected:
		TRenderInterface* m_pRenderer;                 // 0x0004
		FLAG m_eFlags;                                 // 0x0008
		TUINT m_eClipFlags;                            // 0x000C
		TUINT m_Unk;                                   // 0x0010
		CameraMode m_eCameraMode;                      // 0x0014
		Params m_oParams;                              // 0x0018
		PROJECTIONPARAMS m_ProjParams;                 // 0x0030
		TSkeletonInstance* m_pCurrentSkeletonInstance; // 0x0048
		TMatrix44 m_oModelViewMatrix;                  // 0x004C
		TMatrix44 m_oWorldViewMatrix;                  // 0x008C
		TMatrix44 m_oModelWorldMatrix;                 // 0x00CC
		TMatrix44 m_oViewWorldMatrix;                  // 0x010C
		TVector4 m_AmbientColor;                       // 0x014C
		TVector4 m_FogColor;                           // 0x015C
		// ...
		TFLOAT m_fFogDistanceStart;                    // 0x01A0
		TFLOAT m_fFogDistanceEnd;                      // 0x01A4
		TPlane m_aFrustumPlanes1[6];                   // 0x01A8
		TPlane m_aWorldPlanes[6];                      // 0x0208
		TPlane m_aFrustumPlanes2[6];                   // 0x0268
		// ...
		TMatrix44 m_oWorldModelMatrix;                 // 0x032C
		TMatrix44 m_oViewModelMatrix;                  // 0x0368
		TFLOAT m_fAlphaBlend;                          // 0x03A8
	};

}
