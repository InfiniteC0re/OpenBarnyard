#include "pch.h"
#include "ASkinMesh.h"

ASkinMesh::ASkinMesh()
{
	m_uiFlags = 0;
	m_uiMaxVertices = 0;
	m_uiNumSubMeshes = 0;
	m_pVertexPool = TNULL;
	m_pIndexPool = TNULL;
}

TBOOL ASkinMesh::Validate()
{
	if (!m_State.IsSet(State::Validated))
		m_State.Set(State::Validated);

	return TTRUE;
}

void ASkinMesh::Invalidate()
{
	if (m_State.IsSet(State::Validated))
		m_State.Unset(State::Validated);
}
