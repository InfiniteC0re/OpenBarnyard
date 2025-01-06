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

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TUINT* a_pValue, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1uiv( loc, a_uiNumItems, a_pValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, TUINT a_uiValue )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1ui( loc, a_uiValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TINT* a_pValue, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1iv( loc, a_uiNumItems, a_pValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, TINT a_iValue )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1i( loc, a_iValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TFLOAT* a_pValue, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1fv( loc, a_uiNumItems, a_pValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, TFLOAT a_fValue )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform1f( loc, a_fValue );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TVector3* a_pVector, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform3fv( loc, a_uiNumItems, (const TFLOAT*)a_pVector );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TVector3& a_rVector )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform3fv( loc, 1, &a_rVector.x );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TVector4* a_pVector, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform4fv( loc, a_uiNumItems, (const TFLOAT*)a_pVector );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TVector4& a_rVector )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniform4fv( loc, 1, &a_rVector.x );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TMatrix44* a_pMatrix, TUINT a_uiNumItems )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniformMatrix4fv( loc, a_uiNumItems, GL_FALSE, (const TFLOAT*)a_pMatrix );
}

void T2Shader::SetUniform( const TCHAR* a_szSlotName, const TMatrix44& a_rMatrix )
{
	GLuint loc = glGetUniformLocation( m_uiProgram, a_szSlotName );
	glUniformMatrix4fv( loc, 1, GL_FALSE, &a_rMatrix.m_f11 );
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

TOSHI_NAMESPACE_END
