#include "ToshiPCH.h"
#include "TMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TDEFINE_CLASS_NORUNTIME(TMesh);

	TBOOL Toshi::TMesh::Validate()
	{
		m_State |= State_Validated;
		return TTRUE;
	}

	void Toshi::TMesh::Invalidate()
	{
		m_State &= ~State_Validated;
	}

	TBOOL Toshi::TMesh::Create()
	{
		m_State |= State_Created;
		return TTRUE;
	}

	TBOOL Toshi::TMesh::Render()
	{
		return TTRUE;
	}

	void Toshi::TMesh::OnDestroy()
	{
		m_State &= ~State_Created;
	}

}