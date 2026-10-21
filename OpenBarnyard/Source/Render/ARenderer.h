#pragma once
#include "Cameras/ACamera.h"
#include "ASkyDome.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/ADisplayModes_Win.h"
#endif // TOSHI_SKU_WINDOWS

#include <Toshi/TTask.h>
#include <Render/TViewport.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderAdapter.h>

class MODAPI ARenderer : public Toshi::TTask
    , public Toshi::TSingleton<ARenderer>
{
public:
	TDECLARE_CLASS( ARenderer, Toshi::TTask );

	enum SCREENCAPTURE : TUINT32
	{
		SCREENCAPTURE_NONE,
		SCREENCAPTURE_REQUESTED,
		SCREENCAPTURE_CAPTURING,
		SCREENCAPTURE_READY,
	};

#ifdef BARNYARD_COMMUNITY_PATCH
	using CreateShaders_t = void ( * )( ARenderer* );

	// The shaders of the game, in the order OnCreate creates them
	using SHADERS = TUINT32;
	enum SHADERS_ : SHADERS
	{
		SHADERS_LINE           = BITFLAG( 0 ),
		SHADERS_WORLD          = BITFLAG( 1 ),
		SHADERS_SKIN           = BITFLAG( 2 ),
		SHADERS_GRASS          = BITFLAG( 3 ),
		SHADERS_STATICINSTANCE = BITFLAG( 4 ),
		SHADERS_SYS            = BITFLAG( 5 ),
		SHADERS_ALL            = SHADERS_LINE | SHADERS_WORLD | SHADERS_SKIN | SHADERS_GRASS | SHADERS_STATICINSTANCE | SHADERS_SYS,
	};
#endif // BARNYARD_COMMUNITY_PATCH

public:
	ARenderer();
	~ARenderer();

	virtual TBOOL Reset() override { return TTRUE; } // +0x14, RETURNS_TRUE 0046cde0 (doesn't reset the children)
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	TBOOL CreateTRender();
	TBOOL CreateTRenderResources();

	void UpdateMainCamera( const Toshi::TMatrix44& a_rTransformMatrix, const ACamera* a_pCamera );

	// Ray from a_pCamera through a_rcScreenPos (projection space of the main viewport).
	// a_rOutPosition gets the position of the camera, a_rOutDirection the normalised direction.
	void GetScreenRay( const ACamera* a_pCamera, const Toshi::TVector2& a_rcScreenPos, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection );
	void GetScreenRay( const Toshi::TVector4& a_rcCameraPos, const Toshi::TQuaternion& a_rcCameraRot, const Toshi::TVector2& a_rcScreenPos, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection );

	// Projects a_rcPosition on the screen (pixels of the main viewport) with the current camera.
	// Returns TTRUE if it's on the screen, otherwise a_rOutScreenPos is clamped to the edge (moved in by the margins).
	TBOOL GetScreenPosition( const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutScreenPos );
	TBOOL GetScreenPosition( const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutScreenPos, TFLOAT a_fMarginLeft, TFLOAT a_fMarginRight, TFLOAT a_fMarginTop, TFLOAT a_fMarginBottom );

	// Projects a_rPoint (in place, x and y) on the main viewport, scaled to the width of the GUI system's viewport.
	// Points behind the near clip plane are moved to ( -100, -100 ).
	void WorldToScreen( Toshi::TVector4& a_rPoint );

	// WorldToScreen, then scaled to the dimensions of the AGUI2 root element.
	void WorldToGUI( Toshi::TVector4& a_rPoint );

	// Projects a_rcPosition with a_pCamera into the pixels of the main viewport, TTRUE if it's between the clip planes
	TBOOL ProjectToScreen( ACamera* a_pCamera, const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutScreenPos );

	// ProjectToScreen in the coordinates of the AGUI2 root element
	TBOOL ProjectToGUI( ACamera* a_pCamera, const Toshi::TVector4& a_rcPosition, Toshi::TVector2& a_rOutGUIPos );

	// Scales a_rcScreenPos from the pixels of the main viewport to the GUI viewport (clipped to it)
	void ScreenToGUIViewport( const Toshi::TVector2& a_rcScreenPos, Toshi::TVector2& a_rOutPos );

	// Scales a_rcGUIPos from the GUI viewport to the pixels of the main viewport (clipped to it)
	void GUIViewportToScreen( const Toshi::TVector2& a_rcGUIPos, Toshi::TVector2& a_rOutPos );

	// Width and height of the GUI viewport
	void GetGUIViewportSize( Toshi::TVector2& a_rOutSize );

	// Size of the render context of the main viewport
	void GetMainViewportSize( Toshi::TVector2& a_rOutSize );

	void ForceUpdate30FPS();

	// Writes the finished screen capture to Data/Screenshots/<a_szName without extension><index>.tga
	TBOOL SaveScreenCapture( const TCHAR* a_szName );

	Toshi::TRenderAdapter::Mode::Device* FindSuitableDevice( Toshi::TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder );

	Toshi::TViewport*     GetMainViewport() const { return m_pViewport; }
	Toshi::TCameraObject* GetCameraObject() const { return m_pCameraObject; }

	Toshi::TEmitter<ARenderer, TBOOL*>& GetAnimationUpdateStartEmitter() { return m_AnimationUpdateStartEmitter; }
	Toshi::TEmitter<ARenderer, TBOOL*>& GetAnimationUpdateEndEmitter() { return m_AnimationUpdateEndEmitter; }

	Toshi::TEmitter<ARenderer, TINT>&       GetRenderGUIEmitter() { return m_RenderGUIEmitter; }
	const Toshi::TEmitter<ARenderer, TINT>& GetRenderGUIEmitter() const { return m_RenderGUIEmitter; }

	ASkyDome* GetSkyDome() { return m_pSkyDome; }

	// Returns the previous near clip distance
	// $Barnyard: FUNCTION 0060afd0
	TFLOAT SetNear( TFLOAT a_fNear ) { return std::exchange( m_fNear, a_fNear ); }

	// Returns the previous far clip distance
	// $Barnyard: FUNCTION 0060afc0
	TFLOAT SetFar( TFLOAT a_fFar ) { return std::exchange( m_fFar, a_fFar ); }

	TFLOAT GetNear() const { return m_fNear; }
	TFLOAT GetFar() const { return m_fFar; }

#ifdef BARNYARD_COMMUNITY_PATCH
	static void SetCreateShadersFunction( CreateShaders_t a_fnCallback ) { ms_fnCreateShadersCb = a_fnCallback; }

	static void CreateDefaultShaders( SHADERS a_eShaders = SHADERS_ALL );
#endif // BARNYARD_COMMUNITY_PATCH

private:
	void CreateMainViewport();
	void RenderGUI();
	void RenderMainScene( TFLOAT a_fDeltaTime );

	// Takes the screen capture over two frames once m_eScreenCaptureStatus is SCREENCAPTURE_REQUESTED
	void UpdateScreenCapture( Toshi::TRenderInterface* a_pRender );

private:
	Toshi::TViewport*                  m_pViewport;
	Toshi::TCameraObject*              m_pCameraObject;
	Toshi::TViewport*                  m_pHALViewport1;
	Toshi::TViewport*                  m_pHALViewport2;
	TFLOAT                             m_fFar;
	TFLOAT                             m_fNear;
	TUINT32                            m_eScreenCaptureStatus;  // 0x3C
	Toshi::TRenderCapture*             m_pScreenCapture;        // 0x40
	TUINT8*                            m_pScreenCaptureBuffer;  // 0x44
	const TCHAR*                       m_szScreenCaptureName;   // 0x48
	TUINT32                            m_uiScreenCaptureWidth;  // 0x4C
	TUINT32                            m_uiScreenCaptureHeight; // 0x50
	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateStartEmitter;
	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateEndEmitter;
	Toshi::TEmitter<ARenderer, TINT>   m_RenderGUIEmitter;
	ASkyDome*                          m_pSkyDome;
	TINT                               m_iScreenshotIndex; // 0x84
	TBOOL                              m_bRenderGUI;
	ADisplayModes_Win                  m_DisplayModes;

#ifdef BARNYARD_COMMUNITY_PATCH
	inline static CreateShaders_t ms_fnCreateShadersCb = TNULL;
#endif // BARNYARD_COMMUNITY_PATCH
};
