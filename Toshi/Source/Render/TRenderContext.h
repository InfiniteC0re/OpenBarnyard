#pragma once
#include "Math/Math.h"
#include "Math/TSphere.h"
#include "Toshi/TStack.h"

TOSHI_NAMESPACE_START

class TRenderInterface;
class TSkeletonInstance;
class TCameraObject;

using TLightID = TINT8;

struct TLightIDList
{
	inline static constexpr TINT MAX_NUM_LIGHTS = 4;

	void Add( TLightID a_iLightId );
	void Reset();

	TLightID& operator[]( TINT a_iIndex );

	TLightID aIDs[ MAX_NUM_LIGHTS ];
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
	WORLDPLANE_NUMOF
};

using TFrustum = TPlane[ WORLDPLANE_NUMOF ];

class TRenderContext
{
public:
	typedef TUINT32 FLAG;
	enum FLAG_ : FLAG
	{
		FLAG_DIRTY                  = BITFLAG( 0 ),
		FLAG_FOG                    = BITFLAG( 1 ),
		FLAG_HAS_MODELWORLDMATRIX   = BITFLAG( 2 ),
		FLAG_HAS_VIEWWORLDMATRIX    = BITFLAG( 3 ),
		FLAG_UNK3                   = BITFLAG( 4 ),
		FLAG_UNK4                   = BITFLAG( 5 ),
		FLAG_HAS_WORLDPLANES        = BITFLAG( 6 ),
		FLAG_UNK6                   = BITFLAG( 7 ),
		FLAG_DIRTY_WORLDMODELMATRIX = BITFLAG( 8 ),
		FLAG_DIRTY_VIEWMODELMATRIX  = BITFLAG( 9 ),
	};

	typedef TUINT32 CameraMode;
	enum CameraMode_ : CameraMode
	{
		CameraMode_Perspective,
		CameraMode_Orthographic,
	};

	struct VIEWPORTPARAMS
	{
		TFLOAT fX;
		TFLOAT fY;
		TFLOAT fWidth;
		TFLOAT fHeight;
		TFLOAT fMinZ;
		TFLOAT fMaxZ;
	};

	struct PROJECTIONPARAMS
	{
		TVector2 m_Centre;
		TVector2 m_Proj;
		TFLOAT   m_fNearClip;
		TFLOAT   m_fFarClip;

		void SetFromFOV( TFLOAT a_fViewportWidth, TFLOAT a_fViewportHeight, TFLOAT a_fFOV, TFLOAT a_fNearPlane, TFLOAT a_fFarPlane );
	};

protected:
	void SetDirty( TBOOL a_bDirty );
	void SetFlag( FLAG a_eFlag, TBOOL a_bEnable );

public:
	TRenderContext( TRenderInterface* pRender );
	virtual ~TRenderContext();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void SetModelViewMatrix( const TMatrix44& a_rMatrix );
	virtual void SetWorldViewMatrix( const TMatrix44& a_rMatrix );
	virtual void Update() = 0;

	void ClearLightIDs();
	void AddLight( TLightID a_iLightId );

	const PROJECTIONPARAMS& GetProjectionParams() const { return m_oProjParams; }
	void                    SetProjectionParams( const PROJECTIONPARAMS& a_rParams );

	const VIEWPORTPARAMS& GetViewportParameters() const { return m_oViewportParams; }
	void                  SetViewportParameters( const VIEWPORTPARAMS& a_rParams );

	CameraMode GetCameraMode() { return m_eCameraMode; }
	void       SetCameraMode( CameraMode a_eCameraMode );

	void EnableFog( TBOOL a_bEnable );
	void SetFogDistance( TFLOAT a_fStart, TFLOAT a_fEnd );

	TFLOAT GetX() const { return m_oViewportParams.fX; }
	TFLOAT GetY() const { return m_oViewportParams.fY; }
	TFLOAT GetWidth() const { return m_oViewportParams.fWidth; }
	TFLOAT GetHeight() const { return m_oViewportParams.fHeight; }

	TBOOL IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
	// $Barnyard: FUNCTION 006ce980
	TBOOL IsDirty() const { return m_eFlags & FLAG_DIRTY; }

	TSkeletonInstance* GetSkeletonInstance() const { return m_pCurrentSkeletonInstance; }
	void               SetSkeletonInstance( TSkeletonInstance* a_pSkeletonInstance ) { m_pCurrentSkeletonInstance = a_pSkeletonInstance; }

	TUINT GetClipFlags() const;
	TUINT SetClipFlags( TUINT a_uiClipFlags );

	const TPlane* GetWorldPlanes();

	const TMatrix44& GetViewWorldMatrix();
	const TMatrix44& GetWorldModelMatrix();
	const TMatrix44& GetModelWorldMatrix();
	const TMatrix44& GetViewModelMatrix();
	TMatrix44&       GetModelViewMatrix() { return m_oModelViewMatrix; }
	TMatrix44&       GetWorldViewMatrix() { return m_oWorldViewMatrix; }

	const TPlane* GetFrustumPlanes() const { return m_aFrustumPlanes1; }

	void           SetCameraObject( TCameraObject* a_pCameraObject ) { m_pCurrentCameraObject = a_pCameraObject; }
	TCameraObject* GetCameraObject() const { return m_pCurrentCameraObject; }

	TFLOAT GetAlphaBlend() const { return m_fAlphaBlend; }
	void   SetAlphaBlend( TFLOAT val ) { m_fAlphaBlend = val; }

	TFLOAT GetShadeCoeff() const { return m_fShadeCoeff; }
	void   SetShadeCoeff( TFLOAT val ) { m_fShadeCoeff = val; }

	void            SetAmbientColour( const TVector4& a_rcColour ) { m_AmbientColor = a_rcColour; }
	const TVector4& GetAmbientColour() const { return m_AmbientColor; }

	static void ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );
	static void ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );

	static void ComputePerspectiveFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );
	static void ComputeOrthographicFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );

	static TBOOL CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes );
	static TINT  CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );

#ifndef TOSHI_SDK
protected:
#endif

	TRenderInterface*  m_pRenderer;                // 0x0004
	FLAG               m_eFlags;                   // 0x0008
	TUINT              m_eClipFlags;               // 0x000C
	TUINT              m_Unk;                      // 0x0010
	CameraMode         m_eCameraMode;              // 0x0014
	VIEWPORTPARAMS     m_oViewportParams;          // 0x0018
	PROJECTIONPARAMS   m_oProjParams;              // 0x0030
	TSkeletonInstance* m_pCurrentSkeletonInstance; // 0x0048
	TMatrix44          m_oModelViewMatrix;         // 0x004C
	TMatrix44          m_oWorldViewMatrix;         // 0x008C
	TMatrix44          m_oModelWorldMatrix;        // 0x00CC
	TMatrix44          m_oViewWorldMatrix;         // 0x010C
	TVector4           m_AmbientColor;             // 0x014C
	TVector4           m_FogColor;                 // 0x015C
	// ...
	TFLOAT   m_fFogDistanceStart; // 0x01A0
	TFLOAT   m_fFogDistanceEnd;   // 0x01A4
	TFrustum m_aFrustumPlanes1;   // 0x01A8
	TFrustum m_aWorldPlanes;      // 0x0208
	TFrustum m_aFrustumPlanes2;   // 0x0268
	// ...
	TMatrix44 m_oWorldModelMatrix; // 0x032C
	TMatrix44 m_oViewModelMatrix;  // 0x0368
	TFLOAT    m_fAlphaBlend;       // 0x03A8
	// ...
	TFLOAT m_fShadeCoeff; // 0x03B0
	// TBOOL
	TLightIDList   m_oLightIds;            // 0x03B5
	TCameraObject* m_pCurrentCameraObject; // 0x03BC
};

TOSHI_NAMESPACE_END
