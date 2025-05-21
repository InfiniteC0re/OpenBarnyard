#include "ToshiPCH.h"
#include "TShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TShader );

// $Barnyard: FUNCTION 006ccbb0
TShader::TShader()
{
	m_State = State_None;
	sm_oShaderList.AddShader( this );
}

TShader::~TShader()
{
	sm_oShaderList.RemoveShader( this );
}

// $Barnyard: FUNCTION 006ccc30
void TShader::OnDestroy()
{
	m_State &= ~State_Created;
}

void TShader::Flush()
{
}

// $Barnyard: FUNCTION 006ccc20
TBOOL TShader::Create()
{
	m_State |= State_Created;
	return TTRUE;
}

// $Barnyard: FUNCTION 006ccc40
TBOOL TShader::Validate()
{
	m_State |= State_Validated;
	return TTRUE;
}

// $Barnyard: FUNCTION 006ccc50
void TShader::Invalidate()
{
	m_State &= ~State_Validated;
}

void TShader::TShaderList::AddShader( TShader* pShader )
{
	TShader* pLastShader = m_pRoot;

	while ( pLastShader && pLastShader->m_pNextShader != TNULL )
		pLastShader = pLastShader->m_pNextShader;

	if ( pLastShader != TNULL )
		pLastShader->m_pNextShader = pShader;
	else
		m_pRoot = pShader;

	pShader->m_pNextShader = TNULL;
}

void TShader::TShaderList::RemoveShader( TShader* pShader )
{
	TShader* pCurrShader = m_pRoot;
	TShader* pPrevShader = TNULL;
	TBOOL    bFound      = TFALSE;

	while ( pCurrShader )
	{
		if ( pCurrShader == pShader )
		{
			bFound = TTRUE;
			break;
		}

		pPrevShader = pCurrShader;
		pCurrShader = pCurrShader->m_pNextShader;
	}

	if ( bFound )
	{
		if ( pPrevShader )
		{
			TASSERT( m_pRoot != pShader );
			pPrevShader->m_pNextShader = pShader->m_pNextShader;
		}
		else
		{
			TASSERT( m_pRoot == pShader );
			m_pRoot = pShader->m_pNextShader;
		}
	}
}

TOSHI_NAMESPACE_END
