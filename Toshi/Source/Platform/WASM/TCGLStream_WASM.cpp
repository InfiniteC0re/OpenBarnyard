#include "ToshiPCH.h"
#include "TCGLStream_WASM.h"

#include <emscripten.h>
#include <emscripten/threading.h>
#include <GLES3/gl3.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

// The numbers are also in OpenBarnyard/WASM/gl_replay.js
enum GLOp : TUINT32
{
	GLOP_SKIP,
	GLOP_PRESENT,
	GLOP_RESIZE,
	GLOP_CREATE,
	GLOP_DELETE,
	GLOP_ACTIVE_TEXTURE,
	GLOP_ATTACH_SHADER,
	GLOP_BIND_BUFFER,
	GLOP_BIND_BUFFER_BASE,
	GLOP_BIND_FRAMEBUFFER,
	GLOP_BIND_SAMPLER,
	GLOP_BIND_TEXTURE,
	GLOP_BIND_VERTEX_ARRAY,
	GLOP_BLEND_COLOR,
	GLOP_BLEND_EQUATION_SEPARATE,
	GLOP_BLEND_FUNC_SEPARATE,
	GLOP_BUFFER_DATA,
	GLOP_BUFFER_SUB_DATA,
	GLOP_CLEAR,
	GLOP_CLEAR_COLOR,
	GLOP_CLEAR_DEPTH,
	GLOP_CLEAR_STENCIL,
	GLOP_COLOR_MASK,
	GLOP_COMPILE_SHADER,
	GLOP_COMPRESSED_TEX_IMAGE_2D,
	GLOP_CULL_FACE,
	GLOP_DEPTH_FUNC,
	GLOP_DEPTH_MASK,
	GLOP_DEPTH_RANGE,
	GLOP_DISABLE,
	GLOP_DISABLE_VERTEX_ATTRIB_ARRAY,
	GLOP_DRAW_ARRAYS,
	GLOP_DRAW_ELEMENTS,
	GLOP_ENABLE,
	GLOP_ENABLE_VERTEX_ATTRIB_ARRAY,
	GLOP_FLUSH,
	GLOP_FRONT_FACE,
	GLOP_GENERATE_MIPMAP,
	GLOP_LINK_PROGRAM,
	GLOP_PIXEL_STOREI,
	GLOP_POLYGON_OFFSET,
	GLOP_SAMPLER_PARAMETERI,
	GLOP_SCISSOR,
	GLOP_STENCIL_FUNC,
	GLOP_STENCIL_MASK,
	GLOP_STENCIL_OP,
	GLOP_TEX_IMAGE_2D,
	GLOP_TEX_PARAMETERI,
	GLOP_TEX_SUB_IMAGE_2D,
	GLOP_UNIFORM1I,
	GLOP_UNIFORM_BLOCK_BINDING,
	GLOP_USE_PROGRAM,
	GLOP_VERTEX_ATTRIB_POINTER,
	GLOP_VIEWPORT,
};

enum GLObjectType : TUINT32
{
	GLOBJECT_BUFFER,
	GLOBJECT_TEXTURE,
	GLOBJECT_SAMPLER,
	GLOBJECT_VERTEX_ARRAY,
	GLOBJECT_PROGRAM,
	GLOBJECT_VERTEX_SHADER,
	GLOBJECT_FRAGMENT_SHADER,
};

static constexpr TUINT32 RING_WORDS      = 1u << 20;
static constexpr TUINT32 RING_MASK       = RING_WORDS - 1;
static constexpr TUINT32 INLINE_MAX_SIZE = 256 * 1024;
static constexpr TUINT32 MAX_FRAMES_BEHIND = 2;

struct GLStreamState
{
	volatile TUINT32 uiRead;
	volatile TUINT32 uiPublished;
	volatile TUINT32 uiFramesDone;
	TUINT32*         pRing;
	TUINT32          uiMask;
};

static TUINT32       s_auiRing[ RING_WORDS ];
static GLStreamState s_oState = { 0, 0, 0, s_auiRing, RING_MASK };
static TUINT32       s_uiWrite;
static TUINT32       s_uiFramesSent;
static GLuint        s_uiNextObject = 1;

extern "C" EMSCRIPTEN_KEEPALIVE GLStreamState* WASM_GetGLStream()
{
	return &s_oState;
}

EM_JS( void, WASM_NotifyGLStream, (), {
	postMessage( { bGLStream : true } );
} );

static void WaitForRead( TUINT32 a_uiValue )
{
	emscripten_futex_wait( const_cast<TUINT32*>( &s_oState.uiRead ), a_uiValue, 100.0 );
}

static TUINT32 LoadRead()
{
	return __atomic_load_n( &s_oState.uiRead, __ATOMIC_ACQUIRE );
}

static void WaitForSpace( TUINT32 a_uiWords )
{
	for ( TUINT32 uiRead = LoadRead(); s_uiWrite + a_uiWords - uiRead > RING_WORDS; uiRead = LoadRead() )
	{
		Toshi::TCGLStream::Publish();
		WaitForRead( uiRead );
	}
}

// The words of a command are always in one piece
static TUINT32* Reserve( TUINT32 a_uiWords )
{
	const TUINT32 uiLeft = RING_WORDS - ( s_uiWrite & RING_MASK );

	if ( a_uiWords > uiLeft )
	{
		WaitForSpace( uiLeft );
		s_auiRing[ s_uiWrite & RING_MASK ] = GLOP_SKIP;
		s_uiWrite += uiLeft;
	}

	WaitForSpace( a_uiWords );
	return &s_auiRing[ s_uiWrite & RING_MASK ];
}

template <class T>
static TUINT32 ToWord( T a_Value ) { return TUINT32( a_Value ); }

static TUINT32 ToWord( GLfloat a_fValue )
{
	TUINT32 uiValue;
	memcpy( &uiValue, &a_fValue, sizeof( uiValue ) );
	return uiValue;
}

template <class... Args>
static void Record( GLOp a_eOp, Args... a_Args )
{
	const TUINT32 uiWords = 1 + sizeof...( a_Args );
	TUINT32*      pWords  = Reserve( uiWords );
	*pWords++             = a_eOp;
	( ( *pWords++ = ToWord( a_Args ) ), ... );
	s_uiWrite += uiWords;
}

// Layout: op, args, pointer, words of inline data, inline data. Big data is read from where it is, so the
// caller's memory has to stay valid until the page is done with it.
template <class... Args>
static void RecordData( GLOp a_eOp, const void* a_pData, TUINT32 a_uiSize, Args... a_Args )
{
	const TBOOL   bInline     = a_pData && a_uiSize <= INLINE_MAX_SIZE;
	const TUINT32 uiDataWords = bInline ? ( a_uiSize + 3 ) / 4 : 0;
	const TUINT32 uiWords     = 1 + sizeof...( a_Args ) + 2 + uiDataWords;

	TUINT32* pWords = Reserve( uiWords );
	*pWords++       = a_eOp;
	( ( *pWords++ = ToWord( a_Args ) ), ... );
	*pWords++ = bInline ? 0 : TUINT32( TUINTPTR( a_pData ) );
	*pWords++ = uiDataWords;

	if ( bInline )
		memcpy( pWords, a_pData, a_uiSize );

	s_uiWrite += uiWords;

	if ( a_pData && !bInline )
		Toshi::TCGLStream::Finish();
}

static GLuint CreateObject( GLObjectType a_eType )
{
	const GLuint uiId = s_uiNextObject++;
	Record( GLOP_CREATE, TUINT32( a_eType ), uiId );
	return uiId;
}

static void GenObjects( GLObjectType a_eType, GLsizei a_iCount, GLuint* a_pOutIds )
{
	for ( GLsizei i = 0; i < a_iCount; i++ )
		a_pOutIds[ i ] = CreateObject( a_eType );
}

static void DeleteObjects( GLObjectType a_eType, GLsizei a_iCount, const GLuint* a_pIds )
{
	for ( GLsizei i = 0; i < a_iCount; i++ )
		Record( GLOP_DELETE, TUINT32( a_eType ), a_pIds[ i ] );
}

TOSHI_NAMESPACE_START

void TCGLStream::Publish()
{
	if ( s_oState.uiPublished != s_uiWrite )
	{
		__atomic_store_n( &s_oState.uiPublished, s_uiWrite, __ATOMIC_RELEASE );
		WASM_NotifyGLStream();
	}
}

void TCGLStream::Finish()
{
	Publish();

	for ( TUINT32 uiRead = LoadRead(); uiRead != s_uiWrite; uiRead = LoadRead() )
		WaitForRead( uiRead );
}

TBOOL TCGLStream::CreateContext( TUINT a_uiWidth, TUINT a_uiHeight )
{
	return MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.Create( $0, $1 ); }, a_uiWidth, a_uiHeight );
}

void TCGLStream::DestroyContext()
{
	Finish();
}

TBOOL TCGLStream::EnableExtension( const TCHAR* a_szName )
{
	Finish();
	return MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.gl.getExtension( UTF8ToString( $0 ) ) ? 1 : 0; }, a_szName );
}

void TCGLStream::ResizeCanvas( TUINT a_uiWidth, TUINT a_uiHeight )
{
	Record( GLOP_RESIZE, a_uiWidth, a_uiHeight );
}

void TCGLStream::Present()
{
	Record( GLOP_PRESENT );
	Publish();

	s_uiFramesSent++;

	for ( ;; )
	{
		const TUINT32 uiDone = __atomic_load_n( &s_oState.uiFramesDone, __ATOMIC_ACQUIRE );

		if ( s_uiFramesSent - uiDone < MAX_FRAMES_BEHIND )
			break;

		emscripten_futex_wait( const_cast<TUINT32*>( &s_oState.uiFramesDone ), uiDone, 100.0 );
	}
}

TOSHI_NAMESPACE_END

using Toshi::TCGLStream;

extern "C"
{
	void glGenBuffers( GLsizei n, GLuint* buffers ) { GenObjects( GLOBJECT_BUFFER, n, buffers ); }
	void glGenTextures( GLsizei n, GLuint* textures ) { GenObjects( GLOBJECT_TEXTURE, n, textures ); }
	void glGenSamplers( GLsizei n, GLuint* samplers ) { GenObjects( GLOBJECT_SAMPLER, n, samplers ); }
	void glGenVertexArrays( GLsizei n, GLuint* arrays ) { GenObjects( GLOBJECT_VERTEX_ARRAY, n, arrays ); }
	void glDeleteBuffers( GLsizei n, const GLuint* buffers ) { DeleteObjects( GLOBJECT_BUFFER, n, buffers ); }
	void glDeleteTextures( GLsizei n, const GLuint* textures ) { DeleteObjects( GLOBJECT_TEXTURE, n, textures ); }
	void glDeleteSamplers( GLsizei n, const GLuint* samplers ) { DeleteObjects( GLOBJECT_SAMPLER, n, samplers ); }
	void glDeleteVertexArrays( GLsizei n, const GLuint* arrays ) { DeleteObjects( GLOBJECT_VERTEX_ARRAY, n, arrays ); }
	void glDeleteProgram( GLuint program ) { Record( GLOP_DELETE, TUINT32( GLOBJECT_PROGRAM ), program ); }
	void glDeleteShader( GLuint shader ) { Record( GLOP_DELETE, TUINT32( GLOBJECT_VERTEX_SHADER ), shader ); }
	GLuint glCreateProgram() { return CreateObject( GLOBJECT_PROGRAM ); }
	GLuint glCreateShader( GLenum type ) { return CreateObject( type == GL_VERTEX_SHADER ? GLOBJECT_VERTEX_SHADER : GLOBJECT_FRAGMENT_SHADER ); }

	void glActiveTexture( GLenum texture ) { Record( GLOP_ACTIVE_TEXTURE, texture ); }
	void glAttachShader( GLuint program, GLuint shader ) { Record( GLOP_ATTACH_SHADER, program, shader ); }
	void glBindBuffer( GLenum target, GLuint buffer ) { Record( GLOP_BIND_BUFFER, target, buffer ); }
	void glBindBufferBase( GLenum target, GLuint index, GLuint buffer ) { Record( GLOP_BIND_BUFFER_BASE, target, index, buffer ); }
	void glBindFramebuffer( GLenum target, GLuint framebuffer ) { Record( GLOP_BIND_FRAMEBUFFER, target, framebuffer ); }
	void glBindSampler( GLuint unit, GLuint sampler ) { Record( GLOP_BIND_SAMPLER, unit, sampler ); }
	void glBindTexture( GLenum target, GLuint texture ) { Record( GLOP_BIND_TEXTURE, target, texture ); }
	void glBindVertexArray( GLuint array ) { Record( GLOP_BIND_VERTEX_ARRAY, array ); }
	void glBlendColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) { Record( GLOP_BLEND_COLOR, red, green, blue, alpha ); }
	void glBlendEquationSeparate( GLenum modeRGB, GLenum modeAlpha ) { Record( GLOP_BLEND_EQUATION_SEPARATE, modeRGB, modeAlpha ); }
	void glBlendFuncSeparate( GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha ) { Record( GLOP_BLEND_FUNC_SEPARATE, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha ); }
	void glClear( GLbitfield mask ) { Record( GLOP_CLEAR, mask ); }
	void glClearColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha ) { Record( GLOP_CLEAR_COLOR, red, green, blue, alpha ); }
	void glClearDepthf( GLfloat d ) { Record( GLOP_CLEAR_DEPTH, d ); }
	void glClearStencil( GLint s ) { Record( GLOP_CLEAR_STENCIL, s ); }
	void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) { Record( GLOP_COLOR_MASK, TUINT32( red ), TUINT32( green ), TUINT32( blue ), TUINT32( alpha ) ); }
	void glCompileShader( GLuint shader ) { Record( GLOP_COMPILE_SHADER, shader ); }
	void glCullFace( GLenum mode ) { Record( GLOP_CULL_FACE, mode ); }
	void glDepthFunc( GLenum func ) { Record( GLOP_DEPTH_FUNC, func ); }
	void glDepthMask( GLboolean flag ) { Record( GLOP_DEPTH_MASK, TUINT32( flag ) ); }
	void glDepthRangef( GLfloat n, GLfloat f ) { Record( GLOP_DEPTH_RANGE, n, f ); }
	void glDisable( GLenum cap ) { Record( GLOP_DISABLE, cap ); }
	void glDisableVertexAttribArray( GLuint index ) { Record( GLOP_DISABLE_VERTEX_ATTRIB_ARRAY, index ); }
	void glDrawArrays( GLenum mode, GLint first, GLsizei count ) { Record( GLOP_DRAW_ARRAYS, mode, first, count ); }
	void glDrawElements( GLenum mode, GLsizei count, GLenum type, const void* indices ) { Record( GLOP_DRAW_ELEMENTS, mode, count, type, TUINT32( TUINTPTR( indices ) ) ); }
	void glEnable( GLenum cap ) { Record( GLOP_ENABLE, cap ); }
	void glEnableVertexAttribArray( GLuint index ) { Record( GLOP_ENABLE_VERTEX_ATTRIB_ARRAY, index ); }
	void glFlush() { Record( GLOP_FLUSH ); }
	void glFrontFace( GLenum mode ) { Record( GLOP_FRONT_FACE, mode ); }
	void glGenerateMipmap( GLenum target ) { Record( GLOP_GENERATE_MIPMAP, target ); }
	void glLinkProgram( GLuint program ) { Record( GLOP_LINK_PROGRAM, program ); }
	void glPixelStorei( GLenum pname, GLint param ) { Record( GLOP_PIXEL_STOREI, pname, param ); }
	void glPolygonOffset( GLfloat factor, GLfloat units ) { Record( GLOP_POLYGON_OFFSET, factor, units ); }
	void glSamplerParameteri( GLuint sampler, GLenum pname, GLint param ) { Record( GLOP_SAMPLER_PARAMETERI, sampler, pname, param ); }
	void glScissor( GLint x, GLint y, GLsizei width, GLsizei height ) { Record( GLOP_SCISSOR, x, y, width, height ); }
	void glStencilFunc( GLenum func, GLint ref, GLuint mask ) { Record( GLOP_STENCIL_FUNC, func, ref, mask ); }
	void glStencilMask( GLuint mask ) { Record( GLOP_STENCIL_MASK, mask ); }
	void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) { Record( GLOP_STENCIL_OP, fail, zfail, zpass ); }
	void glTexParameteri( GLenum target, GLenum pname, GLint param ) { Record( GLOP_TEX_PARAMETERI, target, pname, param ); }
	void glUniform1i( GLint location, GLint v0 ) { Record( GLOP_UNIFORM1I, location, v0 ); }
	void glUniformBlockBinding( GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding ) { Record( GLOP_UNIFORM_BLOCK_BINDING, program, uniformBlockIndex, uniformBlockBinding ); }
	void glUseProgram( GLuint program ) { Record( GLOP_USE_PROGRAM, program ); }
	void glVertexAttribPointer( GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer ) { Record( GLOP_VERTEX_ATTRIB_POINTER, index, size, type, TUINT32( normalized ), stride, TUINT32( TUINTPTR( pointer ) ) ); }
	void glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) { Record( GLOP_VIEWPORT, x, y, width, height ); }

	void glBufferData( GLenum target, GLsizeiptr size, const void* data, GLenum usage )
	{
		RecordData( GLOP_BUFFER_DATA, data, TUINT32( size ), target, TUINT32( size ), usage );
	}

	void glBufferSubData( GLenum target, GLintptr offset, GLsizeiptr size, const void* data )
	{
		RecordData( GLOP_BUFFER_SUB_DATA, data, TUINT32( size ), target, TUINT32( offset ), TUINT32( size ) );
	}

	void glCompressedTexImage2D( GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data )
	{
		RecordData( GLOP_COMPRESSED_TEX_IMAGE_2D, data, TUINT32( imageSize ), target, level, internalformat, width, height, imageSize );
	}

	// The renderer only uploads RGBA8 like this
	void glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels )
	{
		RecordData( GLOP_TEX_IMAGE_2D, pixels, TUINT32( width * height * 4 ), target, level, internalformat, width, height, format, type );
	}

	void glTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels )
	{
		RecordData( GLOP_TEX_SUB_IMAGE_2D, pixels, TUINT32( width * height * 4 ), target, level, xoffset, yoffset, width, height, format, type );
	}

	// These need an answer: the page runs them once everything before them is replayed

	GLenum glGetError()
	{
		TCGLStream::Finish();
		return MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.gl.getError(); } );
	}

	const GLubyte* glGetString( GLenum name )
	{
		static char s_szRenderer[ 256 ];
		static char s_szVersion[ 256 ];
		char*       szResult = name == GL_RENDERER ? s_szRenderer : s_szVersion;

		TCGLStream::Finish();
		MAIN_THREAD_EM_ASM( { stringToUTF8( String( Module.oGLReplay.gl.getParameter( $0 ) ), $1, 256 ); }, name, szResult );

		return TREINTERPRETCAST( const GLubyte*, szResult );
	}

	void glShaderSource( GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length )
	{
		TCGLStream::Finish();

		for ( GLsizei i = 0; i < count; i++ )
			MAIN_THREAD_EM_ASM( { Module.oGLReplay.AddShaderSource( $0, UTF8ToString( $1 ), $2 ); }, shader, string[ i ], i == count - 1 );
	}

	void glGetShaderiv( GLuint shader, GLenum pname, GLint* params )
	{
		TCGLStream::Finish();
		*params = MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.GetShaderParameter( $0, $1 ); }, shader, pname );
	}

	void glGetProgramiv( GLuint program, GLenum pname, GLint* params )
	{
		TCGLStream::Finish();
		*params = MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.GetProgramParameter( $0, $1 ); }, program, pname );
	}

	void glGetShaderInfoLog( GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog )
	{
		TCGLStream::Finish();
		MAIN_THREAD_EM_ASM( { Module.oGLReplay.GetShaderLog( $0, $1, $2 ); }, shader, infoLog, bufSize );

		if ( length )
			*length = GLsizei( strlen( infoLog ) );
	}

	void glGetProgramInfoLog( GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog )
	{
		TCGLStream::Finish();
		MAIN_THREAD_EM_ASM( { Module.oGLReplay.GetProgramLog( $0, $1, $2 ); }, program, infoLog, bufSize );

		if ( length )
			*length = GLsizei( strlen( infoLog ) );
	}

	GLuint glGetUniformBlockIndex( GLuint program, const GLchar* uniformBlockName )
	{
		TCGLStream::Finish();
		return TUINT32( MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.GetUniformBlockIndex( $0, UTF8ToString( $1 ) ); }, program, uniformBlockName ) );
	}

	void glGetActiveUniformBlockiv( GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params )
	{
		TCGLStream::Finish();
		*params = MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.GetUniformBlockParameter( $0, $1, $2 ); }, program, uniformBlockIndex, pname );
	}

	GLint glGetUniformLocation( GLuint program, const GLchar* name )
	{
		TCGLStream::Finish();
		return MAIN_THREAD_EM_ASM_INT( { return Module.oGLReplay.GetUniformLocation( $0, UTF8ToString( $1 ) ); }, program, name );
	}

	void glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels )
	{
		TCGLStream::Finish();
		MAIN_THREAD_EM_ASM( { Module.oGLReplay.ReadPixels( $0, $1, $2, $3, $4, $5, $6 ); }, x, y, width, height, format, type, pixels );
	}
}
