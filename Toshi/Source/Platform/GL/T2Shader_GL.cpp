#include "ToshiPCH.h"
#include "T2Shader_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL T2Shader::Create()
{
	TASSERT( m_uiProgram == 0 );

	if ( m_VShader && m_FShader )
	{
		m_uiProgram = glCreateProgram();
		TASSERT( m_uiProgram != 0, "Couldn't create GL shader program" );

		glAttachShader( m_uiProgram, m_VShader.GetId() );
		glAttachShader( m_uiProgram, m_FShader.GetId() );
		glLinkProgram( m_uiProgram );
		glValidateProgram( m_uiProgram );

		GLint iProgramStatus;
		glGetProgramiv( m_uiProgram, GL_VALIDATE_STATUS, &iProgramStatus );

		return iProgramStatus == GL_TRUE;
	}

	return TFALSE;
}

void T2Shader::Destroy( TBOOL a_bDeleteShaders )
{
	glDeleteProgram( m_uiProgram );

	if ( a_bDeleteShaders )
	{
		glDeleteShader( m_VShader.GetId() );
		glDeleteShader( m_FShader.GetId() );
	}
}

void T2Shader::Use() const
{
	TASSERT( m_uiProgram != 0 );
	glUseProgram( m_uiProgram );
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TUINT* a_pValue, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pValue );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniform1uiv( meta.id, a_uiNumItems, a_pValue );
		meta.uInteger = *a_pValue;

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, TUINT a_uiValue )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	if ( meta.uInteger != a_uiValue )
	{
		glUniform1ui( meta.id, a_uiValue );
		meta.uInteger = a_uiValue;

		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TINT* a_pValue, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pValue );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniform1iv( meta.id, a_uiNumItems, a_pValue );
		meta.integer = *a_pValue;

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, TINT a_iValue )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	if ( meta.integer != a_iValue )
	{
		glUniform1i( meta.id, a_iValue );
		meta.integer = a_iValue;

		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TFLOAT* a_pValue, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pValue );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniform1fv( meta.id, a_uiNumItems, a_pValue );
		meta.fp = *a_pValue;

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, TFLOAT a_fValue )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	if ( meta.fp != a_fValue )
	{
		glUniform1f( meta.id, a_fValue );
		meta.fp = a_fValue;
		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TVector3* a_pVector, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pVector );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniform3fv( meta.id, a_uiNumItems, (const TFLOAT*)a_pVector );
		meta.vec3 = *a_pVector;

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TVector3& a_rVector )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	if ( meta.vec3.x != a_rVector.x || meta.vec3.y != a_rVector.y || meta.vec3.z != a_rVector.z )
	{
		glUniform3fv( meta.id, 1, &a_rVector.x );
		meta.vec3 = a_rVector;

		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TVector4* a_pVector, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pVector );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniform4fv( meta.id, a_uiNumItems, (const TFLOAT*)a_pVector );
		meta.vec4 = _mm_loadu_ps( (float*)a_pVector );

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TVector4& a_rVector )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	__m128 v0   = _mm_loadu_ps( (float*)&a_rVector.x );
	__m128 vcmp = _mm_cmpneq_ps( v0, meta.vec4 );

	if ( _mm_movemask_ps( vcmp ) != 0 )
	{
		glUniform4fv( meta.id, 1, &a_rVector.x );
		meta.vec4 = _mm_loadu_ps( (float*)&a_rVector.x );

		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TMatrix44* a_pMatrix, TUINT a_uiNumItems )
{
	if ( a_uiNumItems == 1 )
		return SetUniform( a_strSlotName, *a_pMatrix );
	else
	{
		UniformMeta& meta = GetUniformSlotId( a_strSlotName );

		if ( meta.id < 0 )
			return TFALSE;

		glUniformMatrix4fv( meta.id, a_uiNumItems, GL_FALSE, (const TFLOAT*)a_pMatrix );
		meta.mat44 = *a_pMatrix;

		return TTRUE;
	}
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const TMatrix44& a_rMatrix )
{
	UniformMeta& meta = GetUniformSlotId( a_strSlotName );

	if ( meta.id < 0 )
		return TFALSE;

	//__m512* m0 = (__m512*)&a_rMatrix;

	if ( TUtil::MemCompare( &meta.mat44, &a_rMatrix, 64 ) != 0 )
	{
		glUniformMatrix4fv( meta.id, 1, GL_FALSE, &a_rMatrix.m_f11 );
		meta.mat44 = a_rMatrix;
		//meta.mat44 = _mm512_loadu_ps( m0 );

		return TTRUE;
	}

	return TFALSE;
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const glm::mat4& a_rMatrix )
{
	return SetUniform( a_strSlotName, *(TMatrix44*)&a_rMatrix );
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const glm::mat4* a_pMatrix, TUINT a_uiNumItems )
{
	return SetUniform( a_strSlotName, (TMatrix44*)a_pMatrix, a_uiNumItems );
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const glm::vec4& a_rVector )
{
	return SetUniform( a_strSlotName, *(TVector4*)&a_rVector );
}

TBOOL T2Shader::SetUniform( const TPString8& a_strSlotName, const glm::vec4* a_pVector, TUINT a_uiNumItems )
{
	return SetUniform( a_strSlotName, (TVector4*)a_pVector, a_uiNumItems );
}

T2Shader& T2Shader::operator=( const T2Shader& a_rOther )
{
	if ( this != &a_rOther )
	{
		m_uiProgram = a_rOther.m_uiProgram;
		m_VShader   = a_rOther.m_VShader;
		m_FShader   = a_rOther.m_FShader;
	}

	return *this;
}

T2Shader::UniformMeta& T2Shader::GetUniformSlotId( const TCHAR* a_szSlotName )
{
	return GetUniformSlotId( TPS8D( a_szSlotName ) );
}

T2Shader::UniformMeta& T2Shader::GetUniformSlotId( const TPString8& a_strSlotName )
{
	auto it = m_UniformToSlotId.Find( a_strSlotName );

	if ( it != m_UniformToSlotId.End() )
		return it->second;

	GLint iLoc = glGetUniformLocation( m_uiProgram, a_strSlotName.GetString() );
	return *m_UniformToSlotId.Emplace( a_strSlotName, ( iLoc == -1 ) ? -2 : iLoc );
}

TOSHI_NAMESPACE_END
