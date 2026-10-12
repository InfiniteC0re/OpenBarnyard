#include "ToshiPCH.h"
#include "TMouse_D3D8_Win.h"
#include "Platform/DX8/TRenderInterface_DX8.h"

#include <d3dx8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006de570, CreateObject 006de5f0, CreateObjectInPlace 006de630 (the constructor is inlined into both)
TDEFINE_CLASS( TMouse_D3D8_Win );

// $Barnyard: FUNCTION 006de670
TMouse_D3D8_Win::~TMouse_D3D8_Win()
{
	// NOTE: retail releases the surface without checking it
	m_pCursorSurface->Release();
}

// $Barnyard: FUNCTION 006de580
TBOOL TMouse_D3D8_Win::SetCursor( const TWCHAR* a_wszFileName )
{
	IDirect3DDevice8* pDevice = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() )->GetDirect3DDevice();

	if ( FAILED( pDevice->CreateImageSurface( 32, 32, D3DFMT_A8R8G8B8, &m_pCursorSurface ) ) )
		return TFALSE;

	if ( FAILED( D3DXLoadSurfaceFromFileW( m_pCursorSurface, TNULL, TNULL, a_wszFileName, TNULL, D3DX_FILTER_LINEAR, 0xff000000, TNULL ) ) )
		return TFALSE;

	pDevice->SetCursorProperties( 0, 0, m_pCursorSurface );
	pDevice->ShowCursor( TRUE );

	return TTRUE;
}

TOSHI_NAMESPACE_END
