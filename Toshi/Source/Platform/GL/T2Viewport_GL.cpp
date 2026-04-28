#include "ToshiPCH.h"
#include "T2Viewport_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void T2Viewport::Begin()
{
	glClearColor( m_vClearColor.x, m_vClearColor.y, m_vClearColor.z, m_vClearColor.w );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport( 0, 0, (GLsizei)m_fWidth, (GLsizei)m_fHeight );
}

void T2Viewport::End()
{
}

TOSHI_NAMESPACE_END
