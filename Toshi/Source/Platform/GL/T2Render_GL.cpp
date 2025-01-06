#include "ToshiPCH.h"
#include "Render/T2Render.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2Render::T2Render()
{

}

T2Render::~T2Render()
{

}

TBOOL T2Render::Create( const WindowParams& a_rcWindowParams )
{
	return TFALSE;
}

void T2Render::Destroy()
{
}

T2VertexBuffer T2Render::CreateVertexBuffer( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage )
{
	GLuint uiBufferId;
	glGenBuffers( 1, &uiBufferId );

	TASSERT( uiBufferId != 0 );
	T2VertexBuffer buffer( uiBufferId );

	if ( a_uiSize != 0 )
	{
		buffer.SetData( a_pData, a_uiSize, a_eUsage );
	}

	return buffer;
}

T2IndexBuffer T2Render::CreateIndexBuffer( const TUINT16* a_pIndices, GLuint a_uiCount, GLenum a_eUsage )
{
	GLuint uiBufferId;
	glGenBuffers( 1, &uiBufferId );

	TASSERT( uiBufferId != 0 );
	T2IndexBuffer buffer( uiBufferId );

	if ( a_uiCount != 0 )
	{
		buffer.SetData( a_pIndices, sizeof( *a_pIndices ) * a_uiCount, a_eUsage );
	}

	return buffer;
}

T2VertexArray T2Render::CreateVertexArray()
{
	GLuint uiVaoId;
	glGenVertexArrays( 1, &uiVaoId );

	TASSERT( uiVaoId != 0 );
	return uiVaoId;
}

T2VertexArray T2Render::CreateVertexArray( T2VertexBuffer a_VertexBuffer, T2IndexBuffer a_IndexBuffer )
{
	GLuint uiVaoId;
	glGenVertexArrays( 1, &uiVaoId );
	TASSERT( uiVaoId != 0 );

	T2VertexArray vao( uiVaoId );
	vao.SetVertexBuffer( a_VertexBuffer );
	vao.SetIndexBuffer( a_IndexBuffer );

	return vao;
}

TOSHI_NAMESPACE_END
