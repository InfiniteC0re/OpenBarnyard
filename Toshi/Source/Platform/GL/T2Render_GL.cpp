#include "ToshiPCH.h"
#include "Render/T2Render.h"
#include "File/TFile.h"

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

T2CompiledShader T2Render::CompileShader( GLenum a_eShader, const TCHAR* a_szSource )
{
	T2CompiledShader shader = glCreateShader( a_eShader );
	TASSERT( shader );

	glShaderSource( shader.GetId(), 1, &a_szSource, TNULL );
	glCompileShader( shader.GetId() );

	GLint compileSuccess = 0;
	glGetShaderiv( shader.GetId(), GL_COMPILE_STATUS, &compileSuccess );

	if ( compileSuccess == GL_FALSE )
	{
		GLint logLength;
		glGetShaderiv( shader.GetId(), GL_INFO_LOG_LENGTH, &logLength );

		GLchar* log = new GLchar[ logLength - 1 ];
		glGetShaderInfoLog( shader.GetId(), logLength - 1, nullptr, log );

		TERROR( "Unable to compile shader\n" );
		TERROR( "Error: %s\n", log );
		delete[] log;
	}

	return shader;
}

T2Shader T2Render::CreateShaderProgram( T2CompiledShader a_VertexShader, T2CompiledShader a_FragmentShader )
{
	T2Shader program( a_VertexShader, a_FragmentShader );
	program.Create();

	return program;
}

T2CompiledShader T2Render::CompileShaderFromFile( GLenum a_eShader, const TCHAR* a_szFileName )
{
	TFile* pFile    = TFile::Create( a_szFileName );
	TSIZE  uiFileSize = pFile->GetSize();

	TCHAR* pSrc    = new TCHAR[ uiFileSize + 1 ];
	pFile->Read( pSrc, uiFileSize );
	pSrc[ uiFileSize ] = '\0';
	pFile->Destroy();

	T2CompiledShader shader = T2Render::CompileShader( a_eShader, pSrc );
	delete[] pSrc;

	return shader;
}

TOSHI_NAMESPACE_END
