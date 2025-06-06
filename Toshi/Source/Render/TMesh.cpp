#include "ToshiPCH.h"
#include "TMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TMesh );

TMesh::TMesh()
{
	m_pOwnerShader = TNULL;
	m_pMaterial    = TNULL;
	m_State        = State_None;
}

// $Barnyard: FUNCTION 005f5e80
TMesh::~TMesh()
{
	m_pOwnerShader = TNULL;
	m_pMaterial    = TNULL;
	m_State        = State_None;
}

TBOOL TMesh::Validate()
{
	m_State |= State_Validated;
	return TTRUE;
}

void TMesh::Invalidate()
{
	m_State &= ~State_Validated;
}

// $Barnyard: FUNCTION 005efff0
TBOOL TMesh::Create()
{
	m_State |= State_Created;
	return TTRUE;
}

TBOOL TMesh::Render()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 005f0000
void TMesh::OnDestroy()
{
	m_State &= ~State_Created;
}

// $Barnyard: FUNCTION 006ccd60
void TMesh::SetFlags( State a_eFlags, TBOOL a_bEnable )
{
	if ( a_bEnable )
		m_State |= a_eFlags;
	else
		m_State &= ~a_eFlags;
}

// $Barnyard: FUNCTION 006ccd40
void TMesh::DestroyResource()
{
	Invalidate();
	OnDestroy();
	delete this;
}

void TMesh::SetMaterial( TMaterial* pMaterial )
{
	m_pMaterial = pMaterial;
}

TMaterial* TMesh::GetMaterial() const
{
	return m_pMaterial;
}

void TMesh::SetOwnerShader( TShader* pShader )
{
	TASSERT( TNULL == m_pOwnerShader );
	m_pOwnerShader = pShader;
}

TBOOL TMesh::IsCreated() const
{
	return m_State & State_Created;
}

TBOOL TMesh::IsValidated() const
{
	return m_State & State_Validated;
}

TBOOL TMesh::IsUnknownState1() const
{
	return m_State & State_Unknown1;
}

TShader* TMesh::GetOwnerShader()
{
	return m_pOwnerShader;
}

TOSHI_NAMESPACE_END
