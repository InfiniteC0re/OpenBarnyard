#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "Toshi/TString8.h"
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
	    : m_uiId( 0 )
	{}

	constexpr T2RenderBuffer( GLuint a_uiId )
	    : m_uiId( a_uiId )
	{}

	constexpr T2RenderBuffer( const T2RenderBuffer& a_rOther )
	    : m_uiId( a_rOther.m_uiId )
	{}

	constexpr GLuint GetId() const { return m_uiId; }

	void SetData( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage )
	{
		glBufferData( Type, a_uiSize, a_pData, a_eUsage );
	}

	void SetAttribPointer( GLuint a_uiIndex, GLint a_iNumComponents, GLenum a_eType, GLsizei a_iStride, GLsizei a_pOffset, GLboolean a_bNormalized = GL_FALSE )
	{
		glVertexAttribPointer( a_uiIndex, a_iNumComponents, a_eType, a_bNormalized, a_iStride, (const void*)a_pOffset );
		glEnableVertexAttribArray( a_uiIndex );
	}

	void Bind() { glBindBuffer( Type, m_uiId ); }
	void Unbind() { glBindBuffer( Type, 0 ); }
	void Clear() { m_uiId = 0; }

	constexpr operator TBOOL() const { return m_uiId != 0; }

private:
	GLuint m_uiId;
};

using T2VertexBuffer = T2RenderBuffer<GL_ARRAY_BUFFER>;
using T2IndexBuffer  = T2RenderBuffer<GL_ELEMENT_ARRAY_BUFFER>;

class T2VertexArray
{
public:
	constexpr T2VertexArray()
	    : m_uiId( 0 )
	{}

	constexpr T2VertexArray( GLuint a_uiId )
	    : m_uiId( a_uiId )
	{}

	constexpr T2VertexArray( GLuint a_uiId, T2VertexBuffer a_VertexBuffer, T2IndexBuffer a_IndexBuffer )
	    : m_uiId( a_uiId ), m_VertexBuffer( a_VertexBuffer ), m_IndexBuffer( a_IndexBuffer )
	{}

	constexpr T2VertexArray( const T2VertexArray& a_rOther )
	    : m_uiId( a_rOther.m_uiId ), m_VertexBuffer( a_rOther.m_VertexBuffer ), m_IndexBuffer( a_rOther.m_IndexBuffer )
	{}

	void SetVertexBuffer( T2VertexBuffer a_VertexBuffer )
	{
		TASSERT( m_uiId != 0 );
		a_VertexBuffer.Bind();
		m_VertexBuffer = a_VertexBuffer;
	}

	void SetIndexBuffer( T2IndexBuffer a_IndexBuffer )
	{
		TASSERT( m_uiId != 0 );
		a_IndexBuffer.Bind();
		m_IndexBuffer = a_IndexBuffer;
	}

	constexpr GLuint GetId() const { return m_uiId; }

	inline static GLuint ms_uiBoundId = 0;

	TBOOL Bind()
	{
		if ( ms_uiBoundId != m_uiId )
		{
			glBindVertexArray( m_uiId );
			ms_uiBoundId = m_uiId;
			return TTRUE;
		}

		return TFALSE;
	}

	static void Unbind()
	{
		if ( ms_uiBoundId != 0 )
		{
			glBindVertexArray( 0 );
			ms_uiBoundId = 0;
		}
	}

	TBOOL IsBound() const
	{
		return ms_uiBoundId == m_uiId;
	}

	void Destroy()
	{
		glDeleteVertexArrays( 1, &m_uiId );
		ms_uiBoundId = 0;
	}

	constexpr T2VertexBuffer GetVertexBuffer() { return m_VertexBuffer; }
	constexpr T2IndexBuffer  GetIndexBuffer() { return m_IndexBuffer; }

	constexpr operator TBOOL() const { return m_uiId != 0; }

private:
	GLuint         m_uiId;
	T2VertexBuffer m_VertexBuffer;
	T2IndexBuffer  m_IndexBuffer;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
