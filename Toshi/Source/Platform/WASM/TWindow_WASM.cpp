#include "ToshiPCH.h"
#include "TWindow_WASM.h"
#include "TCGLStream_WASM.h"

#include <emscripten.h>
#include <emscripten/threading.h>
#include <math.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

// The page fills these from its animation frames and the size of its canvas
int g_iWASMFrameCounter;
int g_iWASMPageWidth  = 800;
int g_iWASMPageHeight = 600;

extern "C" EMSCRIPTEN_KEEPALIVE int* WASM_GetFrameCounter()
{
	return &g_iWASMFrameCounter;
}

extern "C" EMSCRIPTEN_KEEPALIVE int* WASM_GetPageSize()
{
	// Width, height, and if the page is hidden
	static int s_aSize[ 3 ] = { 0, 0, 0 };
	return s_aSize;
}

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TWindow_WASM );

TWindow_WASM::TWindow_WASM()
{
	m_uiWidth  = 800;
	m_uiHeight = 600;
}

TWindow_WASM::~TWindow_WASM()
{
}

void TWindow_WASM::Update( TFLOAT a_fDeltaTime )
{
	// Callbacks of html5.h wait for this thread
	emscripten_current_thread_process_queued_calls();
}

TBOOL TWindow_WASM::Create( TRenderInterface* a_pRenderer, const TCHAR* a_szTitle )
{
	return TTRUE;
}

void TWindow_WASM::UnregisterWindowClass()
{
}

TBOOL TWindow_WASM::CreateContext()
{
	if ( !TCGLStream::CreateContext( m_uiWidth, m_uiHeight ) )
	{
		TERROR( "TWindow_WASM: couldn't create a WebGL2 context\n" );
		return TFALSE;
	}

	return TTRUE;
}

void TWindow_WASM::DestroyContext()
{
	TCGLStream::DestroyContext();
}

void TWindow_WASM::SwapBuffers()
{
	TCGLStream::Present();

	// Wait for an animation frame only if the page hasn't had one since the last present, or a frame that is
	// a little late would lose a whole refresh
	static int s_iLastFrame = 0;

	int* pCounter = WASM_GetFrameCounter();
	int  iFrame   = __atomic_load_n( pCounter, __ATOMIC_SEQ_CST );

	if ( iFrame == s_iLastFrame )
	{
		// Hidden pages get no animation frames but the game keeps going. A visible page that is late has a GPU
		// that is behind, another frame now would never be seen and would only make it later
		emscripten_futex_wait( pCounter, iFrame, __atomic_load_n( &WASM_GetPageSize()[ 2 ], __ATOMIC_SEQ_CST ) ? 100.0 : 1000.0 );
		iFrame = __atomic_load_n( pCounter, __ATOMIC_SEQ_CST );
	}

	s_iLastFrame = iFrame;
}

TBOOL TWindow_WASM::IsExtensionSupported( const TCHAR* a_szExtension ) const
{
	return TCGLStream::EnableExtension( a_szExtension );
}

void TWindow_WASM::SetDisplayMode( TUINT a_uiWidth, TUINT a_uiHeight, TBOOL a_bWindowed, TBOOL a_bBorderless )
{
	m_uiWidth  = a_uiWidth;
	m_uiHeight = a_uiHeight;
	TCGLStream::ResizeCanvas( a_uiWidth, a_uiHeight );
}

void TWindow_WASM::GetPageSize( TINT& a_riWidth, TINT& a_riHeight )
{
	const int* pSize = WASM_GetPageSize();
	a_riWidth        = pSize[ 0 ] > 0 ? pSize[ 0 ] : 800;
	a_riHeight       = pSize[ 1 ] > 0 ? pSize[ 1 ] : 600;
}

TOSHI_NAMESPACE_END
