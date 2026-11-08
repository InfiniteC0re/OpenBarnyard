#pragma once
#include "Toshi/TObject.h"
#include "Render/TRenderInterface.h"

TOSHI_NAMESPACE_START

// The canvas and its WebGL context are on the page, see TCGLStream
class TWindow_WASM : public TObject
{
public:
	TDECLARE_CLASS( TWindow_WASM, TObject );

public:
	TWindow_WASM();
	~TWindow_WASM();

	void Update( TFLOAT a_fDeltaTime );

	TBOOL Create( TRenderInterface* a_pRenderer, const TCHAR* a_szTitle );
	void  UnregisterWindowClass();

	TBOOL CreateContext();
	void  DestroyContext();
	void  SwapBuffers();
	TBOOL IsExtensionSupported( const TCHAR* a_szExtension ) const;

	void SetDisplayMode( TUINT a_uiWidth, TUINT a_uiHeight, TBOOL a_bWindowed, TBOOL a_bBorderless );
	void RestoreDisplayMode() {}
	void CenterCursor( TUINT a_uiWidth, TUINT a_uiHeight ) {}

	TBOOL IsDestroyed() const { return TFALSE; }
	TBOOL IsWindowed() const { return TTRUE; }

	static TBOOL IsBackgroundMode() { return TFALSE; }
	static TBOOL IsNoPauseMode() { return TFALSE; }
	static void  SetFocused( TBOOL a_bFocused ) {}
	static void  Dev_SetKeepRunning( TBOOL a_bKeepRunning ) {}

	// Size of the canvas on the page
	static void GetPageSize( TINT& a_riWidth, TINT& a_riHeight );

private:
	TUINT m_uiWidth;
	TUINT m_uiHeight;
};

TOSHI_NAMESPACE_END
