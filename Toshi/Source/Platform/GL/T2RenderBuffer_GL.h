#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "GL/glew.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class which is not the same as the one used in TOSHI 2.0
// since it was never actually used in any games but in some tools like TXSViewer which
// is not publicly available, so it's not possible to match this class.
//-----------------------------------------------------------------------------
template <GLenum Type>
class T2RenderBuffer
{
public:
	constexpr T2RenderBuffer()
	    : m_uiId( 0 ) {}

	constexpr T2RenderBuffer( GLuint a_uiId )
	    : m_uiId( a_uiId ) {}

	constexpr T2RenderBuffer( const T2RenderBuffer& a_rOther )
	    : m_uiId( a_rOther.m_uiId ) {}

	constexpr GLuint GetId() const { return m_uiId; }

	void SetData( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage )
	{
		TASSERT( m_uiId != 0 );
		glBindBuffer( Type, m_uiId );
		glBufferData( Type, a_uiSize, a_pData, a_eUsage );
	}

	void Bind() { glBindBuffer( Type, m_uiId ); }
	void Unbind() { glBindBuffer( Type, 0 ); }
	void Destroy() { glDeleteBuffers( 1, &m_uiId ); }

	constexpr operator TBOOL() const { return m_uiId != 0; }

private:
	GLuint m_uiId;
};

using T2VertexBuffer = T2RenderBuffer<GL_ARRAY_BUFFER>;
using T2IndexBuffer  = T2RenderBuffer<GL_ELEMENT_ARRAY_BUFFER>;

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
