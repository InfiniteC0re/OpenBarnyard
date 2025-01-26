#include "ToshiPCH.h"
#include "Render/T2Render.h"
#include "File/TFile.h"
#include "T2GLTexture_GL.h"

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

TBOOL g_bCreated = TFALSE;

TBOOL T2Render::Create( const WindowParams& a_rcWindowParams )
{
	TASSERT( TFALSE == g_bCreated );

	TINFO( "Creating T2Render (OpenGL)\n" );

	SDL_Init( SDL_INIT_VIDEO );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	const TCHAR* pchWindowTitle = a_rcWindowParams.pchTitle;

	if ( !pchWindowTitle )
		pchWindowTitle = "TOSHI : T2Render";

	m_pWindow = new T2Window();
	if ( m_pWindow->Create( this, pchWindowTitle ) )
	{
		// Create SDL context
		m_pGLContext = SDL_GL_CreateContext( m_pWindow->GetNativeWindow() );
		SDL_GL_MakeCurrent( m_pWindow->GetNativeWindow(), m_pGLContext );

		// Initialise glew
		GLenum glewStatus = glewInit();
		TASSERT( glewStatus == GLEW_OK, (const char*)glewGetErrorString( glewStatus ) );

		// Create the actual display window
		m_oWindowParams = a_rcWindowParams;

		if ( !m_oWindowParams.bIsWindowed )
		{
			m_pWindow->SetFullscreen( TTRUE );
			TINFO( "Setting window mode to fullscreen\n" );
		}

		m_pWindow->SetPosition( 100, 100, m_oWindowParams.uiWidth, m_oWindowParams.uiHeight );

		T2TextureManager::CreateSingleton();
		g_bCreated = TTRUE;

		return TTRUE;
	}
	else
	{
		TERROR( "Failed to create Window\n" );
		return TFALSE;
	}

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

	if ( a_pData && a_uiSize != 0 )
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

	if ( a_pIndices && a_uiCount != 0 )
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

GLuint T2Render::CreateTexture( GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eFormat, TBOOL a_bGenerateMipmap, const void* a_pData )
{
	GLuint uiTexture;
	glGenTextures( 1, &uiTexture );

	glBindTexture( GL_TEXTURE_2D, uiTexture );
	glTexImage2D( GL_TEXTURE_2D, 0, a_eFormat, a_iWidth, a_iHeight, 0, a_eFormat, GL_UNSIGNED_BYTE, a_pData );

	if ( a_pData && a_bGenerateMipmap )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	return uiTexture;
}

void T2Render::DestroyTexture( GLuint a_iId )
{
	glDeleteTextures( 1, &a_iId );
}

void T2Render::Update( TFLOAT a_fDeltaTime )
{
	TASSERT( TFALSE == m_bIsInScene );
	m_pWindow->Update();
}

void T2Render::BeginScene()
{
	TASSERT( TFALSE == m_bIsInScene );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport( 0, 0, m_oWindowParams.uiWidth, m_oWindowParams.uiHeight );

	m_bIsInScene = TTRUE;
}

void T2Render::EndScene()
{
	TASSERT( TTRUE == m_bIsInScene );

	SDL_GL_SwapWindow( m_pWindow->GetNativeWindow() );
	m_bIsInScene = TFALSE;
}

TOSHI_NAMESPACE_END
