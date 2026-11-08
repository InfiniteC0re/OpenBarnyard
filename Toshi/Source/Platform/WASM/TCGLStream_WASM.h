#pragma once

TOSHI_NAMESPACE_START

// The game thread never returns to its event loop and Chromium frees presented frames only on the event loop of the
// thread that owns the WebGL context (see Docs/OpenGLWebPort.md). So the context lives on the page: gl* calls are
// written to a ring buffer here and OpenBarnyard/WASM/gl_replay.js replays them.
class TCGLStream
{
public:
	// Sends what's written so far to the page
	static void Publish();

	// Waits until the page has replayed everything
	static void Finish();

	static TBOOL CreateContext( TUINT a_uiWidth, TUINT a_uiHeight );
	static void  DestroyContext();
	static TBOOL EnableExtension( const TCHAR* a_szName );
	static void  ResizeCanvas( TUINT a_uiWidth, TUINT a_uiHeight );

	// Ends the frame, waits if the page is behind
	static void Present();
};

TOSHI_NAMESPACE_END
