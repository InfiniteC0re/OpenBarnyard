#pragma once

#ifdef TRENDERINTERFACE_GL

#  include "Toshi/TPString8.h"
#  include "Toshi/T2Map.h"
#  include "Math/TMatrix44.h"
#  include "Math/TVector4.h"
#  include "Math/TVector3.h"

#  include "GL/glew.h"

#include <immintrin.h>
#include <intrin.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class which is not the same as the one used in TOSHI 2.0
// since it was never actually used in any games but in some tools like TXSViewer which
// is not publicly available, so it's not possible to match this class.
//-----------------------------------------------------------------------------

class T2CompiledShader
{
public:
	constexpr T2CompiledShader()
	    : m_uiId( 0 )
	{}

	constexpr T2CompiledShader( GLuint a_uiId )
	    : m_uiId( a_uiId )
	{}

	constexpr T2CompiledShader( const T2CompiledShader& a_rOther )
	    : m_uiId( a_rOther.m_uiId )
	{}

	constexpr GLuint GetId() const { return m_uiId; }

	constexpr operator TBOOL() const { return m_uiId != 0; }

private:
	GLuint m_uiId;
};

//-----------------------------------------------------------------------------
// Purpose: Holds references to vertex and fragment shaders forming OpenGL
// shader program.
//-----------------------------------------------------------------------------
class T2Shader
{
public:
	struct UniformMeta
	{
		GLint id;

		union
		{
			TINT      integer;
			TUINT     uInteger;
			__m512    mat44;
			__m128    vec4;
			TVector3  vec3;
			FLOAT     fp;
		};

		UniformMeta()
		{ }

		UniformMeta( GLint a_iId )
		    : id( a_iId )
		{}

		UniformMeta( const UniformMeta& a_rcOther )
		    : id( a_rcOther.id )
		    , mat44( a_rcOther.mat44 )
		{}
	};

public:
	T2Shader()
	    : m_uiProgram( 0 ), m_VShader( 0 ), m_FShader( 0 )
	{}

	T2Shader( const T2CompiledShader& a_VShader, const T2CompiledShader& a_FShader )
	    : m_uiProgram( 0 ), m_VShader( a_VShader ), m_FShader( a_FShader )
	{}

	T2Shader( const T2Shader& a_rOther )
	    : m_uiProgram( a_rOther.m_uiProgram ), m_VShader( a_rOther.m_VShader ), m_FShader( a_rOther.m_FShader )
	{}

	TBOOL Create();
	void  Destroy( TBOOL a_bDeleteShaders );

	UniformMeta& GetUniformSlotId( const TCHAR* a_szSlotName );
	UniformMeta& GetUniformSlotId( const TPString8& a_strSlotName );

	void SetUniform( const TPString8& a_szSlotName, const TMatrix44& a_rMatrix );
	void SetUniform( const TPString8& a_szSlotName, const TMatrix44* a_pMatrix, TUINT a_uiNumItems );
	void SetUniform( const TPString8& a_szSlotName, const TVector4& a_rVector );
	void SetUniform( const TPString8& a_szSlotName, const TVector4* a_pVector, TUINT a_uiNumItems );
	void SetUniform( const TPString8& a_szSlotName, const TVector3& a_rVector );
	void SetUniform( const TPString8& a_szSlotName, const TVector3* a_pVector, TUINT a_uiNumItems );
	void SetUniform( const TPString8& a_szSlotName, TFLOAT a_fValue );
	void SetUniform( const TPString8& a_szSlotName, const TFLOAT* a_pValue, TUINT a_uiNumItems );
	void SetUniform( const TPString8& a_szSlotName, TINT a_iValue );
	void SetUniform( const TPString8& a_szSlotName, const TINT* a_pValue, TUINT a_uiNumItems );
	void SetUniform( const TPString8& a_szSlotName, TUINT a_uiValue );
	void SetUniform( const TPString8& a_szSlotName, const TUINT* a_pValue, TUINT a_uiNumItems );

	GLuint           GetProgram() const { return m_uiProgram; }
	T2CompiledShader GetVertexShader() const { return m_VShader; }
	T2CompiledShader GetFragmentShader() const { return m_FShader; }

	T2Shader& operator=( const T2Shader& a_rOther );

private:
	friend class T2Render;
	friend class T2RenderContext;
	void Use() const;

private:
	GLuint                                          m_uiProgram;
	T2CompiledShader                                m_VShader;
	T2CompiledShader                                m_FShader;

	T2Map<TPString8, UniformMeta, TPString8::Comparator> m_UniformToSlotId;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
