#include "pch.h"
#include <gl/glew.h>

#include "TRender_SDL.h"
#include "TRenderContext_SDL.h"
#include "TTexture_SDL.h"

namespace Toshi
{

TDEFINE_CLASS( TRenderSDL );

TBOOL TRenderSDL::CreateDisplay( const DISPLAYPARAMS& a_rDisplayParams )
{
	TASSERT( IsCreated() == TTRUE );
	TASSERT( IsDisplayCreated() == TFALSE );

	if ( TRenderInterface::CreateDisplay() == TFALSE )
	{
		ShowDisplayError();
		return TFALSE;
	}

	m_DisplayParams = a_rDisplayParams;

	GetCurrentContext()->SetViewportParameters( { .fX	   = 0.0f,
												  .fY	   = 0.0f,
												  .fWidth  = (TFLOAT)m_DisplayParams.uiWidth,
												  .fHeight = (TFLOAT)m_DisplayParams.uiHeight,
												  .fMinZ   = 0.0f,
												  .fMaxZ   = 1.0f } );

	if ( !m_DisplayParams.bWindowed )
	{
		m_Window.SetFullscreen( TTRUE );
	}

	m_Window.SetPosition( 100, 100, m_DisplayParams.uiWidth, m_DisplayParams.uiHeight );

	TTextureManagerSDL::CreateSingleton();

	return TFALSE;
}

TBOOL TRenderSDL::Update( TFLOAT a_fDeltaTime )
{
	TASSERT( TTRUE == IsCreated() );

	m_Window.Update();
	return TTRUE;
}

TBOOL TRenderSDL::BeginScene()
{
	TASSERT( TTRUE == IsDisplayCreated() );
	TASSERT( TFALSE == IsInScene() );

	TRenderInterface::BeginScene();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport( 0, 0, m_DisplayParams.uiWidth, m_DisplayParams.uiHeight );
	m_bInScene = TTRUE;

	return TTRUE;
}

TBOOL TRenderSDL::EndScene()
{
	TASSERT( TTRUE == IsDisplayCreated() );
	TASSERT( TTRUE == IsInScene() );

	m_uiNumDrawnFrames += 1;

	SDL_GL_SwapWindow( m_Window.GetNativeWindow() );

	m_bInScene = TFALSE;
	return TTRUE;
}

TBOOL TRenderSDL::Create( const char* a_szName )
{
	TASSERT( TFALSE == IsCreated() );
	TBOOL bResult = TRenderInterface::Create();

	if ( bResult )
	{
		TINFO( "Creating TRenderSDL\n" );

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

		if ( m_hAccel != NULL )
		{
			DestroyAcceleratorTable( m_hAccel );
			m_hAccel = NULL;
		}

		ACCEL accels[ 2 ];
		accels[ 0 ].fVirt = FVIRTKEY;
		accels[ 0 ].key	  = VK_ESCAPE;

		accels[ 1 ].fVirt = FALT;
		accels[ 1 ].key	  = VK_RETURN;
		accels[ 1 ].cmd	  = 1;

		m_hAccel = CreateAcceleratorTableA( accels, sizeof( accels ) / sizeof( *accels ) );

		if ( m_Window.Create( this, a_szName ) )
		{
			m_GLContext = SDL_GL_CreateContext( m_Window.GetNativeWindow() );
			SDL_GL_MakeCurrent( m_Window.GetNativeWindow(), m_GLContext );

			GLenum glewStatus = glewInit();
			TASSERT( glewStatus == GLEW_OK, (const char*)glewGetErrorString( glewStatus ) );

			return TTRUE;
		}

		TERROR( "Failed to create Window\n" );
	}

	return TFALSE;
}

void TRenderSDL::WaitForEndOfRender()
{
	TIMPLEMENT();
}

void TRenderSDL::FlushShaders()
{
	TASSERT( TTRUE == IsInScene() );

	for ( auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++ )
	{
		it->Flush();
	}
}

GLuint TRenderSDL::CreateTexture( GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eFormat, TBOOL a_bGenerateMipmap, const void* a_pData )
{
	GLuint uiTexture;
	glGenTextures( 1, &uiTexture );

	glBindTexture( GL_TEXTURE_2D, uiTexture );
	glTexImage2D( GL_TEXTURE_2D, 0, a_eFormat, a_iWidth, a_iHeight, 0, a_eFormat, GL_UNSIGNED_BYTE, a_pData );

	if ( a_bGenerateMipmap )
	{
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	return uiTexture;
}

TGLShaderRef TRenderSDL::CompileShader( GLenum a_eShader, const char* a_szSource )
{
	TGLShaderRef shader = glCreateShader( a_eShader );
	TASSERT( shader, "Couldn't create GL shader" );

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

TGLShaderRef TRenderSDL::CompileShaderFromFile( GLenum a_eShader, const char* a_szFileName )
{
	TFile* pFile	= TFile::Create( a_szFileName );
	DWORD  fileSize = pFile->GetSize();
	char*  srcData	= new char[ fileSize + 1 ];
	pFile->Read( srcData, fileSize );
	srcData[ fileSize ] = '\0';
	pFile->Destroy();

	TGLShaderRef shader = CompileShader( a_eShader, srcData );
	delete[] srcData;

	return shader;
}

TGLShaderProgram TRenderSDL::CreateShaderProgram( TGLShaderRef a_VertexShader, TGLShaderRef a_FragmentShader )
{
	TGLShaderProgram program( a_VertexShader, a_FragmentShader );
	program.Create();

	return program;
}

TGLVertexBuffer TRenderSDL::CreateVertexBuffer( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage )
{
	GLuint bufferId;
	glGenBuffers( 1, &bufferId );
	TASSERT( bufferId != 0 );

	TGLVertexBuffer buffer = bufferId;

	if ( a_uiSize != 0 )
	{
		buffer.SetData( a_pData, a_uiSize, a_eUsage );
	}

	return buffer;
}

TGLIndexBuffer TRenderSDL::CreateIndexBuffer( const uint16_t* a_pIndices, unsigned int a_uiCount, GLenum a_eUsage )
{
	GLuint bufferId;
	glGenBuffers( 1, &bufferId );
	TASSERT( bufferId != 0 );

	TGLIndexBuffer buffer = bufferId;

	if ( a_uiCount != 0 )
	{
		buffer.SetData( a_pIndices, sizeof( *a_pIndices ) * a_uiCount, a_eUsage );
	}

	return buffer;
}

TGLVertexArrayRef TRenderSDL::CreateVertexArray()
{
	GLuint vaoId;
	glGenVertexArrays( 1, &vaoId );
	TASSERT( vaoId != 0 );

	return TGLVertexArrayRef( vaoId );
}

TGLVertexArrayRef TRenderSDL::CreateVertexArray( TGLVertexBuffer a_VertexBuffer, TGLIndexBuffer a_IndexBuffer )
{
	GLuint vaoId;
	glGenVertexArrays( 1, &vaoId );
	TASSERT( vaoId != 0 );

	TGLVertexArrayRef vao( vaoId );
	vao.SetVertexBuffer( a_VertexBuffer );
	vao.SetIndexBuffer( a_IndexBuffer );

	return vao;
}

Toshi::TRenderContext* TRenderSDL::CreateRenderContext()
{
	return new TRenderContextSDL( this );
}

Toshi::TRenderCapture* TRenderSDL::CreateCapture()
{
	throw std::logic_error( "The method or operation is not implemented." );
	return TNULL;
}

void TRenderSDL::DestroyCapture( TRenderCapture* a_pRenderCapture )
{
	throw std::logic_error( "The method or operation is not implemented." );
}

void* TRenderSDL::CreateUnknown( const char* a_szName, int a_iUnk1, int a_iUnk2, int a_iUnk3 )
{
	return TNULL;
}

Toshi::TModel* TRenderSDL::CreateModel( TTMD* a_pTMD, TBOOL a_bLoad )
{
	throw std::logic_error( "The method or operation is not implemented." );
	return TNULL;
}

Toshi::TModel* TRenderSDL::CreateModel( const char* a_szFilePath, TBOOL a_bLoad )
{
	throw std::logic_error( "The method or operation is not implemented." );
	return TNULL;
}

Toshi::TModel* TRenderSDL::CreateModel( const char* a_szFilePath, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen )
{
	throw std::logic_error( "The method or operation is not implemented." );
	return TNULL;
}

Toshi::TDebugText* TRenderSDL::CreateDebugText()
{
	throw std::logic_error( "The method or operation is not implemented." );
	return TNULL;
}

void TRenderSDL::DestroyDebugText()
{
	throw std::logic_error( "The method or operation is not implemented." );
}

TBOOL TRenderSDL::RecreateDisplay( DISPLAYPARAMS* a_pDisplayParams )
{
	TASSERT( TFALSE, "Not implemented" );
	return TFALSE;
}

void TRenderSDL::ShowDeviceError()
{
	MessageBoxA( NULL, "Initialization failure", "Failed to create SDL device", MB_OK );
}

void TRenderSDL::ShowDisplayError()
{
	MessageBoxA( NULL, "Initialization failure", "Failed to create the display. Please run the Barnyard setup program and reconfigure", MB_OK );
}

} // namespace Toshi
